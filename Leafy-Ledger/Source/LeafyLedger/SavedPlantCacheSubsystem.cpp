/// Fill out your copyright notice in the Description page of Project Settings.

#include "SavedPlantCacheSubsystem.h"
#include "ImageUtils.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "BackendApiSubsystem.h"

void USavedPlantCacheSubsystem::WarmAfterLogin()
{
	RefreshSavedPlants(FOnSavedPlantsRefreshed());
}

void USavedPlantCacheSubsystem::RefreshSavedPlants(const FOnSavedPlantsRefreshed& Callback)
{
	if (bRefreshInFlight)
	{
		if (bHasLoadedSavedPlants)
		{
			Callback.ExecuteIfBound(true, TEXT("Using cached saved plants while refresh is in flight"), CachedPlants);
		}
		return;
	}

	if (!GetGameInstance())
	{
		Callback.ExecuteIfBound(false, TEXT("No GameInstance"), CachedPlants);
		return;
	}

	UBackendApiSubsystem* Api = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Api)
	{
		Callback.ExecuteIfBound(false, TEXT("Backend API subsystem not found"), CachedPlants);
		return;
	}

	bRefreshInFlight = true;

	Api->GetSavedSpecies(
		FBackendPlantsResponse::CreateWeakLambda(this,
			[this, Callback](bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
			{
				bRefreshInFlight = false;

				if (!bSuccess)
				{
					Callback.ExecuteIfBound(false, Message, CachedPlants);
					return;
				}

				CachedPlants = Plants;
				bHasLoadedSavedPlants = true;

				CachedPlants = Plants;
				bHasLoadedSavedPlants = true;

				UE_LOG(LogTemp, Warning, TEXT("=== Saved Plants Cached: Count = %d ==="), CachedPlants.Num());

				for (const FBackendPlantDto& Plant : CachedPlants)
				{
					UE_LOG(LogTemp, Warning,
						TEXT("Plant: Id=%d | PerenualId=%d | Name=%s | Scientific=%s | Category=%s | Img=%s"),
						Plant.Id,
						Plant.PerenualId,
						*Plant.CommonName,
						*Plant.ScientificName,
						*Plant.ModelCategory,
						*Plant.ImgSrcUrls.Regular
					);
				}

				PrefetchPlantImages(CachedPlants);

				Callback.ExecuteIfBound(true, TEXT("Saved plants refreshed"), CachedPlants);
			}
		)
	);
}

void USavedPlantCacheSubsystem::PrefetchPlantImages(const TArray<FBackendPlantDto>& Plants)
{
	for (const FBackendPlantDto& Plant : Plants)
	{
		const FString Url = GetPreferredImageUrl(Plant);
		if (Url.IsEmpty())
		{
			continue;
		}

		GetOrLoadImage(
			Url,
			FOnPlantImageReady::CreateLambda(
				[Plant](UTexture2D* Texture)
				{
					UE_LOG(
						LogTemp,
						Log,
						TEXT("Prefetch image for %s (%d): %s"),
						*Plant.CommonName,
						Plant.PerenualId,
						Texture ? TEXT("cached") : TEXT("failed")
					);
				}
			)
		);
	}
}

FString USavedPlantCacheSubsystem::GetPreferredImageUrl(const FBackendPlantDto& Plant) const
{
	return Plant.ImgSrcUrls.Regular;
}

void USavedPlantCacheSubsystem::RemoveCachedPlantByPerenualId(int32 PerenualId)
{
	CachedPlants.RemoveAll(
		[PerenualId](const FBackendPlantDto& Plant)
		{
			return Plant.PerenualId == PerenualId;
		}
	);
}

void USavedPlantCacheSubsystem::ClearAllCaches()
{
	CachedPlants.Empty();
	TextureCache.Empty();
	PendingCallbacks.Empty();
	bHasLoadedSavedPlants = false;
	bRefreshInFlight = false;
}

void USavedPlantCacheSubsystem::GetOrLoadImage(const FString& Url, const FOnPlantImageReady& Callback)
{
	if (Url.IsEmpty())
	{
		Callback.ExecuteIfBound(nullptr);
		return;
	}

	if (UTexture2D** CachedTexture = TextureCache.Find(Url))
	{
		Callback.ExecuteIfBound(*CachedTexture);
		return;
	}

	if (TArray<FOnPlantImageReady>* ExistingPending = PendingCallbacks.Find(Url))
	{
		ExistingPending->Add(Callback);
		return;
	}

	PendingCallbacks.Add(Url, { Callback });

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));

	Request->OnProcessRequestComplete().BindLambda(
		[this, Url](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bSuccess)
		{
			if (!bSuccess || !Response.IsValid() || Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300)
			{
				UE_LOG(LogTemp, Warning, TEXT("Image download failed: %s"), *Url);
				FinishWithFailure(Url);
				return;
			}

			const TArray<uint8>& ImageData = Response->GetContent();
			UTexture2D* Texture = FImageUtils::ImportBufferAsTexture2D(ImageData);

			if (!Texture)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to decode image: %s"), *Url);
				FinishWithFailure(Url);
				return;
			}

			TextureCache.Add(Url, Texture);
			FinishWithSuccess(Url, Texture);
		}
	);

	if (!Request->ProcessRequest())
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to start image request: %s"), *Url);
		FinishWithFailure(Url);
	}
}

void USavedPlantCacheSubsystem::FinishWithFailure(const FString& Url)
{
	TArray<FOnPlantImageReady> Callbacks;
	if (PendingCallbacks.RemoveAndCopyValue(Url, Callbacks))
	{
		for (const FOnPlantImageReady& Callback : Callbacks)
		{
			Callback.ExecuteIfBound(nullptr);
		}
	}
}

void USavedPlantCacheSubsystem::FinishWithSuccess(const FString& Url, UTexture2D* Texture)
{
	TArray<FOnPlantImageReady> Callbacks;
	if (PendingCallbacks.RemoveAndCopyValue(Url, Callbacks))
	{
		for (const FOnPlantImageReady& Callback : Callbacks)
		{
			Callback.ExecuteIfBound(Texture);
		}
	}
}