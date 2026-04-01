// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"
#include "Components/Button.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "PlantCardPopup.generated.h"

class UTextBlock;
class UPlantObject;

UCLASS()
class LEAFYLEDGER_API UPlantCardPopup : public UUserWidget
{
	GENERATED_BODY()
	
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemoveClicked, int32, PerenualId);

    UPROPERTY(BlueprintAssignable, Category = "Plants")
    FOnRemoveClicked OnRemoveClicked;

    virtual bool Initialize() override;

    virtual void PopulateInfo(UObject* ListItemObject);

    UFUNCTION()
    void OnPressUnsavePlant();

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_UnsavePlant;

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_CommonName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ScientificName;

private:
    UPROPERTY()
    UPlantObject* PlantCard = nullptr;
};
