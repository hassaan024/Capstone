// Fill out your copyright notice in the Description page of Project Settings.


#include "GardenDirector.h"
#include "SavedPlantCacheSubsystem.h"
#include "PlantSelect.h"
#include "PlantObject.h"

// Sets default values
AGardenDirector::AGardenDirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGardenDirector::BeginPlay()
{
	Super::BeginPlay();
	MakePlantList();
}

// Called every frame
void AGardenDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
				Plant.CommonName,
				4,
				6,
				//Plant.DaysToBloom,
				//Plant.DaysToWither,
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
				if (!PlantSelect)
				{
					return;
				}

				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("MakePlantList refresh failed: %s"), *Message);
					return;
				}

				PlantSelect->PlantShelf->ClearListItems();
				for (const FBackendPlantDto& Plant : Plants)
				{
					PlantSelect->AddPlantToShelf(
						Plant.CommonName,
						4,
						6,
						//Plant.DaysToBloom,
						//Plant.DaysToWither,
						Plant.ModelCategory
					);
				}
			}
		)
	);
}