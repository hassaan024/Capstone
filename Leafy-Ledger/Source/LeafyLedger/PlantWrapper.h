// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Components/TextBlock.h"
#include "Components/Slider.h"
#include "PlantObject.h"
#include "PlantWrapper.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API UPlantWrapper : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TXT_PlantWrapper;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USlider* PWSlider;

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override
	{
		if (UPlantObject* PlantData = Cast<UPlantObject>(ListItemObject))
		{
			TXT_PlantWrapper->SetText(FText::FromString(PlantData->CommonName));
			PWSlider->SetValue(PlantData->SliderValue);
		}
	}
};
