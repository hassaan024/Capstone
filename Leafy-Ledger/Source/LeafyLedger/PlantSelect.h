// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "GardenDirector.h"
#include "PlantSelect.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API UPlantSelect : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void AddPlantToShelf(int32 Id, FString Name, int32 DaysToBloom, int32 DaysToWither, FString ModelCategory);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UListView* PlantShelf;
};
