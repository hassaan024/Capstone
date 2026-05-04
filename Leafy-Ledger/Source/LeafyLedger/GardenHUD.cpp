// Fill out your copyright notice in the Description page of Project Settings.

#include "GardenHUD.h"
#include "BackendApiSubsystem.h"
#include "BloomDateUtils.h"
#include "GardenTimeManager.h"
#include "Plant.h"
#include "Components/EditableTextBox.h"
#include "Components/Widget.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	bool TryParseNormalizedBackendDate(const FString& BackendDateText, FDateTime& OutDate)
	{
		if (BackendDateText.Len() < 10)
		{
			return false;
		}

		const FString DatePart = BackendDateText.Left(10);
		if (DatePart[4] != TEXT('-') || DatePart[7] != TEXT('-'))
		{
			return false;
		}

		const int32 Year = FCString::Atoi(*DatePart.Mid(0, 4));
		const int32 Month = FCString::Atoi(*DatePart.Mid(5, 2));
		const int32 Day = FCString::Atoi(*DatePart.Mid(8, 2));

		if (Year < 1000 || Year > 9999 || Month < 1 || Month > 12)
		{
			return false;
		}

		const int32 MaxDay = FDateTime::DaysInMonth(Year, Month);
		if (Day < 1 || Day > MaxDay)
		{
			return false;
		}

		OutDate = FDateTime(Year, Month, Day);
		return true;
	}

	int32 GetDaysToBloomForCategory(const FString& Category)
	{
		if (Category.Equals(TEXT("flower"), ESearchCase::IgnoreCase))
		{
			return 20;
		}

		if (Category.Equals(TEXT("tree"), ESearchCase::IgnoreCase))
		{
			return 50;
		}

		return 30;
	}

	FString CalculatePlantedDate(const FString& BloomDate, const FString& Category)
	{
		FDateTime ParsedBloomDate;
		if (!TryParseNormalizedBackendDate(FBloomDateUtils::NormalizeBackendDateString(BloomDate), ParsedBloomDate))
		{
			return TEXT("");
		}

		return FBloomDateUtils::FormatForBackend(ParsedBloomDate - FTimespan::FromDays(GetDaysToBloomForCategory(Category)));
	}

	AGardenTimeManager* FindGardenTimeManager(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(World, AGardenTimeManager::StaticClass(), FoundActors);
		return FoundActors.Num() > 0 ? Cast<AGardenTimeManager>(FoundActors[0]) : nullptr;
	}
}

bool UGardenHUD::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_Save)
    {
        BTN_Save->OnClicked.AddDynamic(this, &UGardenHUD::OnPressSave);
    }

    if (BTN_Exit)
    {
        BTN_Exit->OnClicked.AddDynamic(this, &UGardenHUD::OnPressExit);
    }

	if (BTN_Predict)
	{
		BTN_Predict->OnClicked.AddDynamic(this, &UGardenHUD::OnPressPredict);
	}

	if (BTN_PlantMode)
	{
		BTN_PlantMode->OnClicked.AddDynamic(this, &UGardenHUD::OnPressPlantMode);
	}

	if (BTN_PaintMode)
	{
		BTN_PaintMode->OnClicked.AddDynamic(this, &UGardenHUD::OnPressPaintMode);
	}

	if (BTN_DeleteMode)
	{
		BTN_DeleteMode->OnClicked.AddDynamic(this, &UGardenHUD::OnPressDeleteMode);
	}

	if (SLDR_Date)
	{
		SLDR_Date->OnValueChanged.AddDynamic(this, &UGardenHUD::OnValueChanged);
	}

    return true;
}

void UGardenHUD::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureBloomDateInput();
	HideDateSlider();
	RestoreDateSliderFromDraft();
}

void UGardenHUD::OnPressSave()
{
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressSave: GameInstance is null"));
		return;
	}

	UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();

	if (!GardenSession)
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressSave: GardenSessionSubsystem missing"));
		return;
	}

	if (!GardenSession->HasActiveDraft())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPressSave: no active garden draft"));
		return;
	}

	if (!GardenSession->IsDirty())
	{
		UE_LOG(LogTemp, Log, TEXT("OnPressSave: nothing to save"));
		return;
	}

	UBackendApiSubsystem* BackendApi = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();

	if (!BackendApi)
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressSave: BackendApiSubsystem missing"));
		return;
	}

	if (ET_BloomDate)
	{
		if (!BloomDateValidationError.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("OnPressSave: bloom date input is invalid"));
			return;
		}

		const FString RawBloomDate = GetRawBloomDateText();
		if (RawBloomDate.IsEmpty())
		{
			const FEditableGardenState DraftSnapshot = GardenSession->GetDraftCopy();
			if (bBloomDateWasEdited && !DraftSnapshot.BloomDate.IsEmpty())
			{
				BloomDateValidationError = TEXT("Bloom date is required.");
				if (ET_BloomDate)
				{
					ET_BloomDate->SetError(FText::FromString(BloomDateValidationError));
				}
				return;
			}
		}
		else
		{
			ValidateBloomDateText(false);
			if (!LastValidBloomDateBackend.IsEmpty())
			{
				GardenSession->SetBloomDate(LastValidBloomDateBackend);
			}
		}
	}

	const FEditableGardenState Draft = GardenSession->GetDraftCopy();
	TWeakObjectPtr<UGardenHUD> WeakThis(this);

	if (Draft.BackendGardenId > 0)
	{
		if (Draft.bPendingGardenUpdate)
		{
			BackendApi->UpdateGarden(
				Draft.BackendGardenId,
				Draft.Name,
				Draft.Description,
				Draft.BloomDate,
				Draft.Latitude,
				Draft.Longitude,
				Draft.Timezone,
				FBackendGardenResponse::CreateLambda(
					[WeakThis, GardenSession, Draft](bool bSuccess, const FString& Message, const FBackendGardenDto& Garden)
					{
						if (!bSuccess)
						{
							UE_LOG(LogTemp, Error, TEXT("UpdateGarden failed: %s"), *Message);
							return;
						}

						GardenSession->MarkGardenSaved(Garden.Id);

						if (WeakThis.IsValid())
						{
							WeakThis->SavePendingPlants(Garden.Id, Garden.BloomDate, Draft.bPendingGardenUpdate, Draft.Plants, 0);
						}
					}
				)
			);
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("OnPressSave: reusing existing backend garden id %d"), Draft.BackendGardenId);
		SavePendingPlants(Draft.BackendGardenId, Draft.BloomDate, false, Draft.Plants, 0);
		return;
	}

	BackendApi->CreateGarden(
		Draft.Name,
		Draft.Description,
		Draft.BloomDate,
		Draft.Latitude,
		Draft.Longitude,
		Draft.Timezone,
		FBackendGardenResponse::CreateLambda(
			[WeakThis, GardenSession, Draft](bool bSuccess, const FString& Message, const FBackendGardenDto& Garden)
			{
				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("CreateGarden failed: %s"), *Message);
					return;
				}

				UE_LOG(LogTemp, Log, TEXT("Garden saved successfully. BackendGardenId=%d"), Garden.Id);

				GardenSession->MarkGardenSaved(Garden.Id);

				if (WeakThis.IsValid())
				{
					WeakThis->SavePendingPlants(Garden.Id, Garden.BloomDate, true, Draft.Plants, 0);
				}
			}
		)
	);
}

void UGardenHUD::OnPressPredict()
{
	if (bPredictionInFlight)
	{
		return;
	}

	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressPredict: GameInstance is null"));
		return;
	}

	UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();
	UBackendApiSubsystem* BackendApi = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();

	if (!GardenSession || !BackendApi)
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressPredict: missing subsystem(s)"));
		return;
	}

	if (!GardenSession->HasActiveDraft())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPressPredict: no active garden draft"));
		return;
	}

	if (ET_BloomDate)
	{
		ValidateBloomDateText(false);
		if (!BloomDateValidationError.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("OnPressPredict: bloom date input is invalid"));
			return;
		}

		if (!LastValidBloomDateBackend.IsEmpty())
		{
			GardenSession->SetBloomDate(LastValidBloomDateBackend);
		}
	}

	const FEditableGardenState Draft = GardenSession->GetDraftCopy();
	if (Draft.BackendGardenId <= 0 || GardenSession->IsDirty())
	{
		UE_LOG(LogTemp, Log, TEXT("OnPressPredict: saving pending garden changes before prediction"));
		bRunPredictionAfterSave = true;
		OnPressSave();
		return;
	}

	if (Draft.BloomDate.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPressPredict: bloom date is required"));
		return;
	}

	bool bHasSavedPlant = false;
	for (const FEditablePlantPlacement& Plant : Draft.Plants)
	{
		if (!Plant.bPendingDelete && Plant.BackendPlantInstanceId > 0)
		{
			bHasSavedPlant = true;
			break;
		}
	}

	if (!bHasSavedPlant)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPressPredict: no saved plant instances found for this garden"));
		return;
	}

	bPredictionInFlight = true;
	if (BTN_Predict)
	{
		BTN_Predict->SetIsEnabled(false);
	}

	TWeakObjectPtr<UGardenHUD> WeakThis(this);
	BackendApi->GenerateGardenTimeline(
		Draft.BackendGardenId,
		Draft.BloomDate,
		FBackendGardenTimelineResponse::CreateLambda(
			[WeakThis](bool bSuccess, const FString& Message, const FBackendGardenTimelineDto& Timeline)
			{
				if (!WeakThis.IsValid())
				{
					return;
				}

				UGardenHUD* HUD = WeakThis.Get();
				HUD->bPredictionInFlight = false;
				if (HUD->BTN_Predict)
				{
					HUD->BTN_Predict->SetIsEnabled(true);
				}

				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("GenerateGardenTimeline failed: %s"), *Message);
					HUD->HideDateSlider();
					return;
				}

				FDateTime EarliestPlantDate;
				bool bHasEarliestPlantDate = false;
				int32 LongestTimeToBloom = 0;

				for (const FBackendGardenTimelinePlantDto& Plant : Timeline.Plants)
				{
					FDateTime ParsedPlantDate;
					if (Plant.DaysToMature > 0
						&& TryParseNormalizedBackendDate(FBloomDateUtils::NormalizeBackendDateString(Plant.PlantedDate), ParsedPlantDate))
					{
						if (!bHasEarliestPlantDate || ParsedPlantDate < EarliestPlantDate)
						{
							EarliestPlantDate = ParsedPlantDate;
							bHasEarliestPlantDate = true;
						}

						LongestTimeToBloom = FMath::Max(LongestTimeToBloom, Plant.DaysToMature);
					}
				}

				if (!bHasEarliestPlantDate || LongestTimeToBloom <= 0)
				{
					UE_LOG(LogTemp, Error, TEXT("GenerateGardenTimeline returned no usable planted dates"));
					HUD->HideDateSlider();
					return;
				}

				HUD->ApplyTimelineToPlantActors(Timeline);
				HUD->ConfigureDateSlider(
					FBloomDateUtils::FormatForBackend(EarliestPlantDate),
					Timeline.BloomDate,
					LongestTimeToBloom
				);
			}
		)
	);
}

void UGardenHUD::SavePendingPlants(int32 GardenId, const FString& BloomDate, bool bRefreshPlantedDates, const TArray<FEditablePlantPlacement>& Plants, int32 StartIndex)
{
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("SavePendingPlants: GameInstance is null"));
		return;
	}

	UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();
	UBackendApiSubsystem* BackendApi = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();

	if (!GardenSession || !BackendApi)
	{
		UE_LOG(LogTemp, Error, TEXT("SavePendingPlants: missing subsystem(s)"));
		return;
	}

	for (int32 i = StartIndex; i < Plants.Num(); ++i)
	{
		const FEditablePlantPlacement& Plant = Plants[i];

		if (Plant.bPendingDelete)
		{
			if (Plant.BackendPlantInstanceId <= 0)
			{
				continue;
			}

			TWeakObjectPtr<UGardenHUD> WeakThis(this);
			BackendApi->DeletePlantInstance(
				Plant.BackendPlantInstanceId,
				FBackendOperationResponse::CreateLambda(
					[WeakThis, GardenSession, Plants, GardenId, BloomDate, bRefreshPlantedDates, i, Plant](bool bSuccess, const FString& Message)
					{
						if (!bSuccess)
						{
							UE_LOG(LogTemp, Error, TEXT("DeletePlantInstance failed: %s"), *Message);
							return;
						}

						UE_LOG(
							LogTemp,
							Log,
							TEXT("Plant instance deleted. LocalId=%s BackendPlantInstanceId=%d"),
							*Plant.LocalId.ToString(),
							Plant.BackendPlantInstanceId
						);

						GardenSession->MarkPlantDeleted(Plant.LocalId);

						if (WeakThis.IsValid())
						{
							WeakThis->SavePendingPlants(GardenId, BloomDate, bRefreshPlantedDates, Plants, i + 1);
						}
					}
				)
			);

			return;
		}

		if (Plant.bPendingCreate && Plant.SpeciesId <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping plant save with invalid species id. LocalId=%s SpeciesId=%d SoilId=%d"),
				*Plant.LocalId.ToString(),
				Plant.SpeciesId,
				Plant.SoilId);
			continue;
		}

		TWeakObjectPtr<UGardenHUD> WeakThis(this);
		const FString PlantedDate = CalculatePlantedDate(BloomDate, Plant.SpeciesModelCategory);
		const bool bShouldUpdateExistingPlant = Plant.bPendingUpdate || (bRefreshPlantedDates && Plant.BackendPlantInstanceId > 0);

		if (bShouldUpdateExistingPlant && Plant.BackendPlantInstanceId > 0)
		{
			UE_LOG(
				LogTemp,
				Log,
				TEXT("Updating plant instance: LocalId=%s BackendPlantInstanceId=%d PlantedDate=%s"),
				*Plant.LocalId.ToString(),
				Plant.BackendPlantInstanceId,
				*PlantedDate
			);

			BackendApi->UpdatePlantInstance(
				Plant.BackendPlantInstanceId,
				Plant.Location,
				Plant.Rotation,
				Plant.Scale,
				Plant.HeightCm,
				Plant.AgeDays,
				Plant.HealthStatus,
				Plant.LastWateredIso8601,
				PlantedDate,
				Plant.Notes,
				FBackendPlantInstanceResponse::CreateLambda(
					[WeakThis, GardenSession, Plants, GardenId, BloomDate, bRefreshPlantedDates, i, Plant](bool bSuccess, const FString& Message, const FBackendPlantInstanceDto& PlantInstance)
					{
						if (!bSuccess)
						{
							UE_LOG(LogTemp, Error, TEXT("UpdatePlantInstance failed: %s"), *Message);
							return;
						}

						UE_LOG(
							LogTemp,
							Log,
							TEXT("Plant instance updated. LocalId=%s BackendPlantInstanceId=%d"),
							*Plant.LocalId.ToString(),
							PlantInstance.Id
						);

						GardenSession->MarkPlantSaved(Plant.LocalId, PlantInstance.Id, PlantInstance.PlantedDate);

						if (WeakThis.IsValid())
						{
							WeakThis->SavePendingPlants(GardenId, BloomDate, bRefreshPlantedDates, Plants, i + 1);
						}
					}
				)
			);

			return;
		}

		if (!Plant.bPendingCreate)
		{
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("Saving plant instance: SpeciesId=%d SoilType=LOAM"), Plant.SpeciesId);

		const float HeightValue = Plant.HeightCm;
		const int32 AgeValue = Plant.AgeDays;
		const FString HealthValue = Plant.HealthStatus;
		const FString LastWateredValue = Plant.LastWateredIso8601;
		const FString PlantedDateValue = !Plant.PlantedDate.IsEmpty()
			? Plant.PlantedDate
			: CalculatePlantedDate(BloomDate, Plant.SpeciesModelCategory);
		const float* HeightPtr = &HeightValue;
		const int32* AgePtr = &AgeValue;
		const FString* HealthPtr = &HealthValue;
		const FString* LastWateredPtr = &LastWateredValue;
		const FString* PlantedDatePtr = &PlantedDateValue;

		BackendApi->CreatePlantInstance(
			GardenId,
			Plant.SpeciesId,
			TEXT("LOAM"),
			Plant.Location,
			Plant.Rotation,
			Plant.Scale,
			HeightPtr,
			AgePtr,
			HealthPtr,
			LastWateredPtr,
			PlantedDatePtr,
			Plant.Notes,
			FBackendPlantInstanceResponse::CreateLambda(
				[WeakThis, GardenSession, Plants, GardenId, BloomDate, bRefreshPlantedDates, i, Plant](bool bSuccess, const FString& Message, const FBackendPlantInstanceDto& PlantInstance)
				{
					if (!bSuccess)
					{
						UE_LOG(LogTemp, Error, TEXT("CreatePlantInstance failed: %s"), *Message);
						return;
					}

					UE_LOG(LogTemp, Log, TEXT("Plant instance saved. LocalId=%s BackendPlantInstanceId=%d"),
						*Plant.LocalId.ToString(),
						PlantInstance.Id);

					GardenSession->MarkPlantSaved(Plant.LocalId, PlantInstance.Id, PlantInstance.PlantedDate);

					if (WeakThis.IsValid())
					{
						WeakThis->SavePendingPlants(GardenId, BloomDate, bRefreshPlantedDates, Plants, i + 1);
					}
				}
			)
		);

		return;
	}

	GardenSession->MarkGardenSaved(GardenId);
	UE_LOG(LogTemp, Log, TEXT("All pending plants saved"));

	if (bRunPredictionAfterSave)
	{
		bRunPredictionAfterSave = false;
		OnPressPredict();
	}
}

void UGardenHUD::OnPressExit()
{
	if (!GetGameInstance()) return;

	if (UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>())
	{
		if (GardenSession->IsDirty())
		{
			UE_LOG(LogTemp, Warning, TEXT("OnPressExit: unsaved changes exist"));
			return;
		}
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName("Login"));
}

void UGardenHUD::EnsureBloomDateInput()
{
	if (!ET_BloomDate)
	{
		UE_LOG(LogTemp, Warning, TEXT("GardenHUD: ET_BloomDate is not bound on the widget"));
		return;
	}

	if (!bBloomDateTextEventsBound)
	{
		ET_BloomDate->OnTextChanged.AddDynamic(this, &UGardenHUD::HandleBloomDateTextChanged);
		ET_BloomDate->OnTextCommitted.AddDynamic(this, &UGardenHUD::HandleBloomDateTextCommitted);
		bBloomDateTextEventsBound = true;
	}

	ET_BloomDate->SetHintText(FText::FromString(TEXT("MM/DD/YYYY")));
	ApplyDraftBloomDate();
}

void UGardenHUD::ApplyDraftBloomDate()
{
	if (!ET_BloomDate || !GetGameInstance())
	{
		return;
	}

	if (UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>())
	{
		const FString DraftBloomDate = GardenSession->HasActiveDraft() ? GardenSession->GetDraft().BloomDate : TEXT("");
		const FString NormalizedBloomDate = FBloomDateUtils::NormalizeBackendDateString(DraftBloomDate);
		FString DisplayText = DraftBloomDate.TrimStartAndEnd();

		FDateTime ParsedDate;
		if (TryParseNormalizedBackendDate(NormalizedBloomDate, ParsedDate))
		{
			DisplayText = FBloomDateUtils::FormatForDisplay(ParsedDate);
			LastValidBloomDateDisplay = DisplayText;
			LastValidBloomDateBackend = NormalizedBloomDate;
			BloomDateValidationError.Empty();
		}
		else
		{
			LastValidBloomDateDisplay.Empty();
			LastValidBloomDateBackend.Empty();
			BloomDateValidationError.Empty();
		}

		bSuppressBloomDateCallbacks = true;
		ET_BloomDate->SetText(FText::FromString(DisplayText));
		ET_BloomDate->SetError(FText::GetEmpty());
		bSuppressBloomDateCallbacks = false;
		bBloomDateWasEdited = false;
	}
}

void UGardenHUD::ValidateBloomDateText(bool bBroadcastOnSuccess)
{
	if (!ET_BloomDate) return;

	const FBloomDateValidationResult ValidationResult = FBloomDateUtils::ValidateUserInput(GetRawBloomDateText(), true);
	if (!ValidationResult.bIsValid)
	{
		BloomDateValidationError = ValidationResult.ErrorMessage;
		ET_BloomDate->SetError(FText::FromString(BloomDateValidationError));
		return;
	}

	BloomDateValidationError.Empty();
	ET_BloomDate->SetError(FText::GetEmpty());

	if (ValidationResult.DisplayText.IsEmpty())
	{
		LastValidBloomDateDisplay.Empty();
		LastValidBloomDateBackend.Empty();
		return;
	}

	LastValidBloomDateDisplay = ValidationResult.DisplayText;
	LastValidBloomDateBackend = ValidationResult.BackendText;

	if (bBroadcastOnSuccess && GetGameInstance())
	{
		if (UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>())
		{
			GardenSession->SetBloomDate(LastValidBloomDateBackend);
		}
	}
}

FString UGardenHUD::GetRawBloomDateText() const
{
	return ET_BloomDate ? ET_BloomDate->GetText().ToString().TrimStartAndEnd() : TEXT("");
}

void UGardenHUD::HandleBloomDateTextChanged(const FText& NewText)
{
	if (bSuppressBloomDateCallbacks)
	{
		return;
	}

	bBloomDateWasEdited = true;
	ValidateBloomDateText(true);
}

void UGardenHUD::HandleBloomDateTextCommitted(const FText& NewText, ETextCommit::Type CommitMethod)
{
	if (bSuppressBloomDateCallbacks)
	{
		return;
	}

	bBloomDateWasEdited = true;
	ValidateBloomDateText(true);

	if (CommitMethod == ETextCommit::OnCleared || !BloomDateValidationError.IsEmpty() || GetRawBloomDateText().IsEmpty())
	{
		return;
	}

	bSuppressBloomDateCallbacks = true;
	ET_BloomDate->SetText(FText::FromString(LastValidBloomDateDisplay));
	bSuppressBloomDateCallbacks = false;
}

void UGardenHUD::OnValueChanged(float Value)
{
	if (!bDateSliderReady || bSuppressSliderCallbacks || !SLDR_Date || !TXT_CurrentDate)
	{
		return;
	}

	ApplySliderDay(Value);
	UpdateSliderDateText(Value);

	//if (!bSliderWidthSet)
	//{
	//	SliderWidth = SLDR_Date->GetCachedGeometry().GetLocalSize().X;

	//	if (SliderWidth > 0.0f)
	//	{
	//		bSliderWidthSet = true;
	//	}
	//}

	//float TextWidth = TXT_CurrentDate->GetCachedGeometry().GetLocalSize().X;
	//TXT_CurrentDate->SetRenderTranslation(FVector2D((Value * SliderWidth) - (TextWidth * 0.5f), 0.0f));

	//TXT_CurrentDate->SetRenderTranslation(FVector2D(Value * SliderWidth, 0.0f));
}

void UGardenHUD::ConfigureDateSlider(const FString& FirstPlantDate, const FString& BloomDate, int32 LongestTimeToBloom)
{
	if (!SLDR_Date || !TXT_CurrentDate)
	{
		UE_LOG(LogTemp, Warning, TEXT("ConfigureDateSlider: slider or current-date text is not bound"));
		return;
	}

	FDateTime ParsedFirstPlantDate;
	FDateTime ParsedBloomDate;
	if (LongestTimeToBloom <= 0
		|| !TryParseNormalizedBackendDate(FBloomDateUtils::NormalizeBackendDateString(FirstPlantDate), ParsedFirstPlantDate)
		|| !TryParseNormalizedBackendDate(FBloomDateUtils::NormalizeBackendDateString(BloomDate), ParsedBloomDate)
		|| ParsedFirstPlantDate > ParsedBloomDate)
	{
		HideDateSlider();
		return;
	}

	SliderStartDate = ParsedFirstPlantDate;
	SliderBloomDate = ParsedBloomDate;
	SliderLongestTimeToBloom = LongestTimeToBloom;

	const FDateTime EpochDate(2000, 1, 1);
	SliderStartDayIndex = static_cast<int32>((SliderStartDate - EpochDate).GetDays());
	SliderBloomDayIndex = static_cast<int32>((SliderBloomDate - EpochDate).GetDays());

	if (SliderStartDayIndex < 0 || SliderBloomDayIndex < SliderStartDayIndex)
	{
		HideDateSlider();
		return;
	}

	bDateSliderReady = true;
	//bSliderWidthSet = false;

	SLDR_Date->SetStepSize(1.0f / static_cast<float>(SliderLongestTimeToBloom));
	SLDR_Date->SetIsEnabled(true);
	SLDR_Date->SetVisibility(ESlateVisibility::Visible);

	if (TXT_CurrentDate)
	{
		TXT_CurrentDate->SetVisibility(ESlateVisibility::Visible);
	}

	if (TXT_StartDate)
	{
		TXT_StartDate->SetText(FText::FromString(FBloomDateUtils::FormatForDisplay(SliderStartDate)));
		TXT_StartDate->SetVisibility(ESlateVisibility::Visible);
	}

	if (TXT_EndDate)
	{
		TXT_EndDate->SetText(FText::FromString(FBloomDateUtils::FormatForDisplay(SliderBloomDate)));
		TXT_EndDate->SetVisibility(ESlateVisibility::Visible);
	}

	if (AGardenTimeManager* TimeManager = FindGardenTimeManager(GetWorld()))
	{
		TimeManager->GlobalBloomDate = SliderBloomDayIndex;
		TimeManager->SetCurrentDayIndex(SliderStartDayIndex);
		TimeManager->RefreshCurrentDay();
	}

	bSuppressSliderCallbacks = true;
	SLDR_Date->SetValue(0.0f);
	bSuppressSliderCallbacks = false;

	UpdateSliderDateText(0.0f);
	SetPlantingDateTextVisibilityForAllPlants(true);
}

void UGardenHUD::HideDateSlider()
{
	bDateSliderReady = false;
	//bSliderWidthSet = false;
	SliderLongestTimeToBloom = 0;

	if (SLDR_Date)
	{
		SLDR_Date->SetIsEnabled(false);
		SLDR_Date->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (TXT_CurrentDate)
	{
		TXT_CurrentDate->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (TXT_StartDate)
	{
		TXT_StartDate->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (TXT_EndDate)
	{
		TXT_EndDate->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetPlantingDateTextVisibilityForAllPlants(false);
}

bool UGardenHUD::IsDateSliderVisible() const
{
	return bDateSliderReady && SLDR_Date && SLDR_Date->GetVisibility() == ESlateVisibility::Visible;
}

void UGardenHUD::RestoreDateSliderFromDraft()
{
	if (!GetGameInstance())
	{
		return;
	}

	UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();
	if (!GardenSession || !GardenSession->HasActiveDraft())
	{
		return;
	}

	const FEditableGardenState& Draft = GardenSession->GetDraft();

	FDateTime ParsedBloomDate;
	if (!TryParseNormalizedBackendDate(FBloomDateUtils::NormalizeBackendDateString(Draft.BloomDate), ParsedBloomDate))
	{
		return;
	}

	FDateTime EarliestPlantDate;
	bool bHasEarliestPlantDate = false;
	int32 LongestTimeToBloom = 0;

	for (const FEditablePlantPlacement& Plant : Draft.Plants)
	{
		if (Plant.bPendingDelete)
		{
			continue;
		}

		if (Plant.PlantedDate.IsEmpty())
		{
			return;
		}

		FDateTime ParsedPlantDate;
		if (!TryParseNormalizedBackendDate(FBloomDateUtils::NormalizeBackendDateString(Plant.PlantedDate), ParsedPlantDate)
			|| ParsedPlantDate > ParsedBloomDate)
		{
			continue;
		}

		if (!bHasEarliestPlantDate || ParsedPlantDate < EarliestPlantDate)
		{
			EarliestPlantDate = ParsedPlantDate;
			bHasEarliestPlantDate = true;
		}

		LongestTimeToBloom = FMath::Max(LongestTimeToBloom, static_cast<int32>((ParsedBloomDate - ParsedPlantDate).GetDays()));
	}

	if (!bHasEarliestPlantDate || LongestTimeToBloom <= 0)
	{
		return;
	}

	ConfigureDateSlider(
		FBloomDateUtils::FormatForBackend(EarliestPlantDate),
		FBloomDateUtils::FormatForBackend(ParsedBloomDate),
		LongestTimeToBloom
	);
}

void UGardenHUD::ApplyTimelineToPlantActors(const FBackendGardenTimelineDto& Timeline)
{
	if (!GetGameInstance() || !GetWorld())
	{
		return;
	}

	FDateTime ParsedBloomDate;
	if (!TryParseNormalizedBackendDate(FBloomDateUtils::NormalizeBackendDateString(Timeline.BloomDate), ParsedBloomDate))
	{
		return;
	}

	const FDateTime EpochDate(2000, 1, 1);
	const int32 BloomDayIndex = static_cast<int32>((ParsedBloomDate - EpochDate).GetDays());
	if (BloomDayIndex < 0)
	{
		return;
	}

	UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();
	if (!GardenSession || !GardenSession->HasActiveDraft())
	{
		return;
	}

	TMap<FGuid, FBackendGardenTimelinePlantDto> TimelineByLocalId;
	for (const FBackendGardenTimelinePlantDto& TimelinePlant : Timeline.Plants)
	{
		if (TimelinePlant.PlantInstanceId <= 0 || TimelinePlant.PlantedDate.IsEmpty())
		{
			continue;
		}

		GardenSession->SetPlantPlantedDateByBackendId(TimelinePlant.PlantInstanceId, TimelinePlant.PlantedDate);
	}

	const FEditableGardenState Draft = GardenSession->GetDraftCopy();
	for (const FBackendGardenTimelinePlantDto& TimelinePlant : Timeline.Plants)
	{
		for (const FEditablePlantPlacement& Placement : Draft.Plants)
		{
			if (Placement.BackendPlantInstanceId == TimelinePlant.PlantInstanceId)
			{
				TimelineByLocalId.Add(Placement.LocalId, TimelinePlant);
				break;
			}
		}
	}

	TArray<AActor*> FoundPlants;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlant::StaticClass(), FoundPlants);
	for (AActor* Actor : FoundPlants)
	{
		APlant* PlantActor = Cast<APlant>(Actor);
		if (!PlantActor)
		{
			continue;
		}

		const FBackendGardenTimelinePlantDto* TimelinePlant = TimelineByLocalId.Find(PlantActor->LocalPlantId);
		if (!TimelinePlant)
		{
			continue;
		}

		FDateTime ParsedPlantDate;
		if (!TryParseNormalizedBackendDate(FBloomDateUtils::NormalizeBackendDateString(TimelinePlant->PlantedDate), ParsedPlantDate))
		{
			continue;
		}

		const int32 PlantingDayIndex = static_cast<int32>((ParsedPlantDate - EpochDate).GetDays());
		if (PlantingDayIndex < 0 || PlantingDayIndex > BloomDayIndex)
		{
			continue;
		}

		PlantActor->PlantingDayIndex = PlantingDayIndex;
		PlantActor->BloomDayIndex = BloomDayIndex;
		PlantActor->DaysToBloom = FMath::Max(1, BloomDayIndex - PlantingDayIndex);
		PlantActor->WitherDayIndex = PlantActor->DaysToWither > 0
			? BloomDayIndex + PlantActor->DaysToWither
			: MAX_int32;
		PlantActor->PlantingDate = FBloomDateUtils::DayIndexToDisplayDate(PlantingDayIndex);
		PlantActor->UpdatePlantingDateText();
	}
}

void UGardenHUD::SetPlantingDateTextVisibilityForAllPlants(bool bVisible) const
{
	if (!GetWorld())
	{
		return;
	}

	TArray<AActor*> FoundPlants;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlant::StaticClass(), FoundPlants);
	for (AActor* Actor : FoundPlants)
	{
		if (APlant* PlantActor = Cast<APlant>(Actor))
		{
			PlantActor->SetPlantingDateTextVisible(bVisible);
		}
	}
}

void UGardenHUD::UpdateSliderDateText(float Value)
{
	if (!bDateSliderReady || !TXT_CurrentDate)
	{
		return;
	}

	const int32 OffsetDays = FMath::RoundToInt(FMath::Clamp(Value, 0.0f, 1.0f) * static_cast<float>(SliderLongestTimeToBloom));
	const FDateTime CurrentDate = SliderStartDate + FTimespan::FromDays(OffsetDays);
	TXT_CurrentDate->SetText(FText::FromString(FBloomDateUtils::FormatForDisplay(CurrentDate)));
}

void UGardenHUD::ApplySliderDay(float Value)
{
	if (!bDateSliderReady)
	{
		return;
	}

	const int32 OffsetDays = FMath::RoundToInt(FMath::Clamp(Value, 0.0f, 1.0f) * static_cast<float>(SliderLongestTimeToBloom));
	const int32 DayIndex = FMath::Clamp(SliderStartDayIndex + OffsetDays, SliderStartDayIndex, SliderBloomDayIndex);

	if (AGardenTimeManager* TimeManager = FindGardenTimeManager(GetWorld()))
	{
		TimeManager->SetCurrentDayIndex(DayIndex);
	}
}

void UGardenHUD::OnPressPlantMode()
{
	SetGardenMode(EGardenEditMode::Plant);
}

void UGardenHUD::OnPressPaintMode()
{
	SetGardenMode(EGardenEditMode::Paint);
}

void UGardenHUD::OnPressDeleteMode()
{
	SetGardenMode(EGardenEditMode::Delete);
}

void UGardenHUD::SetGardenMode(EGardenEditMode NewMode)
{
	if (!GetWorld())
	{
		return;
	}

	if (AUserDrone* Drone = Cast<AUserDrone>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
	{
		Drone->SetGardenEditMode(NewMode);
	}
}
