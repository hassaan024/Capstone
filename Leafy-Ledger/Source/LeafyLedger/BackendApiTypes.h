#pragma once

#include "CoreMinimal.h"

struct FBackendPlantImageUrlsDto
{
	FString Regular;
};

struct FBackendPlantAnatomyDto
{
	FString Part;
	FString Description;
};

struct FBackendPlantDto
{
	int32 Id = 0;
	int32 PerenualId = 0;

	FString CommonName;
	FString ScientificName;
	int32 GrowthRate = 0;
	FString Cycle;
	FString Type;
	FString Maintenance;
	FString CareLevel;
	int32 AvgHoursSun = 0;

	float MinTemp = 0.0f;
	float MaxTemp = 0.0f;
	float MinHeight = 0.0f;
	float MaxHeight = 0.0f;

	FString WateringFreq;
	int32 WateringMinDays = 0;
	int32 WateringMaxDays = 0;

	TArray<FString> OtherNames;
	FString Family;
	FString Genus;
	FString SpeciesEpithet;
	TArray<FString> Origin;

	bool bFlowers = false;
	FString FloweringSeason;
	bool bFruits = false;
	bool bEdibleFruit = false;
	FString HarvestSeason;
	bool bLeaf = false;
	bool bEdibleLeaf = false;
	bool bCuisine = false;
	bool bMedicinal = false;
	bool bDroughtTolerant = false;
	bool bSaltTolerant = false;
	bool bTropical = false;
	bool bIndoor = false;
	bool bThorny = false;
	bool bInvasive = false;

	TArray<FString> PruningMonths;
	FString PruningFrequency;
	FString PruningInterval;

	TArray<FBackendPlantAnatomyDto> PlantAnatomy;
	FBackendPlantImageUrlsDto ImgSrcUrls;
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