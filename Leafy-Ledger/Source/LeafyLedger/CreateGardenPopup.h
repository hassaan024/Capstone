// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "CreateGardenPopup.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API UCreateGardenPopup : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	UFUNCTION()
	void OnPressCreate();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_Create;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ET_GardenName;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ET_GardenDesc;
};
