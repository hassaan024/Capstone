#pragma once

#include "CoreMinimal.h"
#include "BackendApiTypes.generated.h"

USTRUCT(BlueprintType)
struct FBackendPlantImageUrlsDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Original;
	UPROPERTY(BlueprintReadWrite)
	FString Regular;
	UPROPERTY(BlueprintReadWrite)
	FString Medium;
	UPROPERTY(BlueprintReadWrite)
	FString Small;
	UPROPERTY(BlueprintReadWrite)
	FString Thumbnail;
};

USTRUCT(BlueprintType)
struct FBackendPlantAnatomyDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Part;
	UPROPERTY(BlueprintReadWrite)
	FString Description;
};

USTRUCT(BlueprintType)
struct FBackendPlantDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id = 0;
	UPROPERTY(BlueprintReadWrite)
	int32 PerenualId = 0;

	UPROPERTY(BlueprintReadWrite)
	FString CommonName;
	UPROPERTY(BlueprintReadWrite)
	FString ScientificName;
	UPROPERTY(BlueprintReadWrite)
	int32 GrowthRate = 0;
	UPROPERTY(BlueprintReadWrite)
	FString GrowthRateText;
	UPROPERTY(BlueprintReadWrite)
	FString Cycle;
	UPROPERTY(BlueprintReadWrite)
	FString Type;
	UPROPERTY(BlueprintReadWrite)
	FString Maintenance;
	UPROPERTY(BlueprintReadWrite)
	FString CareLevel;
	UPROPERTY(BlueprintReadWrite)
	int32 AvgHoursSun = 0;
	UPROPERTY(BlueprintReadWrite)
	FString SunlightText;
	UPROPERTY(BlueprintReadWrite)
	FString HardinessZones;
	UPROPERTY(BlueprintReadWrite)
	int32 DaysToBloom = 0;

	UPROPERTY(BlueprintReadWrite)
	float MinTemp = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	float MaxTemp = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	float MinHeight = 0.0f;
	UPROPERTY(BlueprintReadWrite)
	float MaxHeight = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	FString WateringFreq;
	UPROPERTY(BlueprintReadWrite)
	int32 WateringMinDays = 0;
	UPROPERTY(BlueprintReadWrite)
	int32 WateringMaxDays = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> OtherNames;
	UPROPERTY(BlueprintReadWrite)
	FString Family;
	UPROPERTY(BlueprintReadWrite)
	FString Genus;
	UPROPERTY(BlueprintReadWrite)
	FString SpeciesEpithet;
	UPROPERTY(BlueprintReadWrite)
	TArray<FString> Origin;

	UPROPERTY(BlueprintReadWrite)
	bool bFlowers = false;
	UPROPERTY(BlueprintReadWrite)
	FString FloweringSeason;
	UPROPERTY(BlueprintReadWrite)
	bool bFruits = false;
	UPROPERTY(BlueprintReadWrite)
	bool bEdibleFruit = false;
	UPROPERTY(BlueprintReadWrite)
	FString HarvestSeason;
	UPROPERTY(BlueprintReadWrite)
	bool bLeaf = false;
	UPROPERTY(BlueprintReadWrite)
	bool bEdibleLeaf = false;
	UPROPERTY(BlueprintReadWrite)
	bool bCuisine = false;
	UPROPERTY(BlueprintReadWrite)
	bool bMedicinal = false;
	UPROPERTY(BlueprintReadWrite)
	bool bDroughtTolerant = false;
	UPROPERTY(BlueprintReadWrite)
	bool bSaltTolerant = false;
	UPROPERTY(BlueprintReadWrite)
	bool bTropical = false;
	UPROPERTY(BlueprintReadWrite)
	bool bIndoor = false;
	UPROPERTY(BlueprintReadWrite)
	bool bThorny = false;
	UPROPERTY(BlueprintReadWrite)
	bool bInvasive = false;

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> PruningMonths;
	UPROPERTY(BlueprintReadWrite)
	FString PruningFrequency;
	UPROPERTY(BlueprintReadWrite)
	FString PruningInterval;

	UPROPERTY(BlueprintReadWrite)
	TArray<FBackendPlantAnatomyDto> PlantAnatomy;
	UPROPERTY(BlueprintReadWrite)
	FBackendPlantImageUrlsDto ImgSrcUrls;
	UPROPERTY(BlueprintReadWrite)
	FString ModelCategory;
};

USTRUCT(BlueprintType)
struct FBackendPlantSearchResultDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 PerenualId = 0;

	UPROPERTY(BlueprintReadWrite)
	FString CommonName;

	UPROPERTY(BlueprintReadWrite)
	FString ScientificName;

	UPROPERTY(BlueprintReadWrite)
	FString ImageUrl;
};

USTRUCT(BlueprintType)
struct FBackendUserDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id = 0;
	UPROPERTY(BlueprintReadWrite)
	FString DisplayName;
	UPROPERTY(BlueprintReadWrite)
	FString GoogleDisplayName;
	UPROPERTY(BlueprintReadWrite)
	bool bConfirmedName = false;
};

USTRUCT(BlueprintType)
struct FBackendUserLocationDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float Latitude = 0.0;
	UPROPERTY(BlueprintReadWrite)
	float Longitude = 0.0;
	UPROPERTY(BlueprintReadWrite)
	FString UpdatedAt;

	UPROPERTY(BlueprintReadWrite)
	bool bHasLatitude = false;
	UPROPERTY(BlueprintReadWrite)
	bool bHasLongitude = false;
};

USTRUCT(BlueprintType)
struct FBackendWeatherDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float Temperature2m = 0.f;
	UPROPERTY(BlueprintReadWrite)
	FString Description;
	UPROPERTY(BlueprintReadWrite)
	bool bHasTemperature2m = false;
};

USTRUCT(BlueprintType)
struct FBackendGardenDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 OwnerId = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FString Description;

	UPROPERTY(BlueprintReadWrite)
	FString BloomDate;

	UPROPERTY(BlueprintReadWrite)
	FString PaintMaskData;

	UPROPERTY(BlueprintReadWrite)
	float Latitude = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float Longitude = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FString Timezone;

	UPROPERTY(BlueprintReadWrite)
	FString CreationTimestamp;

	UPROPERTY(BlueprintReadWrite)
	FString LastUpdated;
};

USTRUCT(BlueprintType)
struct FBackendGardenSummaryDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FString Description;

	UPROPERTY(BlueprintReadWrite)
	float Latitude = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float Longitude = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FString Timezone;

	UPROPERTY(BlueprintReadWrite)
	int32 PlantCount = 0;
};

USTRUCT(BlueprintType)
struct FBackendPlantInstanceDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 GardenId = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 SpeciesId = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 SoilId = 0;

	UPROPERTY(BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	UPROPERTY(BlueprintReadWrite)
	float HeightCm = 0.f;

	UPROPERTY(BlueprintReadWrite)
	int32 AgeDays = 0;

	UPROPERTY(BlueprintReadWrite)
	FString HealthStatus;

	UPROPERTY(BlueprintReadWrite)
	FString LastWatered;

	UPROPERTY(BlueprintReadWrite)
	FString PlantedDate;

	UPROPERTY(BlueprintReadWrite)
	FString Notes;

	UPROPERTY(BlueprintReadWrite)
	FString CreationTimestamp;

	UPROPERTY(BlueprintReadWrite)
	FString LastUpdated;

	UPROPERTY(BlueprintReadWrite)
	bool bHasHeightCm = false;

	UPROPERTY(BlueprintReadWrite)
	bool bHasAgeDays = false;

	UPROPERTY(BlueprintReadWrite)
	bool bHasHealthStatus = false;

	UPROPERTY(BlueprintReadWrite)
	bool bHasLastWatered = false;

	UPROPERTY(BlueprintReadWrite)
	bool bHasPlantedDate = false;
};

USTRUCT(BlueprintType)
struct FBackendGardenPlantInstanceDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 GardenId = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 SpeciesId = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 SoilId = 0;

	UPROPERTY(BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	UPROPERTY(BlueprintReadWrite)
	float HeightCm = 0.f;

	UPROPERTY(BlueprintReadWrite)
	int32 AgeDays = 0;

	UPROPERTY(BlueprintReadWrite)
	FString HealthStatus;

	UPROPERTY(BlueprintReadWrite)
	FString LastWatered;

	UPROPERTY(BlueprintReadWrite)
	FString PlantedDate;

	UPROPERTY(BlueprintReadWrite)
	FString Notes;

	UPROPERTY(BlueprintReadWrite)
	FBackendPlantDto Species;
};

USTRUCT(BlueprintType)
struct FBackendGardenDetailDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 OwnerId = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FString Description;

	UPROPERTY(BlueprintReadWrite)
	FString BloomDate;

	UPROPERTY(BlueprintReadWrite)
	FString PaintMaskData;

	UPROPERTY(BlueprintReadWrite)
	float Latitude = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float Longitude = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FString Timezone;

	UPROPERTY(BlueprintReadWrite)
	TArray<FBackendGardenPlantInstanceDto> Plants;
};

USTRUCT(BlueprintType)
struct FBackendGardenTimelinePlantDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 PlantInstanceId = 0;

	UPROPERTY(BlueprintReadWrite)
	FString SpeciesName;

	UPROPERTY(BlueprintReadWrite)
	bool bFeasible = false;

	UPROPERTY(BlueprintReadWrite)
	FString FeasibilityNote;

	UPROPERTY(BlueprintReadWrite)
	FString PlantedDate;

	UPROPERTY(BlueprintReadWrite)
	int32 DaysToMature = 0;

	UPROPERTY(BlueprintReadWrite)
	float MaxHeightCm = 0.f;
};

USTRUCT(BlueprintType)
struct FBackendGardenTimelineDto
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 GardenId = 0;

	UPROPERTY(BlueprintReadWrite)
	FString BloomDate;

	UPROPERTY(BlueprintReadWrite)
	TArray<FBackendGardenTimelinePlantDto> Plants;
};
