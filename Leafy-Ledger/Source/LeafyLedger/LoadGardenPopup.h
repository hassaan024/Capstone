// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackendApiTypes.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "LoadGardenPopup.generated.h"

class UButton;
class UComboBoxString;

DECLARE_DELEGATE_OneParam(FOnGardenChosen, int32)

UCLASS()
class LEAFYLEDGER_API ULoadGardenPopup : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;

	void SetGardens(const TArray<FBackendGardenSummaryDto>& InGardens);

	UFUNCTION()
	void OnGardenSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_Load;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxString* GardenCombo;

	UFUNCTION()
	void OnPressLoad();

	FOnGardenChosen OnGardenChosen;

	UPROPERTY()
	TArray<FBackendGardenSummaryDto> Gardens;

	int32 SelectedGardenId = 0;
};
