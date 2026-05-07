// Fill out your copyright notice in the Description page of Project Settings.

#include "GardenDirector.h"
#include "BackendApiSubsystem.h"
#include "BloomDateUtils.h"
#include "SavedPlantCacheSubsystem.h"
#include "GardenSessionSubsystem.h"
#include "GardenTimeManager.h"
#include "PlantSelect.h"
#include "PlantObject.h"
#include "GardenHUD.h"
#include "UserDrone.h"
#include "Plant.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	bool TryDateToDayIndex(const FString& DateText, int32& OutDayIndex)
	{
		const FString NormalizedDate = FBloomDateUtils::NormalizeBackendDateString(DateText);
		if (NormalizedDate.IsEmpty())
		{
			return false;
		}

		FDateTime ParsedDate;
		if (!FDateTime::ParseIso8601(*NormalizedDate, ParsedDate))
		{
			return false;
		}

		const FDateTime DateOnly(ParsedDate.GetYear(), ParsedDate.GetMonth(), ParsedDate.GetDay());
		const FDateTime EpochDate(2000, 1, 1);
		OutDayIndex = static_cast<int32>((DateOnly - EpochDate).GetDays());
		return OutDayIndex >= 0;
	}

	int32 GetFallbackDaysToBloom(const FString& Category)
	{
		if (Category.Equals(TEXT("flower"), ESearchCase::IgnoreCase))
		{
			return 20;
		}

		if (Category.Equals(TEXT("tree"), ESearchCase::IgnoreCase))
		{
			return 50;
		}

		return 30;
	}

	bool IsGardenDateSliderVisible(UWorld* World)
	{
		if (!World)
		{
			return false;
		}

		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, UGardenHUD::StaticClass(), false);
		for (UUserWidget* Widget : FoundWidgets)
		{
			if (const UGardenHUD* GardenHUD = Cast<UGardenHUD>(Widget))
			{
				return GardenHUD->IsDateSliderVisible();
			}
		}

		return false;
	}
}

// Sets default values
AGardenDirector::AGardenDirector()
{
 	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGardenDirector::BeginPlay()
{
	Super::BeginPlay();
	MakePlantList();
	AddButtons();

	if (!GetGameInstance()) return;

	UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();

	if (!GardenSession || !GardenSession->HasActiveDraft())
	{
		UE_LOG(LogTemp, Warning, TEXT("No active garden draft found"));
		return;
	}

	const FEditableGardenState& Draft = GardenSession->GetDraft();
	if (Draft.BackendGardenId > 0 && Draft.Plants.Num() > 0)
	{
		SpawnLoadedPlants();
	}
}

// Called every frame
void AGardenDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGardenDirector::AddButtons()
{
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	UGardenHUD* GardenHUD = CreateWidget<UGardenHUD>(PlayerController, GardenHUDClass);
	GardenHUD->AddToViewport();
}

void AGardenDirector::SpawnLoadedPlants()
{
	if (!GetGameInstance() || !GetWorld()) return;

	UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();
	if (!GardenSession || !GardenSession->HasActiveDraft()) return;

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	AUserDrone* UserDrone = PlayerController ? Cast<AUserDrone>(PlayerController->GetPawn()) : nullptr;
	TSubclassOf<APlant> PlantClass = UserDrone ? UserDrone->GetDefaultPlantClass() : APlant::StaticClass();

	const FEditableGardenState& Draft = GardenSession->GetDraft();
	const bool bDateSliderVisible = IsGardenDateSliderVisible(GetWorld());
	int32 FinalBloomDayIndex = -1;
	const bool bHasFinalBloomDayIndex = TryDateToDayIndex(Draft.BloomDate, FinalBloomDayIndex);
	AGardenTimeManager* TimeManager = nullptr;
	TArray<AActor*> FoundTimeManagers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGardenTimeManager::StaticClass(), FoundTimeManagers);
	if (FoundTimeManagers.Num() > 0)
	{
		TimeManager = Cast<AGardenTimeManager>(FoundTimeManagers[0]);
	}

	for (const FEditablePlantPlacement& PlantPlacement : Draft.Plants)
	{
		if (PlantPlacement.BackendPlantInstanceId <= 0) continue;

		FActorSpawnParameters SpawnParams;
		APlant* PlantActor = GetWorld()->SpawnActor<APlant>(
			PlantClass,
			PlantPlacement.Location,
			PlantPlacement.Rotation,
			SpawnParams
		);

		if (!PlantActor) continue;

		UPlantObject* PlantData = NewObject<UPlantObject>(this);
		if (PlantData)
		{
			PlantData->CommonName = PlantPlacement.SpeciesCommonName;
			PlantData->ScientificName = PlantPlacement.SpeciesScientificName;
			PlantData->PerenualId = PlantPlacement.PerenualId;
			PlantData->SpeciesId = PlantPlacement.SpeciesId;
			PlantData->ModelCategory = PlantPlacement.SpeciesModelCategory;
			const FString EffectiveBloomDate = !PlantPlacement.BloomDate.IsEmpty() ? PlantPlacement.BloomDate : Draft.BloomDate;
			int32 BloomDayIndex = -1;
			const bool bHasBloomDayIndex = TryDateToDayIndex(EffectiveBloomDate, BloomDayIndex);
			int32 PlantingDayIndex = -1;
			const bool bHasPlantingDayIndex = bHasBloomDayIndex
				&& TryDateToDayIndex(PlantPlacement.PlantedDate, PlantingDayIndex)
				&& PlantingDayIndex <= BloomDayIndex;
			if (bHasPlantingDayIndex)
			{
				PlantData->DaysToBloom = FMath::Max(1, BloomDayIndex - PlantingDayIndex);
			}
			else
			{
				PlantData->DaysToBloom = GetFallbackDaysToBloom(PlantPlacement.SpeciesModelCategory);
			}
			PlantData->DaysToWither = APlant::CalculateDaysToWitherFromBloom(PlantData->DaysToBloom);
			PlantActor->InitializeFromPlantData(PlantData);

			if (bHasBloomDayIndex)
			{
				PlantActor->BloomDayIndex = bHasFinalBloomDayIndex
					? FMath::Min(BloomDayIndex, FinalBloomDayIndex)
					: BloomDayIndex;

				if (bHasPlantingDayIndex)
				{
					PlantActor->PlantingDayIndex = PlantingDayIndex;
					PlantActor->BloomDayIndex = FMath::Max(PlantActor->BloomDayIndex, PlantingDayIndex);
					PlantActor->DaysToBloom = FMath::Max(1, PlantActor->BloomDayIndex - PlantActor->PlantingDayIndex);
					PlantActor->DaysToWither = APlant::CalculateDaysToWitherFromBloom(PlantActor->DaysToBloom);
					PlantActor->WitherDayIndex = PlantActor->BloomDayIndex + PlantActor->DaysToWither;
					PlantActor->PlantingDate = FBloomDateUtils::DayIndexToDisplayDate(PlantingDayIndex);
				}

				PlantActor->UpdatePlantingDateText();
			}
		}

		PlantActor->SetActorRotation(PlantPlacement.Rotation);
		PlantActor->SetActorScale3D(PlantPlacement.Scale);
		PlantActor->SetActorLocation(PlantPlacement.Location);
		PlantActor->LocalPlantId = PlantPlacement.LocalId;
		PlantActor->bIsTrackedInGardenDraft = true;
		PlantActor->HeightCm = PlantPlacement.HeightCm;
		PlantActor->AgeDays = PlantPlacement.AgeDays;
		PlantActor->HealthStatus = PlantPlacement.HealthStatus;
		PlantActor->LastWateredIso8601 = PlantPlacement.LastWateredIso8601;
		PlantActor->Notes = PlantPlacement.Notes;
		PlantActor->SetPlacedMaterial();

		if (TimeManager)
		{
			PlantActor->UpdateForDay(TimeManager->GetCurrentDayIndex());
		}

		PlantActor->SetPlantingDateTextVisible(bDateSliderVisible);
	}
}

void AGardenDirector::MakePlantList()
{
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("MakePlantList: PlayerController is missing"));
		return;
	}

	if (!PlantSelectClass)
	{
		UE_LOG(LogTemp, Error, TEXT("MakePlantList: PlantSelectClass is not set on GardenDirector"));
		return;
	}

	UPlantSelect* PlantSelect = CreateWidget<UPlantSelect>(PlayerController, PlantSelectClass);
	if (!PlantSelect)
	{
		UE_LOG(LogTemp, Error, TEXT("MakePlantList: failed to create PlantSelect"));
		return;
	}

	PlantSelect->AddToViewport();

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UGardenSessionSubsystem* GardenSession = GI->GetSubsystem<UGardenSessionSubsystem>();
	const int32 ActiveGardenId = (GardenSession && GardenSession->HasActiveDraft()) ? GardenSession->GetDraft().BackendGardenId : 0;
	USavedPlantCacheSubsystem* PlantCache = GI->GetSubsystem<USavedPlantCacheSubsystem>();

	if (ActiveGardenId > 0)
	{
		if (PlantCache && PlantCache->HasCachedPlants())
		{
			PlantSelect->SetGlobalPlants(PlantCache->GetCachedPlants());
		}

		if (PlantCache)
		{
			PlantCache->RefreshSavedPlants(
				FOnSavedPlantsRefreshed::CreateWeakLambda(
					PlantSelect,
					[PlantSelect](bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
					{
						if (!PlantSelect) return;

						if (!bSuccess)
						{
							UE_LOG(LogTemp, Error, TEXT("MakePlantList global refresh failed: %s"), *Message);
							return;
						}

						PlantSelect->SetGlobalPlants(Plants);
					}
				)
			);
		}

		if (!PlantCache)
		{
			UE_LOG(LogTemp, Error, TEXT("MakePlantList: SavedPlantCacheSubsystem missing"));
			return;
		}

		if (const TArray<FBackendPlantDto>* CachedGardenPlants = PlantCache->GetCachedGardenPlants(ActiveGardenId))
		{
			PlantSelect->SetGardenPlants(*CachedGardenPlants);
		}

		PlantCache->RefreshGardenSavedPlants(
			ActiveGardenId,
			FOnSavedPlantsRefreshed::CreateWeakLambda(
				PlantSelect,
				[PlantSelect, ActiveGardenId](bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
				{
					if (!PlantSelect) return;

					if (!bSuccess)
					{
						UE_LOG(LogTemp, Error, TEXT("MakePlantList garden refresh failed for garden %d: %s"), ActiveGardenId, *Message);
						return;
					}

					PlantSelect->SetGardenPlants(Plants);
				}
			)
		);
		return;
	}

	if (!PlantCache)
	{
		UE_LOG(LogTemp, Error, TEXT("MakePlantList: SavedPlantCacheSubsystem missing"));
		return;
	}

	// Use cached plants
	if (PlantCache->HasCachedPlants())
	{
		PlantSelect->SetGardenPlants(PlantCache->GetCachedPlants());
	}

	// Check for non-cached plants and refresh if any
	PlantCache->RefreshSavedPlants(
		FOnSavedPlantsRefreshed::CreateWeakLambda(
			PlantSelect,
			[PlantSelect](bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
			{
				if (!PlantSelect) return;

				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("MakePlantList refresh failed: %s"), *Message);
					return;
				}

				PlantSelect->SetGardenPlants(Plants);
			}
		)
	);
}
