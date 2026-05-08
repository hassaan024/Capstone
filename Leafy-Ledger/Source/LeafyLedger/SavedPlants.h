// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Components/TileView.h"
#include "Components/Button.h"
#include "Components/WrapBox.h"
#include "BackendApiTypes.h"
#include "MenuController.h"
#include "SavedPlants.generated.h"

class UPlantObject;
class UTextBlock;
class UUserWidget;
class UWidget;
class USavedPlants;

UCLASS()
class LEAFYLEDGER_API UGardenSelectorButtonProxy : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(USavedPlants* InOwner, int32 InGardenId);

	UFUNCTION()
	void HandleClicked();

private:
	UPROPERTY()
	USavedPlants* Owner = nullptr;

	int32 GardenId = 0;
};

UCLASS()
class LEAFYLEDGER_API USavedPlants : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Plants")
	void DeleteSavedPlant(int32 PerenualId);

	UFUNCTION(BlueprintCallable, Category = "Plants")
	void RemoveSavedPlantFromList(int32 PerenualId);

	UFUNCTION()
	void HandleRemoveClicked(int32 PerenualId);

	UFUNCTION()
	void HandleSaveStateChanged();

	void HandleEntryGenerated(UUserWidget& EntryWidget);

	UFUNCTION()
	void OnPressBack();

	void SelectGarden(int32 GardenId);

	UPROPERTY(meta = (BindWidget))
	UTileView* TV_PlantCards;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWrapBox* WB_Gardens;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidget* BTN_Back;

	AMenuController* MenuController;

protected:
	virtual void NativeConstruct() override;

private:
	UButton* ResolveMenuButton(UWidget* MenuButtonWidget) const;
	void RefreshCurrentSource();
	void FetchGlobalSavedSpecies();
	void FetchGardenSavedSpecies(int32 GardenId);
	void FetchGardens();
	void HandleFetchSavedSpeciesResponse(bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants);
	void HandleFetchGardensResponse(bool bSuccess, const FString& Message, const TArray<FBackendGardenSummaryDto>& InGardens);
	void RebuildGardenTabs();
	void CreateGardenTabButton(int32 GardenId, const FString& Label);
	void UpdateGardenTabStyles();
	FString BuildGardenOptionLabel(const FBackendGardenSummaryDto& Garden, const TSet<FString>& DuplicateNames) const;
	int32 GetSelectedGardenId() const;

	void PopulatePlants(const TArray<FBackendPlantDto>& Plants);

	UPROPERTY()
	TArray<FBackendGardenSummaryDto> Gardens;

	UPROPERTY()
	TMap<UButton*, int32> GardenButtonToId;

	UPROPERTY()
	TMap<UButton*, UTextBlock*> GardenButtonLabels;

	UPROPERTY()
	TArray<UGardenSelectorButtonProxy*> GardenButtonHandlers;

	int32 SelectedGardenId = 0;
};
