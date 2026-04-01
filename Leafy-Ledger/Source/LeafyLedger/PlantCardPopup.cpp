// Fill out your copyright notice in the Description page of Project Settings.


#include "PlantCardPopup.h"
#include "PlantObject.h"
#include "Components/TextBlock.h"

bool UPlantCardPopup::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_UnsavePlant)
    {
        BTN_UnsavePlant->OnClicked.AddDynamic(this, &UPlantCardPopup::OnPressUnsavePlant);
    }

    return true;
}

void UPlantCardPopup::OnPressUnsavePlant()
{
    if (!PlantCard)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unsave clicked but PlantCard is null"));
        return;
    }
    OnRemoveClicked.Broadcast(PlantCard->PerenualId);
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