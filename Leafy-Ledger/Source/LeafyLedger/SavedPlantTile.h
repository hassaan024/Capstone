// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"
#include "Components/Button.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "PlantCardPopup.h"
#include "SavedPlantTile.generated.h"

/**
 * 
 */
class UImage;
class UTextBlock;
class UPlantObject;

UCLASS()
class LEAFYLEDGER_API USavedPlantTile : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemoveClicked, int32, PerenualId);

    UPROPERTY(BlueprintAssignable, Category = "Plants")
    FOnRemoveClicked OnRemoveClicked;

    virtual bool Initialize() override;

    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

    UFUNCTION()
    void OnPressUnsavePlant();

    UFUNCTION()
    void OnPressOpenPlantCard();

    UFUNCTION()
    void HandlePopupRemoveClicked(int32 PerenualId);

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_UnsavePlant;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_OpenPlantCard;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UPlantCardPopup> CardPopupClass;

    UPROPERTY()
    UPlantCardPopup* CardPopupInstance = nullptr;

protected:
    UPROPERTY(meta = (BindWidget))
    UImage* IMG_Plant;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_CommonName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ScientificName;

private:
    void DownloadImage(const FString& Url);
    void OnImageDownloaded(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess);

    UPROPERTY()
    UPlantObject* PlantCard = nullptr;

    FString CurrentUrl;
};