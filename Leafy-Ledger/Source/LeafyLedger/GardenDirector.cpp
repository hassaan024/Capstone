// Fill out your copyright notice in the Description page of Project Settings.


#include "GardenDirector.h"
#include "SavedPlantCacheSubsystem.h"
#include "GardenSessionSubsystem.h"
#include "PlantSelect.h"
#include "PlantObject.h"
#include "GardenExit.h"
#include "UserDrone.h"
#include "Plant.h"
#include "GameFramework/PlayerController.h"

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
	UGardenExit* GardenExit = CreateWidget<UGardenExit>(PlayerController, GardenExitClass);
	GardenExit->AddToViewport();
}

void AGardenDirector::SpawnLoadedPlants()
{
	if (!GetGameInstance() || !GetWorld())
	{
		return;
	}

	UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();
	if (!GardenSession || !GardenSession->HasActiveDraft())
	{
		return;
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	AUserDrone* UserDrone = PlayerController ? Cast<AUserDrone>(PlayerController->GetPawn()) : nullptr;
	TSubclassOf<APlant> PlantClass = UserDrone ? UserDrone->GetDefaultPlantClass() : APlant::StaticClass();

	const FEditableGardenState& Draft = GardenSession->GetDraft();
	for (const FEditablePlantPlacement& PlantPlacement : Draft.Plants)
	{
		if (PlantPlacement.BackendPlantInstanceId <= 0)
		{
			continue;
		}

		FActorSpawnParameters SpawnParams;
		APlant* PlantActor = GetWorld()->SpawnActor<APlant>(
			PlantClass,
			PlantPlacement.Location,
			PlantPlacement.Rotation,
			SpawnParams
		);

		if (!PlantActor)
		{
			continue;
		}

		UPlantObject* PlantData = NewObject<UPlantObject>(this);
		if (PlantData)
		{
			PlantData->CommonName = PlantPlacement.SpeciesCommonName;
			PlantData->ScientificName = PlantPlacement.SpeciesScientificName;
			PlantData->PerenualId = PlantPlacement.PerenualId;
			PlantData->SpeciesId = PlantPlacement.SpeciesId;
			PlantData->ModelCategory = PlantPlacement.SpeciesModelCategory;
			PlantData->DaysToBloom = 4;
			PlantData->DaysToWither = 6;
			PlantActor->InitializeFromPlantData(PlantData);
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
	}
}

void AGardenDirector::MakePlantList()
{
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	UPlantSelect* PlantSelect = CreateWidget<UPlantSelect>(PlayerController, PlantSelectClass);
	PlantSelect->AddToViewport();

	UGameInstance* GI = GetGameInstance();
	USavedPlantCacheSubsystem* PlantCache = GI->GetSubsystem<USavedPlantCacheSubsystem>();

	// Use cached plants
	if (PlantCache->HasCachedPlants())
	{
		const TArray<FBackendPlantDto>& CachedPlants = PlantCache->GetCachedPlants();

		for (const FBackendPlantDto& Plant : CachedPlants)
		{
			PlantSelect->AddPlantToShelf(
				Plant.PerenualId,
				Plant.Id,
				Plant.CommonName,
				4, //Plant.DaysToBloom,
				6, //Plant.DaysToWither,
				Plant.ModelCategory
			);
		}
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

				PlantSelect->PlantShelf->ClearListItems();
				for (const FBackendPlantDto& Plant : Plants)
				{
					PlantSelect->AddPlantToShelf(
						Plant.PerenualId,
						Plant.Id,
						Plant.CommonName,
						4, //Plant.DaysToBloom,
						6, //Plant.DaysToWither,
						Plant.ModelCategory
					);
				}
			}
		)
	);
}
