// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Texture2D.h"
#include "BackendApiTypes.h"
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

	const TArray<FBackendPlantDto>& GetCachedPlants() const { return CachedPlants; }
	bool HasCachedPlants() const { return bHasLoadedSavedPlants; }

	void RemoveCachedPlantByPerenualId(int32 PerenualId);

	void GetOrLoadImage(const FString& Url, const FOnPlantImageReady& Callback);

	UFUNCTION(BlueprintCallable, Category = "Plants")
	void ClearAllCaches();

private:
	UPROPERTY()
	TArray<FBackendPlantDto> CachedPlants;

	UPROPERTY()
	TMap<FString, UTexture2D*> TextureCache;

	TMap<FString, TArray<FOnPlantImageReady>> PendingCallbacks;

	bool bHasLoadedSavedPlants = false;
	bool bRefreshInFlight = false;

	void PrefetchPlantImages(const TArray<FBackendPlantDto>& Plants);
	FString GetPreferredImageUrl(const FBackendPlantDto& Plant) const;

	void FinishWithFailure(const FString& Url);
	void FinishWithSuccess(const FString& Url, UTexture2D* Texture);
};