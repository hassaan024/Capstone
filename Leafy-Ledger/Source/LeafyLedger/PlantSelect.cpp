// Fill out your copyright notice in the Description page of Project Settings.

#include "PlantSelect.h"
#include "PlantObject.h"

void UPlantSelect::AddPlantToShelf(int32 PerenualId, int32 SpeciesId, FString Name, int32 DaysToBloom, int32 DaysToWither, FString ModelCategory, bool bIsDropdownToggle, bool bIsGlobalPlant)
{
	UPlantObject* PlantObject = NewObject<UPlantObject>(this);

	PlantObject->PerenualId = PerenualId;
	PlantObject->SpeciesId = SpeciesId;
	PlantObject->CommonName = Name;
	PlantObject->DaysToBloom = DaysToBloom;
	PlantObject->DaysToWither = DaysToWither;
	PlantObject->ModelCategory = ModelCategory;
	PlantObject->bIsDropdownToggle = bIsDropdownToggle;
	PlantObject->bIsGlobalPlant = bIsGlobalPlant;

	PlantObject->SliderValue = 0.0f;

	PlantShelf->AddItem(PlantObject);
}

void UPlantSelect::SetGardenPlants(const TArray<FBackendPlantDto>& Plants)
{
	GardenPlants = Plants;
	RebuildPlantShelf();
}

void UPlantSelect::SetGlobalPlants(const TArray<FBackendPlantDto>& Plants)
{
	GlobalPlants = Plants;
	RebuildPlantShelf();
}

void UPlantSelect::HandlePlantItemClicked(const UPlantObject* ClickedPlant)
{
	if (!ClickedPlant || !ClickedPlant->bIsDropdownToggle) return;

	bAreGlobalPlantsVisible = !bAreGlobalPlantsVisible;
	RebuildPlantShelf();
}

void UPlantSelect::RebuildPlantShelf()
{
	if (!PlantShelf) return;

	PlantShelf->ClearListItems();

	for (const FBackendPlantDto& Plant : GardenPlants)
	{
		AddPlantToShelf(
			Plant.PerenualId,
			Plant.Id,
			Plant.CommonName,
			4,
			6,
			Plant.ModelCategory
		);
	}

	int32 UniqueGlobalPlantCount = 0;
	for (const FBackendPlantDto& Plant : GlobalPlants)
	{
		if (!ShouldSkipGlobalPlant(Plant))
		{
			++UniqueGlobalPlantCount;
		}
	}

	if (UniqueGlobalPlantCount <= 0) return;

	AddPlantToShelf(
		-1,
		-1,
		bAreGlobalPlantsVisible ? TEXT("Hide global plants") : TEXT("Show global plants"),
		0,
		0,
		TEXT(""),
		true,
		false
	);

	if (!bAreGlobalPlantsVisible) return;

	for (const FBackendPlantDto& Plant : GlobalPlants)
	{
		if (ShouldSkipGlobalPlant(Plant)) continue;

		AddPlantToShelf(
			Plant.PerenualId,
			Plant.Id,
			Plant.CommonName,
			4,
			6,
			Plant.ModelCategory,
			false,
			true
		);
	}
}

bool UPlantSelect::ShouldSkipGlobalPlant(const FBackendPlantDto& Plant) const
{
	auto MatchesPlant = [&Plant](const FBackendPlantDto& ExistingPlant)
	{
		const bool bMatchingPerenual = Plant.PerenualId > 0 && ExistingPlant.PerenualId == Plant.PerenualId;
		const bool bMatchingSpecies = Plant.Id > 0 && ExistingPlant.Id == Plant.Id;
		return bMatchingPerenual || bMatchingSpecies;
	};

	if (GardenPlants.ContainsByPredicate(MatchesPlant))
	{
		return true;
	}

	for (const FBackendPlantDto& ExistingGlobalPlant : GlobalPlants)
	{
		if (&ExistingGlobalPlant == &Plant)
		{
			break;
		}

		if (MatchesPlant(ExistingGlobalPlant))
		{
			return true;
		}
	}

	return false;
}
