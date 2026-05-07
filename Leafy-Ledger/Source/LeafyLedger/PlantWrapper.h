// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Input/Reply.h"
#include "PlantWrapper.generated.h"

class UTextBlock;
class USlider;
class AUserDrone;
class UPlantObject;

UCLASS()
class LEAFYLEDGER_API UPlantWrapper : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TXT_PlantWrapper = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Plant")
	AUserDrone* UserDrone = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Plant")
	UPlantObject* PlantData = nullptr;

	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	bool bWasSelected = false;
	bool bHasSelectionState = false;

	void RefreshSelectionState(bool bForce = false);
};
