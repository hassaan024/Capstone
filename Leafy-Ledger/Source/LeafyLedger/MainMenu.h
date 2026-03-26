// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "MenuController.h"
#include "MainMenu.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;

protected:
	UFUNCTION()
	void OnPressUpdateDisplayName();

	UFUNCTION()
	void OnPressSavedPlants();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_UpdateDisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_SavedPlants;

	AMenuController* MenuController;
};
