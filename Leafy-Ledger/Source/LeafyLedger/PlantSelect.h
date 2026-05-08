// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackendApiTypes.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "PlantSelect.generated.h"

class UPlantObject;
class UTextBlock;

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API UPlantSelect : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void AddPlantToShelf(int32 PerenualId, int32 SpeciesId, FString Name, FString ModelCategory, bool bIsDropdownToggle = false, bool bIsGlobalPlant = false);
	void SetGardenPlants(const TArray<FBackendPlantDto>& Plants);
	void SetGlobalPlants(const TArray<FBackendPlantDto>& Plants);
	void HandlePlantItemClicked(const UPlantObject* ClickedPlant);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UListView* PlantShelf;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* TXT_GardenName = nullptr;

protected:
	virtual void NativeConstruct() override;

private:
	TArray<FBackendPlantDto> GardenPlants;
	TArray<FBackendPlantDto> GlobalPlants;
	bool bAreGlobalPlantsVisible = false;

	void UpdateGardenNameText();
	void RebuildPlantShelf();
	bool ShouldSkipGlobalPlant(const FBackendPlantDto& Plant) const;
};
