// Fill out your copyright notice in the Description page of Project Settings.

#include "PlantWrapper.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "PlantObject.h"
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

	if (PWSlider)
	{
		PWSlider->SetValue(PlantData->SliderValue);
	}
}

FReply UPlantWrapper::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent
)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
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
			UE_LOG(LogTemp, Warning, TEXT("SELECTED: %s"), *PlantData->CommonName);
			return FReply::Handled();
		}

		UE_LOG(LogTemp, Warning, TEXT("PlantWrapper: UserDrone or PlantData is null"));
		return FReply::Unhandled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}