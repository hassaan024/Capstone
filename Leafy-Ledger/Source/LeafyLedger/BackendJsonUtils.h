// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BackendApiTypes.h"

class FJsonObject;
class FJsonValue;

struct FBackendJsonUtils
{
	static bool ParseObject(const FString& JsonString, TSharedPtr<FJsonObject>& OutObject);
	static bool ParseValue(const FString& JsonString, TSharedPtr<FJsonValue>& OutValue);

	static FString StringifyObject(const TSharedRef<FJsonObject>& JsonObject);

	static bool TryGetErrorMessage(const FString& JsonString, FString& OutErrorMessage);
	static bool ParsePlantArray(const FString& JsonString, TArray<FBackendPlantDto>& OutPlants);
	static bool ParseCurrentUser(const FString& JsonString, FBackendUserDto& OutUser);
	static bool ParseUserLocation(const FString& JsonString, FBackendUserLocationDto& OutLocation);
	static bool ParseWeather(const FString& JsonString, FBackendWeatherDto& OutWeather);
};