// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"
#include "Components/Button.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "PlantCardPopup.h"
#include "ManageSave.h"
#include "PlantTile.generated.h"

/**
 * 
 */
class UImage;
class UTextBlock;
class UPlantObject;

UCLASS()
class LEAFYLEDGER_API UPlantTile : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemoveClicked, int32, PerenualId);

    UPROPERTY(BlueprintAssignable, Category = "Plants")
    FOnRemoveClicked OnRemoveClicked;

    virtual bool Initialize() override;

    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

    UFUNCTION()
    void OnPressManageSave();

    UFUNCTION()
    void OnPressOpenPlantCard();

    UFUNCTION()
    void HandlePopupRemoveClicked(int32 PerenualId);

    UFUNCTION()
    void HandleManageSaveApplied(int32 PerenualId, bool bIsGloballySaved);

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_ManageSave;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_OpenPlantCard;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UPlantCardPopup> CardPopupClass;

    UPROPERTY()
    UPlantCardPopup* CardPopupInstance = nullptr;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UManageSave> ManageSaveClass;

    UPROPERTY()
    UManageSave* ManageSaveInstance = nullptr;

protected:
    UPROPERTY(meta = (BindWidget))
    UImage* IMG_Plant;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_CommonName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ScientificName;

private:
    UPROPERTY()
    UPlantObject* PlantCard = nullptr;

    FString CurrentUrl;
};
