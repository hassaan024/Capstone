// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Http.h"
#include "BackendApiTypes.h"
#include "BackendApiSubsystem.generated.h"

DECLARE_DELEGATE_TwoParams(FBackendOperationResponse, bool /*bSuccess*/, const FString& /*Message*/)
DECLARE_DELEGATE_ThreeParams(FBackendPlantsResponse, bool /*bSuccess*/, const FString& /*Message*/, const TArray<FBackendPlantDto>& /*Plants*/)
DECLARE_DELEGATE_ThreeParams(FBackendCurrentUserResponse, bool /*bSuccess*/, const FString& /*Message*/, const FBackendUserDto& /*User*/)
DECLARE_DELEGATE_ThreeParams(FBackendUserLocationResponse, bool /*bSuccess*/, const FString& /*Message*/, const FBackendUserLocationDto& /*Location*/)
DECLARE_DELEGATE_ThreeParams(FBackendWeatherResponse, bool /*bSuccess*/, const FString& /*Message*/, const FBackendWeatherDto& /*Weather*/)

UCLASS()
class LEAFYLEDGER_API UBackendApiSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backend")
	FString BaseUrl = TEXT("http://localhost:4000/backend");

	void GetSavedSpecies(const FBackendPlantsResponse& Callback);
	void DeleteSavedPlant(int32 PerenualId, const FBackendOperationResponse& Callback);
	void UpdateDisplayName(const FString& NewDisplayName, const FBackendOperationResponse& Callback);
	void GetCurrentUser(const FBackendCurrentUserResponse& Callback);
	void GetUserLocation(const FBackendUserLocationResponse& Callback);
	void GetCurrentWeather(float Latitude, float Longitude, const FBackendWeatherResponse& Callback);

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