// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Components/TileView.h"
#include "BackendApiTypes.h"
#include "SavedPlants.generated.h"

class UPlantObject;
class UUserWidget;

UCLASS()
class LEAFYLEDGER_API USavedPlants : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Plants")
	void DeleteSavedPlant(int32 TrefleId);

	UFUNCTION(BlueprintCallable, Category = "Plants")
	void RemoveSavedPlantFromList(int32 TrefleId);

	UFUNCTION()
	void HandleRemoveClicked(int32 TrefleId);

	void HandleEntryGenerated(UUserWidget& EntryWidget);

	UPROPERTY(meta = (BindWidget))
	UTileView* TV_PlantCards;

protected:
	virtual void NativeConstruct() override;

private:
	void FetchSavedSpecies();
	void HandleFetchSavedSpeciesResponse(bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants);
	void PopulatePlants(const TArray<FBackendPlantDto>& Plants);
};