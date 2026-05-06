// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackendApiTypes.h"
#include "MenuController.h"
#include "BrowseSpecies.generated.h"

class UButton;
class UEditableTextBox;
class UTileView;
class UWidget;

UCLASS()
class LEAFYLEDGER_API UBrowseSpecies : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPressSearchSpecies();

	UFUNCTION()
	void OnPressBack();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_SearchSpecies;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_Back;

	UPROPERTY(meta = (BindWidgetOptional))
	UEditableTextBox* ET_Browse;

	UPROPERTY(meta = (BindWidgetOptional))
	UTileView* TV_PlantCards = nullptr;

	AMenuController* MenuController;

private:
	void ResolveWidgetReferences();
	void HandleSearchResponse(bool bSuccess, const FString& Message, const TArray<FBackendPlantSearchResultDto>& Plants);
};
