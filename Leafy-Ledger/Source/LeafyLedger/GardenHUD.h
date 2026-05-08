// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "GardenSessionSubsystem.h"
#include "UserDrone.h"
#include "GardenHUD.generated.h"

class UBackendApiSubsystem;
class UEditableTextBox;
class UBorder;
class UButton;
class UHorizontalBox;
class UWidget;
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
	void OnPressPaintDirt();

	UFUNCTION()
	void OnPressPaintGrass();

	UFUNCTION()
	void OnBrushSizeChanged(float Value);

    UFUNCTION()
	void OnValueChanged(float Value);

	UFUNCTION(BlueprintCallable)
	void ConfigureDateSlider(const FString& FirstPlantDate, const FString& BloomDate, int32 LongestTimeToBloom);

	UFUNCTION(BlueprintCallable)
	void HideDateSlider();

	UFUNCTION(BlueprintCallable)
	bool IsPredictionRunning() const;
	bool IsDateSliderVisible() const;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UWidget* BTN_Save;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UWidget* BTN_Exit;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UWidget* BTN_Predict;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_PlantMode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_PaintMode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_DeleteMode;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_Dirt = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_Grass = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USlider* SLDR_Date;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UProgressBar* PB_DateProgress = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TXT_CurrentDate;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* TXT_StartDate;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* TXT_EndDate;

private:
	UButton* ResolveMenuButton(UWidget* MenuButtonWidget) const;
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
	void ConfigurePrePredictionDateSlider(const FString& BloomDate);
	void ApplyTimelineToPlantActors(const FBackendGardenTimelineDto& Timeline);
	void UpdateDateProgress(float Value) const;
	void UpdateSliderDateText(float Value);
	void ApplySliderDay(float Value);
	void SetPlantingDateTextVisibilityForAllPlants(bool bVisible) const;
	void SetAllPlantActorsForceBloomed(bool bForce) const;
	void SetPredictedPlacementSchedulesEnabled(bool bEnabled) const;
	bool DraftHasPredictedPlantingDates() const;
	int32 RemovePlantsBloomingAfterDate(const FString& BloomDate);
	void SetGardenMode(EGardenEditMode NewMode);
	void BuildPaintLayerControls();
	void BuildPaintBrushControls();
	void WrapModeButtonsWithBorders();
	UBorder* WrapModeButtonWithBorder(UButton* Button, const FName& BorderName);
	void SyncModeControls(EGardenEditMode ActiveMode);
	void SetModeBorderActive(UBorder* Border, bool bActive) const;
	void SetPaintLayerButtonActive(UButton* Button, bool bActive) const;
	void SetSelectedPaintLayer(FName LayerName);
	EGardenEditMode GetCurrentGardenMode() const;
	void CancelActiveGardenModification() const;

	UPROPERTY(Transient, meta = (BindWidget))
	UEditableTextBox* ET_BloomDate = nullptr;

	UPROPERTY(Transient)
	UHorizontalBox* PaintLayerControls = nullptr;

	UPROPERTY(Transient)
	UHorizontalBox* PaintBrushControls = nullptr;

	UPROPERTY(Transient)
	USlider* RuntimeSLDR_BrushSize = nullptr;

	UPROPERTY(Transient)
	UTextBlock* RuntimeTXT_BrushSize = nullptr;

	UPROPERTY(Transient)
	UButton* RuntimeBTN_Dirt = nullptr;

	UPROPERTY(Transient)
	UButton* RuntimeBTN_Grass = nullptr;

	UPROPERTY(Transient)
	UBorder* PlantModeBorder = nullptr;

	UPROPERTY(Transient)
	UBorder* PaintModeBorder = nullptr;

	UPROPERTY(Transient)
	UBorder* DeleteModeBorder = nullptr;

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
	bool bPredictionTimelineActive = false;
	FName SelectedPaintLayerName = TEXT("Dirt");
	FDateTime SliderStartDate = FDateTime::MinValue();
	FDateTime SliderBloomDate = FDateTime::MinValue();
	int32 SliderStartDayIndex = 0;
	int32 SliderBloomDayIndex = 0;
	int32 SliderLongestTimeToBloom = 0;
};
