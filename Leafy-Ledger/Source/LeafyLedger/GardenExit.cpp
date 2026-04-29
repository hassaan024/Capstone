// Fill out your copyright notice in the Description page of Project Settings.

#include "GardenExit.h"
#include "BackendApiSubsystem.h"
#include "BloomDateUtils.h"
#include "Components/EditableTextBox.h"
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
}

bool UGardenExit::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_Save)
    {
        BTN_Save->OnClicked.AddDynamic(this, &UGardenExit::OnPressSave);
    }

    if (BTN_Exit)
    {
        BTN_Exit->OnClicked.AddDynamic(this, &UGardenExit::OnPressExit);
    }

    return true;
}

void UGardenExit::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureBloomDateInput();
}

void UGardenExit::OnPressSave()
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
	TWeakObjectPtr<UGardenExit> WeakThis(this);

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

void UGardenExit::SavePendingPlants(int32 GardenId, const FString& BloomDate, bool bRefreshPlantedDates, const TArray<FEditablePlantPlacement>& Plants, int32 StartIndex)
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
			continue;
		}

		if (Plant.bPendingCreate && Plant.SpeciesId <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping plant save with invalid species id. LocalId=%s SpeciesId=%d SoilId=%d"),
				*Plant.LocalId.ToString(),
				Plant.SpeciesId,
				Plant.SoilId);
			continue;
		}

		TWeakObjectPtr<UGardenExit> WeakThis(this);
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

						GardenSession->MarkPlantSaved(Plant.LocalId, PlantInstance.Id);

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

		BackendApi->EnsureGenericSoil(
			FBackendSoilIdResponse::CreateLambda(
				[WeakThis, GardenSession, BackendApi, Plants, GardenId, BloomDate, bRefreshPlantedDates, i, Plant](bool bSoilSuccess, const FString& SoilMessage, int32 SoilId)
				{
					if (!bSoilSuccess || SoilId <= 0)
					{
						UE_LOG(LogTemp, Error, TEXT("EnsureGenericSoil failed: %s"), *SoilMessage);
						return;
					}

					UE_LOG(LogTemp, Log, TEXT("Saving plant instance: SpeciesId=%d SoilId=%d"), Plant.SpeciesId, SoilId);

					const float HeightValue = Plant.HeightCm;
					const int32 AgeValue = Plant.AgeDays;
					const FString HealthValue = Plant.HealthStatus;
					const FString LastWateredValue = Plant.LastWateredIso8601;
					const FString PlantedDateValue = CalculatePlantedDate(BloomDate, Plant.SpeciesModelCategory);
					const float* HeightPtr = &HeightValue;
					const int32* AgePtr = &AgeValue;
					const FString* HealthPtr = &HealthValue;
					const FString* LastWateredPtr = &LastWateredValue;
					const FString* PlantedDatePtr = &PlantedDateValue;

					BackendApi->CreatePlantInstance(
						GardenId,
						Plant.SpeciesId,
						SoilId,
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

								GardenSession->MarkPlantSaved(Plant.LocalId, PlantInstance.Id);

								if (WeakThis.IsValid())
								{
									WeakThis->SavePendingPlants(GardenId, BloomDate, bRefreshPlantedDates, Plants, i + 1);
								}
							}
						)
					);
				}
			)
		);

		return;
	}

	GardenSession->MarkGardenSaved(GardenId);
	UE_LOG(LogTemp, Log, TEXT("All pending plants saved"));
}

void UGardenExit::OnPressExit()
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

void UGardenExit::EnsureBloomDateInput()
{
	if (!ET_BloomDate)
	{
		UE_LOG(LogTemp, Warning, TEXT("GardenExit: ET_BloomDate is not bound on the widget"));
		return;
	}

	if (!bBloomDateTextEventsBound)
	{
		ET_BloomDate->OnTextChanged.AddDynamic(this, &UGardenExit::HandleBloomDateTextChanged);
		ET_BloomDate->OnTextCommitted.AddDynamic(this, &UGardenExit::HandleBloomDateTextCommitted);
		bBloomDateTextEventsBound = true;
	}

	ET_BloomDate->SetHintText(FText::FromString(TEXT("MM/DD/YYYY")));
	ApplyDraftBloomDate();
}

void UGardenExit::ApplyDraftBloomDate()
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

void UGardenExit::ValidateBloomDateText(bool bBroadcastOnSuccess)
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

FString UGardenExit::GetRawBloomDateText() const
{
	return ET_BloomDate ? ET_BloomDate->GetText().ToString().TrimStartAndEnd() : TEXT("");
}

void UGardenExit::HandleBloomDateTextChanged(const FText& NewText)
{
	if (bSuppressBloomDateCallbacks)
	{
		return;
	}

	bBloomDateWasEdited = true;
	ValidateBloomDateText(true);
}

void UGardenExit::HandleBloomDateTextCommitted(const FText& NewText, ETextCommit::Type CommitMethod)
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
