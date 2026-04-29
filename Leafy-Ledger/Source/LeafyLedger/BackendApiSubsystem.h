// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Http.h"
#include "BackendApiTypes.h"
#include "BackendApiSubsystem.generated.h"

DECLARE_DELEGATE_TwoParams(FBackendOperationResponse, bool /*bSuccess*/, const FString& /*Message*/)
DECLARE_DELEGATE_ThreeParams(FBackendSoilIdResponse, bool /*bSuccess*/, const FString& /*Message*/, int32 /*SoilId*/)
DECLARE_DELEGATE_ThreeParams(FBackendPlantsResponse, bool /*bSuccess*/, const FString& /*Message*/, const TArray<FBackendPlantDto>& /*Plants*/)
DECLARE_DELEGATE_ThreeParams(FBackendCurrentUserResponse, bool /*bSuccess*/, const FString& /*Message*/, const FBackendUserDto& /*User*/)
DECLARE_DELEGATE_ThreeParams(FBackendUserLocationResponse, bool /*bSuccess*/, const FString& /*Message*/, const FBackendUserLocationDto& /*Location*/)
DECLARE_DELEGATE_ThreeParams(FBackendWeatherResponse, bool /*bSuccess*/, const FString& /*Message*/, const FBackendWeatherDto& /*Weather*/)
DECLARE_DELEGATE_FourParams(FBackendZipLocationResponse, bool /*bSuccess*/, const FString& /*Message*/, float /*Latitude*/, float /*Longitude*/)
DECLARE_DELEGATE_ThreeParams(FBackendGardenResponse, bool /*bSuccess*/, const FString& /*Message*/, const FBackendGardenDto& /*Garden*/)
DECLARE_DELEGATE_ThreeParams(FBackendPlantInstanceResponse, bool /*bSuccess*/, const FString& /*Message*/, const FBackendPlantInstanceDto& /*PlantInstance*/)
DECLARE_DELEGATE_ThreeParams(FBackendGardenSummariesResponse, bool /*bSuccess*/, const FString& /*Message*/, const TArray<FBackendGardenSummaryDto>& /*Gardens*/)
DECLARE_DELEGATE_ThreeParams(FBackendGardenDetailResponse, bool /*bSuccess*/, const FString& /*Message*/, const FBackendGardenDetailDto& /*Garden*/)
DECLARE_DELEGATE_ThreeParams(FBackendPlantSearchResponse, bool /*bSuccess*/, const FString& /*Message*/, const TArray<FBackendPlantSearchResultDto>& /*Plants*/)
DECLARE_DELEGATE_ThreeParams(FBackendPlantDetailsResponse, bool /*bSuccess*/, const FString& /*Message*/, const FBackendPlantDto& /*Plant*/)

UCLASS()
class LEAFYLEDGER_API UBackendApiSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backend")
	FString BaseUrl = TEXT("http://localhost:4000/backend");

	void SearchPerenualPlants(const FString& Query, const FBackendPlantSearchResponse& Callback);
	void GetPerenualPlantDetails(int32 PerenualId, const FBackendPlantDetailsResponse& Callback);
	void GetSavedSpecies(const FBackendPlantsResponse& Callback);
	void GetSavedSpeciesForGarden(int32 GardenId, const FBackendPlantsResponse& Callback);
	void SavePlant(int32 PerenualId, const FBackendOperationResponse& Callback);
	void SavePlantToGarden(int32 PerenualId, int32 GardenId, const FBackendOperationResponse& Callback);
	void DeleteSavedPlant(int32 PerenualId, const FBackendOperationResponse& Callback);
	void DeleteSavedPlantFromGarden(int32 PerenualId, int32 GardenId, const FBackendOperationResponse& Callback);
	void UpdateDisplayName(const FString& NewDisplayName, const FBackendOperationResponse& Callback);
	void GetCurrentUser(const FBackendCurrentUserResponse& Callback);
	void GetUserLocation(const FBackendUserLocationResponse& Callback);
	void ResolveZipCodeLocation(const FString& ZipCode, const FBackendZipLocationResponse& Callback);
	void GetCurrentWeather(float Latitude, float Longitude, const FBackendWeatherResponse& Callback);
	void GetGardensByUser(const FBackendGardenSummariesResponse& Callback);
	void GetGardenDetail(int32 GardenId, const FBackendGardenDetailResponse& Callback);
	void CreateGarden(const FString& Name, const FString& Description, const FString& BloomDate, float Latitude, float Longitude, const FString& Timezone, const FBackendGardenResponse& Callback);
	void UpdateGarden(int32 GardenId, const FString& Name, const FString& Description, const FString& BloomDate, float Latitude, float Longitude, const FString& Timezone, const FBackendGardenResponse& Callback);
	void DeleteGarden(int32 GardenId, const FBackendOperationResponse& Callback);
	void EnsureGenericSoil(const FBackendSoilIdResponse& Callback);
	void CreatePlantInstance(int32 GardenId, int32 SpeciesId, int32 SoilId, const FVector& Location, const FRotator& Rotation, const FVector& Scale, const float* HeightCm, const int32* AgeDays, const FString* HealthStatus, const FString* LastWateredIso8601, const FString* PlantedDateIso8601, const FString& Notes, const FBackendPlantInstanceResponse& Callback);
	void UpdatePlantInstance(int32 PlantInstanceId, const FVector& Location, const FRotator& Rotation, const FVector& Scale, float HeightCm, int32 AgeDays, const FString& HealthStatus, const FString& LastWateredIso8601, const FString& PlantedDateIso8601, const FString& Notes, const FBackendPlantInstanceResponse& Callback);

private:
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateRequest(
		const FString& Route,
		const FString& Verb,
		const FString& JsonBody = TEXT("")
	);

	void AddDefaultHeaders(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request);
	void AddAuthHeader(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request);

	bool TryGetUserId(int32& OutUserId, FString& OutError) const;
	static bool IsHttpSuccess(FHttpResponsePtr Response);
	static FString BuildErrorMessage(FHttpResponsePtr Response, const FString& Fallback);
};
