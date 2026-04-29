// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackendApiTypes.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "DeleteGardenPopup.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API UDeleteGardenPopup : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;

	void SetGardens(const TArray<FBackendGardenSummaryDto>& InGardens);

	UFUNCTION()
	void OnGardenSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_Delete;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxString* GardenCombo;

	UFUNCTION()
	void OnPressDelete();

	UPROPERTY()
	TArray<FBackendGardenSummaryDto> Gardens;

	int32 SelectedGardenId = 0;
};
