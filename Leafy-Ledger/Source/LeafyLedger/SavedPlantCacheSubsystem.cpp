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
		else if (Callback.IsBound())
		{
			PendingRefreshCallbacks.Add(Callback);
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
	if (Callback.IsBound())
	{
		PendingRefreshCallbacks.Add(Callback);
	}

	Api->GetSavedSpecies(
		FBackendPlantsResponse::CreateWeakLambda(this,
			[this](bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
			{
				bRefreshInFlight = false;

				if (!bSuccess)
				{
					ExecutePendingRefreshCallbacks(false, Message, CachedPlants);
					return;
				}

				CachedPlants = MergePlantsPreservingOrder(CachedPlants, Plants);
				bHasLoadedSavedPlants = true;
				CachePlantDetails(CachedPlants);

				LogCachedPlants(TEXT("Global"), CachedPlants);

				PrefetchPlantImages(CachedPlants);
				WarmAllGardenSavedPlantCaches();

				ExecutePendingRefreshCallbacks(true, TEXT("Saved plants refreshed"), CachedPlants);
			}
		)
	);
}

void USavedPlantCacheSubsystem::RefreshGardenSavedPlants(int32 GardenId, const FOnSavedPlantsRefreshed& Callback)
{
	if (GardenId <= 0)
	{
		static const TArray<FBackendPlantDto> EmptyPlants;
		Callback.ExecuteIfBound(false, TEXT("GardenId must be > 0"), EmptyPlants);
		return;
	}

	if (GardenRefreshesInFlight.Contains(GardenId))
	{
		if (const TArray<FBackendPlantDto>* CachedGarden = CachedGardenPlants.Find(GardenId))
		{
			Callback.ExecuteIfBound(true, TEXT("Using cached garden saved plants while refresh is in flight"), *CachedGarden);
		}
		else if (Callback.IsBound())
		{
			PendingGardenRefreshCallbacks.FindOrAdd(GardenId).Add(Callback);
		}
		return;
	}

	if (!GetGameInstance())
	{
		static const TArray<FBackendPlantDto> EmptyPlants;
		Callback.ExecuteIfBound(false, TEXT("No GameInstance"), EmptyPlants);
		return;
	}

	UBackendApiSubsystem* Api = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Api)
	{
		static const TArray<FBackendPlantDto> EmptyPlants;
		Callback.ExecuteIfBound(false, TEXT("Backend API subsystem not found"), EmptyPlants);
		return;
	}

	GardenRefreshesInFlight.Add(GardenId);
	if (Callback.IsBound())
	{
		PendingGardenRefreshCallbacks.FindOrAdd(GardenId).Add(Callback);
	}

	Api->GetSavedSpeciesForGarden(
		GardenId,
		FBackendPlantsResponse::CreateWeakLambda(this,
			[this, GardenId](bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
			{
				GardenRefreshesInFlight.Remove(GardenId);

				if (!bSuccess)
				{
					const TArray<FBackendPlantDto>* CachedGarden = CachedGardenPlants.Find(GardenId);
					static const TArray<FBackendPlantDto> EmptyPlants;
					ExecutePendingGardenRefreshCallbacks(GardenId, false, Message, CachedGarden ? *CachedGarden : EmptyPlants);
					return;
				}

				const TArray<FBackendPlantDto>* ExistingPlants = CachedGardenPlants.Find(GardenId);
				const TArray<FBackendPlantDto> OrderedPlants = ExistingPlants
					? MergePlantsPreservingOrder(*ExistingPlants, Plants)
					: Plants;

				CachedGardenPlants.Add(GardenId, OrderedPlants);
				CachePlantDetails(OrderedPlants);
				LogCachedPlants(FString::Printf(TEXT("Garden %d"), GardenId), OrderedPlants);
				PrefetchPlantImages(OrderedPlants);

				ExecutePendingGardenRefreshCallbacks(GardenId, true, TEXT("Garden saved plants refreshed"), OrderedPlants);
			}
		)
	);
}

void USavedPlantCacheSubsystem::PrefetchPlantImages(const TArray<FBackendPlantDto>& Plants)
{
	for (const FBackendPlantDto& Plant : Plants)
	{
		const TArray<FString> Urls = GetImageUrlCandidates(Plant);
		if (Urls.Num() == 0)
		{
			continue;
		}

		for (const FString& Url : Urls)
		{
			GetOrLoadImage(
				Url,
				FOnPlantImageReady::CreateLambda(
					[Plant, Url](UTexture2D* Texture)
					{
						UE_LOG(LogTemp, Log, TEXT("Prefetch image for %s (%d): %s | %s"), 
							*Plant.CommonName, 
							Plant.PerenualId,
							Texture ? TEXT("cached") : TEXT("failed"), 
							*Url
						);
					}
				)
			);
		}
	}
}

void USavedPlantCacheSubsystem::WarmAllGardenSavedPlantCaches()
{
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not warm garden saved plant caches: No GameInstance"));
		return;
	}

	UBackendApiSubsystem* Api = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Api)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not warm garden saved plant caches: Backend API subsystem not found"));
		return;
	}

	Api->GetGardensByUser(
		FBackendGardenSummariesResponse::CreateWeakLambda(this,
			[this](bool bSuccess, const FString& Message, const TArray<FBackendGardenSummaryDto>& Gardens)
			{
				if (!bSuccess)
				{
					UE_LOG(LogTemp, Warning, TEXT("Could not warm garden saved plant caches: %s"), *Message);
					return;
				}

				UE_LOG(LogTemp, Warning, TEXT("=== Warming Garden Saved Plant Caches: Garden Count = %d ==="), Gardens.Num());

				for (const FBackendGardenSummaryDto& Garden : Gardens)
				{
					if (Garden.Id <= 0)
					{
						continue;
					}

					UE_LOG(
						LogTemp,
						Warning,
						TEXT("Queueing garden saved plant cache warmup: GardenId=%d | Name=%s | PlantCount=%d"),
						Garden.Id,
						*Garden.Name,
						Garden.PlantCount
					);

					RefreshGardenSavedPlants(Garden.Id, FOnSavedPlantsRefreshed());
				}
			}
		)
	);
}

void USavedPlantCacheSubsystem::CachePlantDetails(const FBackendPlantDto& Plant)
{
	if (Plant.PerenualId > 0)
	{
		PlantDetailsCache.Add(Plant.PerenualId, Plant);
	}
}

void USavedPlantCacheSubsystem::CachePlantDetails(const TArray<FBackendPlantDto>& Plants)
{
	for (const FBackendPlantDto& Plant : Plants)
	{
		CachePlantDetails(Plant);
	}
}

bool USavedPlantCacheSubsystem::HasCompletePopupDetails(const FBackendPlantDto& Plant)
{
	return !Plant.SunlightText.IsEmpty()
		&& !Plant.WateringFreq.IsEmpty()
		&& (!Plant.Maintenance.IsEmpty() || !Plant.CareLevel.IsEmpty())
		&& !Plant.Type.IsEmpty()
		&& !Plant.HardinessZones.IsEmpty()
		&& Plant.DaysToBloom > 0
		&& !Plant.Cycle.IsEmpty()
		&& (!Plant.GrowthRateText.IsEmpty() || Plant.GrowthRate > 0);
}

FBackendPlantDto USavedPlantCacheSubsystem::MergePlantDetails(const FBackendPlantDto& CachedPlant, const FBackendPlantDto& LoadedPlant)
{
	FBackendPlantDto MergedPlant = LoadedPlant;

	if (MergedPlant.Id <= 0)
	{
		MergedPlant.Id = CachedPlant.Id;
	}
	if (MergedPlant.PerenualId <= 0)
	{
		MergedPlant.PerenualId = CachedPlant.PerenualId;
	}
	if (MergedPlant.DaysToBloom <= 0)
	{
		MergedPlant.DaysToBloom = CachedPlant.DaysToBloom;
	}
	if (MergedPlant.ModelCategory.IsEmpty())
	{
		MergedPlant.ModelCategory = CachedPlant.ModelCategory;
	}
	if (MergedPlant.CommonName.IsEmpty())
	{
		MergedPlant.CommonName = CachedPlant.CommonName;
	}
	if (MergedPlant.ScientificName.IsEmpty())
	{
		MergedPlant.ScientificName = CachedPlant.ScientificName;
	}

	auto CopyImageIfEmpty = [](FString& Target, const FString& Source)
	{
		if (Target.IsEmpty())
		{
			Target = Source;
		}
	};

	CopyImageIfEmpty(MergedPlant.ImgSrcUrls.Original, CachedPlant.ImgSrcUrls.Original);
	CopyImageIfEmpty(MergedPlant.ImgSrcUrls.Regular, CachedPlant.ImgSrcUrls.Regular);
	CopyImageIfEmpty(MergedPlant.ImgSrcUrls.Medium, CachedPlant.ImgSrcUrls.Medium);
	CopyImageIfEmpty(MergedPlant.ImgSrcUrls.Small, CachedPlant.ImgSrcUrls.Small);
	CopyImageIfEmpty(MergedPlant.ImgSrcUrls.Thumbnail, CachedPlant.ImgSrcUrls.Thumbnail);

	return MergedPlant;
}

int32 USavedPlantCacheSubsystem::GetStablePlantKey(const FBackendPlantDto& Plant)
{
	return Plant.PerenualId > 0 ? Plant.PerenualId : Plant.Id;
}

TArray<FBackendPlantDto> USavedPlantCacheSubsystem::MergePlantsPreservingOrder(const TArray<FBackendPlantDto>& ExistingPlants, const TArray<FBackendPlantDto>& RefreshedPlants)
{
	if (ExistingPlants.Num() == 0 || RefreshedPlants.Num() <= 1)
	{
		return RefreshedPlants;
	}

	TMap<int32, FBackendPlantDto> RefreshedByKey;
	for (const FBackendPlantDto& Plant : RefreshedPlants)
	{
		const int32 Key = GetStablePlantKey(Plant);
		if (Key > 0)
		{
			RefreshedByKey.Add(Key, Plant);
		}
	}

	TArray<FBackendPlantDto> OrderedPlants;
	OrderedPlants.Reserve(RefreshedPlants.Num());

	TSet<int32> AddedKeys;
	for (const FBackendPlantDto& ExistingPlant : ExistingPlants)
	{
		const int32 Key = GetStablePlantKey(ExistingPlant);
		if (Key <= 0 || AddedKeys.Contains(Key))
		{
			continue;
		}

		if (const FBackendPlantDto* RefreshedPlant = RefreshedByKey.Find(Key))
		{
			OrderedPlants.Add(*RefreshedPlant);
			AddedKeys.Add(Key);
		}
	}

	for (const FBackendPlantDto& RefreshedPlant : RefreshedPlants)
	{
		const int32 Key = GetStablePlantKey(RefreshedPlant);
		if (Key <= 0 || !AddedKeys.Contains(Key))
		{
			OrderedPlants.Add(RefreshedPlant);
			if (Key > 0)
			{
				AddedKeys.Add(Key);
			}
		}
	}

	return OrderedPlants;
}

FString USavedPlantCacheSubsystem::GetPreferredImageUrl(const FBackendPlantDto& Plant) const
{
	if (!Plant.ImgSrcUrls.Regular.IsEmpty())
	{
		return Plant.ImgSrcUrls.Regular;
	}

	if (!Plant.ImgSrcUrls.Medium.IsEmpty())
	{
		return Plant.ImgSrcUrls.Medium;
	}

	if (!Plant.ImgSrcUrls.Small.IsEmpty())
	{
		return Plant.ImgSrcUrls.Small;
	}

	if (!Plant.ImgSrcUrls.Thumbnail.IsEmpty())
	{
		return Plant.ImgSrcUrls.Thumbnail;
	}

	return Plant.ImgSrcUrls.Original;
}

bool USavedPlantCacheSubsystem::HasCachedGardenPlants(int32 GardenId) const
{
	return CachedGardenPlants.Contains(GardenId);
}

const TArray<FBackendPlantDto>* USavedPlantCacheSubsystem::GetCachedGardenPlants(int32 GardenId) const
{
	return CachedGardenPlants.Find(GardenId);
}

TArray<FString> USavedPlantCacheSubsystem::GetImageUrlCandidates(const FBackendPlantDto& Plant) const
{
	TArray<FString> Urls;
	auto AddUrl = [&Urls](const FString& Url)
	{
		if (!Url.IsEmpty())
		{
			Urls.AddUnique(Url);
		}
	};

	AddUrl(Plant.ImgSrcUrls.Regular);
	AddUrl(Plant.ImgSrcUrls.Medium);
	AddUrl(Plant.ImgSrcUrls.Small);
	AddUrl(Plant.ImgSrcUrls.Thumbnail);
	AddUrl(Plant.ImgSrcUrls.Original);

	return Urls;
}

void USavedPlantCacheSubsystem::LogCachedPlants(const FString& CacheName, const TArray<FBackendPlantDto>& Plants) const
{
	UE_LOG(LogTemp, Warning, TEXT("=== Saved Plants Cached [%s]: Count = %d ==="), *CacheName, Plants.Num());

	for (const FBackendPlantDto& Plant : Plants)
	{
		const FString PreferredImageUrl = GetPreferredImageUrl(Plant);
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[%s Cache] Plant: Id=%d | PerenualId=%d | Name=%s | Scientific=%s | Category=%s | Regular=%s"),
			*CacheName,
			Plant.Id,
			Plant.PerenualId,
			*Plant.CommonName,
			*Plant.ScientificName,
			*Plant.ModelCategory,
			// PreferredImg=%s | 
			//*PreferredImageUrl,
			*Plant.ImgSrcUrls.Regular
			// | Medium=%s | Small=%s | Thumbnail=%s | Original=%s
			//*Plant.ImgSrcUrls.Medium,
			//*Plant.ImgSrcUrls.Small,
			//*Plant.ImgSrcUrls.Thumbnail,
			//*Plant.ImgSrcUrls.Original
		);
	}
}

void USavedPlantCacheSubsystem::ExecutePendingRefreshCallbacks(bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
{
	TArray<FOnSavedPlantsRefreshed> Callbacks = MoveTemp(PendingRefreshCallbacks);
	PendingRefreshCallbacks.Reset();

	for (const FOnSavedPlantsRefreshed& PendingCallback : Callbacks)
	{
		PendingCallback.ExecuteIfBound(bSuccess, Message, Plants);
	}
}

void USavedPlantCacheSubsystem::ExecutePendingGardenRefreshCallbacks(int32 GardenId, bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
{
	TArray<FOnSavedPlantsRefreshed> Callbacks;
	if (!PendingGardenRefreshCallbacks.RemoveAndCopyValue(GardenId, Callbacks))
	{
		return;
	}

	for (const FOnSavedPlantsRefreshed& PendingCallback : Callbacks)
	{
		PendingCallback.ExecuteIfBound(bSuccess, Message, Plants);
	}
}

void USavedPlantCacheSubsystem::RemoveCachedPlantByPerenualId(int32 PerenualId)
{
	CachedPlants.RemoveAll(
		[PerenualId](const FBackendPlantDto& Plant)
		{
			return Plant.PerenualId == PerenualId;
		}
	);

	PlantDetailsCache.Remove(PerenualId);

	for (TPair<int32, TArray<FBackendPlantDto>>& GardenCache : CachedGardenPlants)
	{
		GardenCache.Value.RemoveAll(
			[PerenualId](const FBackendPlantDto& Plant)
			{
				return Plant.PerenualId == PerenualId;
			}
		);
	}
}

void USavedPlantCacheSubsystem::ClearAllCaches()
{
	CachedPlants.Empty();
	CachedGardenPlants.Empty();
	PlantDetailsCache.Empty();
	TextureCache.Empty();
	PendingCallbacks.Empty();
	PendingPlantDetailsCallbacks.Empty();
	PendingRefreshCallbacks.Empty();
	PendingGardenRefreshCallbacks.Empty();
	GardenRefreshesInFlight.Empty();
	bHasLoadedSavedPlants = false;
	bRefreshInFlight = false;
}

void USavedPlantCacheSubsystem::GetOrLoadPlantDetails(int32 PerenualId, const FBackendPlantDetailsResponse& Callback)
{
	if (PerenualId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("PerenualId must be > 0"), FBackendPlantDto{});
		return;
	}

	FBackendPlantDto CachedPlantSnapshot;
	const bool bHasCachedPlant = PlantDetailsCache.Contains(PerenualId);
	if (const FBackendPlantDto* CachedPlant = PlantDetailsCache.Find(PerenualId))
	{
		CachedPlantSnapshot = *CachedPlant;
		if (HasCompletePopupDetails(*CachedPlant))
		{
			Callback.ExecuteIfBound(true, TEXT("Using cached plant details"), *CachedPlant);
			return;
		}
	}

	if (TArray<FBackendPlantDetailsResponse>* ExistingPending = PendingPlantDetailsCallbacks.Find(PerenualId))
	{
		ExistingPending->Add(Callback);
		return;
	}

	if (!GetGameInstance())
	{
		Callback.ExecuteIfBound(false, TEXT("No GameInstance"), FBackendPlantDto{});
		return;
	}

	UBackendApiSubsystem* Api = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Api)
	{
		Callback.ExecuteIfBound(false, TEXT("Backend API subsystem not found"), FBackendPlantDto{});
		return;
	}

	PendingPlantDetailsCallbacks.Add(PerenualId, { Callback });

	Api->GetPerenualPlantDetails(
		PerenualId,
		FBackendPlantDetailsResponse::CreateWeakLambda(this,
			[this, PerenualId, bHasCachedPlant, CachedPlantSnapshot](bool bSuccess, const FString& Message, const FBackendPlantDto& Plant)
			{
				if (!bSuccess)
				{
					FinishPlantDetailsWithFailure(PerenualId, Message);
					return;
				}

				FinishPlantDetailsWithSuccess(PerenualId, bHasCachedPlant ? MergePlantDetails(CachedPlantSnapshot, Plant) : Plant);
			}
		)
	);
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

void USavedPlantCacheSubsystem::FinishPlantDetailsWithFailure(int32 PerenualId, const FString& Message)
{
	TArray<FBackendPlantDetailsResponse> Callbacks;
	if (PendingPlantDetailsCallbacks.RemoveAndCopyValue(PerenualId, Callbacks))
	{
		const FBackendPlantDto EmptyPlant;
		for (const FBackendPlantDetailsResponse& Callback : Callbacks)
		{
			Callback.ExecuteIfBound(false, Message, EmptyPlant);
		}
	}
}

void USavedPlantCacheSubsystem::FinishPlantDetailsWithSuccess(int32 PerenualId, const FBackendPlantDto& Plant)
{
	CachePlantDetails(Plant);

	TArray<FBackendPlantDetailsResponse> Callbacks;
	if (PendingPlantDetailsCallbacks.RemoveAndCopyValue(PerenualId, Callbacks))
	{
		for (const FBackendPlantDetailsResponse& Callback : Callbacks)
		{
			Callback.ExecuteIfBound(true, TEXT("Plant details loaded"), Plant);
		}
	}
}
