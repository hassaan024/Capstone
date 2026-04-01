/// Fill out your copyright notice in the Description page of Project Settings.

#include "PlantImageCacheSubsystem.h"
#include "ImageUtils.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

void UPlantImageCacheSubsystem::GetOrLoadImage(const FString& Url, const FOnPlantImageReady& Callback)
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

void UPlantImageCacheSubsystem::FinishWithFailure(const FString& Url)
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

void UPlantImageCacheSubsystem::FinishWithSuccess(const FString& Url, UTexture2D* Texture)
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

void UPlantImageCacheSubsystem::ClearCache()
{
	TextureCache.Empty();
	PendingCallbacks.Empty();
}