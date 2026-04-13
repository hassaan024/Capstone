// Fill out your copyright notice in the Description page of Project Settings.

#include "LoadGardenPopup.h"

bool ULoadGardenPopup::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (BTN_Load)
	{
		BTN_Load->OnClicked.AddDynamic(this, &ULoadGardenPopup::OnPressLoad);
	}

	if (GardenCombo)
	{
		GardenCombo->OnSelectionChanged.AddDynamic(this, &ULoadGardenPopup::OnGardenSelectionChanged);
	}

	return true;
}

void ULoadGardenPopup::SetGardens(const TArray<FBackendGardenSummaryDto>& InGardens)
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
			TEXT("%s [#%d] (%d plants)"),
			*Garden.Name,
			Garden.Id,
			Garden.PlantCount
		);
		GardenCombo->AddOption(Label);
	}

	if (Gardens.Num() > 0)
	{
		SelectedGardenId = Gardens[0].Id;
		const FString InitialLabel = FString::Printf(
			TEXT("%s [#%d] (%d plants)"),
			*Gardens[0].Name,
			Gardens[0].Id,
			Gardens[0].PlantCount
		);
		GardenCombo->SetSelectedOption(InitialLabel);
	}
}

void ULoadGardenPopup::OnGardenSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!GardenCombo) return;

	const int32 SelectedIndex = GardenCombo->FindOptionIndex(SelectedItem);
	if (Gardens.IsValidIndex(SelectedIndex))
	{
		SelectedGardenId = Gardens[SelectedIndex].Id;
	}
}

void ULoadGardenPopup::OnPressLoad()
{
	if (SelectedGardenId <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPressLoad: no garden selected"));
		return;
	}

	OnGardenChosen.ExecuteIfBound(SelectedGardenId);
	RemoveFromParent();
}
