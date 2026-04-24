// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GardenSessionSubsystem.h"
#include "GardenExit.generated.h"

class UBackendApiSubsystem;
class UEditableTextBox;

UCLASS()
class LEAFYLEDGER_API UGardenExit : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual bool Initialize() override;
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnPressSave();

    UFUNCTION()
    void OnPressExit();

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_Save;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_Exit;

private:
	void EnsureBloomDateInput();
	void ApplyDraftBloomDate();
	void ValidateBloomDateText(bool bBroadcastOnSuccess);
	FString GetRawBloomDateText() const;

	UFUNCTION()
	void HandleBloomDateTextChanged(const FText& NewText);

	UFUNCTION()
	void HandleBloomDateTextCommitted(const FText& NewText, ETextCommit::Type CommitMethod);

	void SavePendingPlants(int32 GardenId, const TArray<FEditablePlantPlacement>& Plants, int32 StartIndex = 0);

	UPROPERTY(Transient, meta = (BindWidget))
	UEditableTextBox* ET_BloomDate = nullptr;

	FString LastValidBloomDateDisplay;
	FString LastValidBloomDateBackend;
	FString BloomDateValidationError;
	bool bBloomDateTextEventsBound = false;
	bool bSuppressBloomDateCallbacks = false;
	bool bBloomDateWasEdited = false;
};
