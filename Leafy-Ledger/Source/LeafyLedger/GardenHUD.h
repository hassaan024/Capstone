// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "GardenSessionSubsystem.h"
#include "UserDrone.h"
#include "GardenHUD.generated.h"

class UBackendApiSubsystem;
class UEditableTextBox;
struct FBackendGardenTimelineDto;

UCLASS()
class LEAFYLEDGER_API UGardenHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual bool Initialize() override;
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnPressSave();

    UFUNCTION()
    void OnPressExit();

	UFUNCTION()
	void OnPressPredict();

	UFUNCTION()
	void OnPressPlantMode();

	UFUNCTION()
	void OnPressPaintMode();

	UFUNCTION()
	void OnPressDeleteMode();

    UFUNCTION()
	void OnValueChanged(float Value);

	UFUNCTION(BlueprintCallable)
	void ConfigureDateSlider(const FString& FirstPlantDate, const FString& BloomDate, int32 LongestTimeToBloom);

	UFUNCTION(BlueprintCallable)
	void HideDateSlider();

	bool IsDateSliderVisible() const;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_Save;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_Exit;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_Predict;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_PlantMode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_PaintMode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_DeleteMode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USlider* SLDR_Date;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TXT_CurrentDate;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* TXT_StartDate;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* TXT_EndDate;

private:
	void EnsureBloomDateInput();
	void ApplyDraftBloomDate();
	void ValidateBloomDateText(bool bBroadcastOnSuccess);
	FString GetRawBloomDateText() const;

	UFUNCTION()
	void HandleBloomDateTextChanged(const FText& NewText);

	UFUNCTION()
	void HandleBloomDateTextCommitted(const FText& NewText, ETextCommit::Type CommitMethod);

	void SavePendingPlants(int32 GardenId, const FString& BloomDate, bool bRefreshPlantedDates, const TArray<FEditablePlantPlacement>& Plants, int32 StartIndex = 0);
	void RestoreDateSliderFromDraft();
	void ApplyTimelineToPlantActors(const FBackendGardenTimelineDto& Timeline);
	void UpdateSliderDateText(float Value);
	void ApplySliderDay(float Value);
	void SetPlantingDateTextVisibilityForAllPlants(bool bVisible) const;
	void SetGardenMode(EGardenEditMode NewMode);

	UPROPERTY(Transient, meta = (BindWidget))
	UEditableTextBox* ET_BloomDate = nullptr;

	FString LastValidBloomDateDisplay;
	FString LastValidBloomDateBackend;
	FString BloomDateValidationError;
	float SliderWidth;
	bool bBloomDateTextEventsBound = false;
	bool bSuppressBloomDateCallbacks = false;
	bool bBloomDateWasEdited = false;
	//bool bSliderWidthSet = false;
	bool bDateSliderReady = false;
	bool bSuppressSliderCallbacks = false;
	bool bPredictionInFlight = false;
	bool bRunPredictionAfterSave = false;
	FDateTime SliderStartDate = FDateTime::MinValue();
	FDateTime SliderBloomDate = FDateTime::MinValue();
	int32 SliderStartDayIndex = 0;
	int32 SliderBloomDayIndex = 0;
	int32 SliderLongestTimeToBloom = 0;
};
