// Fill out your copyright notice in the Description page of Project Settings.

#include "DeleteGardenPopup.h"
#include "BackendApiSubsystem.h"

bool UDeleteGardenPopup::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (BTN_Delete)
	{
		BTN_Delete->OnClicked.AddDynamic(this, &UDeleteGardenPopup::OnPressDelete);
	}

	if (GardenCombo)
	{
		GardenCombo->OnSelectionChanged.AddDynamic(this, &UDeleteGardenPopup::OnGardenSelectionChanged);
	}

	return true;
}

void UDeleteGardenPopup::SetGardens(const TArray<FBackendGardenSummaryDto>& InGardens)
{
	Gardens = InGardens;
	SelectedGardenId = 0;

	if (!GardenCombo)
	{
		UE_LOG(LogTemp, Error, TEXT("SetGardens: GardenCombo is not bound"));
		return;
	}

	GardenCombo->ClearOptions();

	for (const FBackendGardenSummaryDto& Garden : Gardens)
	{
		const FString Label = FString::Printf(
			TEXT("%s (%d plants)"),
			*Garden.Name,
			Garden.PlantCount
		);
		GardenCombo->AddOption(Label);
	}

	if (Gardens.Num() > 0)
	{
		SelectedGardenId = Gardens[0].Id;
		const FString InitialLabel = FString::Printf(
			TEXT("%s (%d plants)"),
			*Gardens[0].Name,
			Gardens[0].PlantCount
		);
		GardenCombo->SetSelectedOption(InitialLabel);
	}
}

void UDeleteGardenPopup::OnGardenSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!GardenCombo)
	{
		return;
	}

	const int32 SelectedIndex = GardenCombo->FindOptionIndex(SelectedItem);
	if (Gardens.IsValidIndex(SelectedIndex))
	{
		SelectedGardenId = Gardens[SelectedIndex].Id;
	}
}

void UDeleteGardenPopup::OnPressDelete()
{
	if (SelectedGardenId <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPressDelete: no garden selected"));
		return;
	}

	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressDelete: GameInstance is null"));
		return;
	}

	UBackendApiSubsystem* Backend = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Backend)
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressDelete: BackendApiSubsystem missing"));
		return;
	}

	Backend->DeleteGarden(
		SelectedGardenId,
		FBackendOperationResponse::CreateWeakLambda(
			this,
			[this](bool bSuccess, const FString& Message)
			{
				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("DeleteGarden failed: %s"), *Message);
					return;
				}

				RemoveFromParent();
			}
		)
	);
}
