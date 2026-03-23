#pragma once

#include "CoreMinimal.h"

struct FBackendPlantDto
{
	FString CommonName;
	FString ScientificName;
	FString ImgSrcUrl;
	int32 TrefleId = 0;
};

struct FBackendUserDto
{
	int32 Id = 0;
	FString DisplayName;
	FString GoogleDisplayName;
	bool bConfirmedName = false;
};

struct FBackendUserLocationDto
{
	double Latitude = 0.0;
	double Longitude = 0.0;
	FString UpdatedAt;

	bool bHasLatitude = false;
	bool bHasLongitude = false;
};

struct FBackendWeatherDto
{
	float Temperature2m = 0.f;
	FString Description;
	bool bHasTemperature2m = false;
};