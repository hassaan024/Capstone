// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Texture2D.h"
#include "BackendApiSubsystem.h"
#include "SavedPlantCacheSubsystem.generated.h"

DECLARE_DELEGATE_OneParam(FOnPlantImageReady, UTexture2D*)
DECLARE_DELEGATE_ThreeParams(FOnSavedPlantsRefreshed, bool /*bSuccess*/, const FString& /*Message*/, const TArray<FBackendPlantDto>& /*Plants*/)

UCLASS()
class LEAFYLEDGER_API USavedPlantCacheSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void WarmAfterLogin();
	void RefreshSavedPlants(const FOnSavedPlantsRefreshed& Callback);
	void RefreshGardenSavedPlants(int32 GardenId, const FOnSavedPlantsRefreshed& Callback);

	const TArray<FBackendPlantDto>& GetCachedPlants() const { return CachedPlants; }
	bool HasCachedPlants() const { return bHasLoadedSavedPlants; }
	bool HasCachedGardenPlants(int32 GardenId) const;
	const TArray<FBackendPlantDto>* GetCachedGardenPlants(int32 GardenId) const;

	void RemoveCachedPlantByPerenualId(int32 PerenualId);

	void GetOrLoadPlantDetails(int32 PerenualId, const FBackendPlantDetailsResponse& Callback);
	void GetOrLoadImage(const FString& Url, const FOnPlantImageReady& Callback);

	UFUNCTION(BlueprintCallable, Category = "Plants")
	void ClearAllCaches();

private:
	UPROPERTY()
	TArray<FBackendPlantDto> CachedPlants;

	TMap<int32, TArray<FBackendPlantDto>> CachedGardenPlants;
	TMap<int32, FBackendPlantDto> PlantDetailsCache;

	UPROPERTY()
	TMap<FString, UTexture2D*> TextureCache;

	TMap<FString, TArray<FOnPlantImageReady>> PendingCallbacks;
	TMap<int32, TArray<FBackendPlantDetailsResponse>> PendingPlantDetailsCallbacks;
	TArray<FOnSavedPlantsRefreshed> PendingRefreshCallbacks;
	TMap<int32, TArray<FOnSavedPlantsRefreshed>> PendingGardenRefreshCallbacks;
	TSet<int32> GardenRefreshesInFlight;

	bool bHasLoadedSavedPlants = false;
	bool bRefreshInFlight = false;

	void PrefetchPlantImages(const TArray<FBackendPlantDto>& Plants);
	void WarmAllGardenSavedPlantCaches();
	void CachePlantDetails(const FBackendPlantDto& Plant);
	void CachePlantDetails(const TArray<FBackendPlantDto>& Plants);
	FString GetPreferredImageUrl(const FBackendPlantDto& Plant) const;
	TArray<FString> GetImageUrlCandidates(const FBackendPlantDto& Plant) const;
	void LogCachedPlants(const FString& CacheName, const TArray<FBackendPlantDto>& Plants) const;
	void ExecutePendingRefreshCallbacks(bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants);
	void ExecutePendingGardenRefreshCallbacks(int32 GardenId, bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants);

	void FinishWithFailure(const FString& Url);
	void FinishWithSuccess(const FString& Url, UTexture2D* Texture);
	void FinishPlantDetailsWithFailure(int32 PerenualId, const FString& Message);
	void FinishPlantDetailsWithSuccess(int32 PerenualId, const FBackendPlantDto& Plant);
};
