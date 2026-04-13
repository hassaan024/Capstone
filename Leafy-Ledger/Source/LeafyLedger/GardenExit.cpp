// Fill out your copyright notice in the Description page of Project Settings.

#include "GardenExit.h"
#include "BackendApiSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

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

	const FEditableGardenState Draft = GardenSession->GetDraftCopy();
	TWeakObjectPtr<UGardenExit> WeakThis(this);

	if (Draft.BackendGardenId > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("OnPressSave: reusing existing backend garden id %d"), Draft.BackendGardenId);
		SavePendingPlants(Draft.BackendGardenId, Draft.Plants, 0);
		return;
	}

	BackendApi->CreateGarden(
		Draft.Name,
		Draft.Description,
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
					WeakThis->SavePendingPlants(Garden.Id, Draft.Plants, 0);
				}
			}
		)
	);
}

void UGardenExit::SavePendingPlants(int32 GardenId, const TArray<FEditablePlantPlacement>& Plants, int32 StartIndex)
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

		if (!Plant.bPendingCreate || Plant.bPendingDelete) continue;

		if (Plant.SpeciesId <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping plant save with invalid species id. LocalId=%s SpeciesId=%d SoilId=%d"),
				*Plant.LocalId.ToString(),
				Plant.SpeciesId,
				Plant.SoilId);
			continue;
		}

		TWeakObjectPtr<UGardenExit> WeakThis(this);

		BackendApi->EnsureGenericSoil(
			FBackendSoilIdResponse::CreateLambda(
				[WeakThis, GardenSession, BackendApi, Plants, GardenId, i, Plant](bool bSoilSuccess, const FString& SoilMessage, int32 SoilId)
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
					const float* HeightPtr = &HeightValue;
					const int32* AgePtr = &AgeValue;
					const FString* HealthPtr = &HealthValue;
					const FString* LastWateredPtr = &LastWateredValue;

					BackendApi->CreatePlantInstance(
						GardenId,
						Plant.SpeciesId,
						SoilId,
						HeightPtr,
						AgePtr,
						HealthPtr,
						LastWateredPtr,
						Plant.Notes,
						FBackendPlantInstanceResponse::CreateLambda(
							[WeakThis, GardenSession, Plants, GardenId, i, Plant](bool bSuccess, const FString& Message, const FBackendPlantInstanceDto& PlantInstance)
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
									WeakThis->SavePendingPlants(GardenId, Plants, i + 1);
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