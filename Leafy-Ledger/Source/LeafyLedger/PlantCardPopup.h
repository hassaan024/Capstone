// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"
#include "Components/Button.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "ManageSave.h"
#include "PlantCardPopup.generated.h"

class UTextBlock;
class UPlantObject;

UCLASS()
class LEAFYLEDGER_API UPlantCardPopup : public UUserWidget
{
	GENERATED_BODY()
	
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGlobalSaveRemoved, int32, PerenualId);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveStateChanged);

    UPROPERTY(BlueprintAssignable, Category = "Plants")
    FOnGlobalSaveRemoved OnGlobalSaveRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Plants")
    FOnSaveStateChanged OnSaveStateChanged;

    virtual bool Initialize() override;

    virtual void PopulateInfo(UObject* ListItemObject);

    UFUNCTION()
    void OnPressManageSave();

    UFUNCTION()
    void HandleManageSaveApplied(int32 PerenualId, bool bIsGloballySaved);

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_ManageSave;

    UPROPERTY()
    TSubclassOf<UManageSave> ManageSaveClass;

    UPROPERTY()
    UManageSave* ManageSaveInstance = nullptr;

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_CommonName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ScientificName;

private:
    UPROPERTY()
    UPlantObject* PlantCard = nullptr;

    //UPROPERTY()
    //UManageSave* ManageSaveInstance = nullptr;
};
