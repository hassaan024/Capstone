// Fill out your copyright notice in the Description page of Project Settings.

#include "PlantWrapper.h"
#include "Components/TextBlock.h"
#include "PlantObject.h"
#include "PlantSelect.h"
#include "UserDrone.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"

void UPlantWrapper::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		UserDrone = Cast<AUserDrone>(PC->GetPawn());
	}
}

void UPlantWrapper::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	PlantData = Cast<UPlantObject>(ListItemObject);

	if (!PlantData)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlantWrapper: List item is not a UPlantObject"));
		return;
	}

	if (TXT_PlantWrapper)
	{
		TXT_PlantWrapper->SetText(FText::FromString(PlantData->CommonName));
	}

	bHasSelectionState = false;
	RefreshSelectionState(true);
}

void UPlantWrapper::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshSelectionState();
}

void UPlantWrapper::RefreshSelectionState(bool bForce)
{
	if (!TXT_PlantWrapper || !PlantData)
	{
		return;
	}

	if (!UserDrone)
	{
		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		if (PC)
		{
			UserDrone = Cast<AUserDrone>(PC->GetPawn());
		}
	}

	const bool bIsSelected = UserDrone && UserDrone->IsPlantDataSelected(PlantData);
	if (!bForce && bHasSelectionState && bIsSelected == bWasSelected)
	{
		return;
	}

	bHasSelectionState = true;
	bWasSelected = bIsSelected;
	TXT_PlantWrapper->SetColorAndOpacity(
		bIsSelected
			? FSlateColor(FLinearColor(0.25f, 0.9f, 0.35f, 1.f))
			: FSlateColor(FLinearColor::White)
	);
}

FReply UPlantWrapper::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent
)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (PlantData && PlantData->bIsDropdownToggle)
		{
			if (UPlantSelect* PlantSelect = Cast<UPlantSelect>(PlantData->GetOuter()))
			{
				PlantSelect->HandlePlantItemClicked(PlantData);
				return FReply::Handled();
			}
		}

		if (!UserDrone)
		{
			APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
			if (PC)
			{
				UserDrone = Cast<AUserDrone>(PC->GetPawn());
			}
		}

		if (UserDrone && PlantData)
		{
			UserDrone->SetSelectedPlantData(PlantData);
			RefreshSelectionState();
			//UE_LOG(LogTemp, Warning, TEXT("SELECTED: %s"), *PlantData->CommonName);
			return FReply::Handled();
		}

		UE_LOG(LogTemp, Warning, TEXT("PlantWrapper: UserDrone or PlantData is null"));
		return FReply::Unhandled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
