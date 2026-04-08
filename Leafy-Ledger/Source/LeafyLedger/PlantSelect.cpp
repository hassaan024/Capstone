// Fill out your copyright notice in the Description page of Project Settings.

#include "PlantSelect.h"
#include "PlantObject.h"

void UPlantSelect::AddPlantToShelf(FString Name, int32 DaysToBloom, int32 DaysToWither, FString ModelCategory)
{
	UPlantObject* PlantObject = NewObject<UPlantObject>(this);

	PlantObject->CommonName = Name;
	PlantObject->DaysToBloom = DaysToBloom;
	PlantObject->DaysToWither = DaysToWither;
	PlantObject->ModelCategory = ModelCategory;

	PlantObject->SliderValue = 0.0f;

	PlantShelf->AddItem(PlantObject);
}
