// Fill out your copyright notice in the Description page of Project Settings.

#include "PlantCardPopup.h"
#include "PlantObject.h"
#include "Components/TextBlock.h"
#include "UObject/UObjectGlobals.h"

bool UPlantCardPopup::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_ManageSave)
    {
        BTN_ManageSave->OnClicked.AddDynamic(this, &UPlantCardPopup::OnPressManageSave);
    }

    return true;
}

void UPlantCardPopup::OnPressManageSave()
{
    if (!ManageSaveClass)
    {
        UE_LOG(LogTemp, Error, TEXT("ManageSaveClass is null"));
        return;
    }

    if (!PlantCard)
    {
        UE_LOG(LogTemp, Warning, TEXT("Manage save clicked but PlantCard is null"));
        return;
    }
    
    if (ManageSaveInstance)
    {
        ManageSaveInstance->RemoveFromParent();
        ManageSaveInstance = nullptr;
    }

    ManageSaveInstance = CreateWidget<UManageSave>(GetOwningPlayer(), ManageSaveClass);
    if (!ManageSaveInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create ManageSave widget"));
        return;
    }

    ManageSaveInstance->ConfigureForPlant(PlantCard->PerenualId, PlantCard->CommonName);
    ManageSaveInstance->OnSaveApplied.AddUniqueDynamic(this, &UPlantCardPopup::HandleManageSaveApplied);
    ManageSaveInstance->AddToViewport(30);
}

void UPlantCardPopup::PopulateInfo(UObject* ListItemObject)
{
    PlantCard = Cast<UPlantObject>(ListItemObject);
    if (!PlantCard) return;

    if (TXT_CommonName)
        TXT_CommonName->SetText(FText::FromString(PlantCard->CommonName));

    if (TXT_ScientificName)
        TXT_ScientificName->SetText(FText::FromString(PlantCard->ScientificName));
}

void UPlantCardPopup::HandleManageSaveApplied(int32 PerenualId, bool bIsGloballySaved)
{
    if (!bIsGloballySaved)
    {
        OnGlobalSaveRemoved.Broadcast(PerenualId);
        RemoveFromParent();
    }
}
