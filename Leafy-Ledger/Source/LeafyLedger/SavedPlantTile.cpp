// Fill out your copyright notice in the Description page of Project Settings.

#include "SavedPlantTile.h"
#include "SavedPlants.h"
#include "PlantObject.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "SavedPlantCacheSubsystem.h"

bool USavedPlantTile::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_UnsavePlant)
    {
        BTN_UnsavePlant->OnClicked.AddDynamic(this, &USavedPlantTile::OnPressUnsavePlant);
    }

    if (BTN_OpenPlantCard)
    {
        BTN_OpenPlantCard->OnClicked.AddDynamic(this, &USavedPlantTile::OnPressOpenPlantCard);
    }

    return true;
}

void USavedPlantTile::OnPressUnsavePlant()
{
    if (!PlantCard)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unsave clicked but PlantCard is null"));
        return;
    }
    OnRemoveClicked.Broadcast(PlantCard->PerenualId);
}

void USavedPlantTile::OnPressOpenPlantCard()
{
    if (!CardPopupClass)
    {
        UE_LOG(LogTemp, Error, TEXT("CardPopupClass is null"));
        return;
    }

    if (!PlantCard)
    {
        UE_LOG(LogTemp, Error, TEXT("PlantCard is null"));
        return;
    }

    CardPopupInstance = CreateWidget<UPlantCardPopup>(GetOwningPlayer(), CardPopupClass);
    if (!CardPopupInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create popup"));
        return;
    }

    CardPopupInstance->PopulateInfo(PlantCard);

    CardPopupInstance->OnRemoveClicked.AddUniqueDynamic(this, &USavedPlantTile::HandlePopupRemoveClicked);

    CardPopupInstance->AddToViewport();
}

void USavedPlantTile::HandlePopupRemoveClicked(int32 PerenualId)
{
    //UE_LOG(LogTemp, Warning, TEXT("Tile received popup remove click for %d"), PerenualId);

    OnRemoveClicked.Broadcast(PerenualId);

    if (CardPopupInstance)
    {
        CardPopupInstance->RemoveFromParent();
        CardPopupInstance = nullptr;
    }
} 

void USavedPlantTile::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    PlantCard = Cast<UPlantObject>(ListItemObject);
    if (!PlantCard) return;

    if (TXT_CommonName)
        TXT_CommonName->SetText(FText::FromString(PlantCard->CommonName));

    if (TXT_ScientificName)
        TXT_ScientificName->SetText(FText::FromString(PlantCard->ScientificName));

    if (!PlantCard->ImgSrcUrl.IsEmpty() && PlantCard->ImgSrcUrl != CurrentUrl)
    {
        CurrentUrl = PlantCard->ImgSrcUrl;

        if (!GetGameInstance())
        {
            return;
        }

        USavedPlantCacheSubsystem* ImageCache =
            GetGameInstance()->GetSubsystem<USavedPlantCacheSubsystem>();

        if (!ImageCache)
        {
            return;
        }

        const FString ExpectedUrl = CurrentUrl;

        ImageCache->GetOrLoadImage(
            ExpectedUrl,
            FOnPlantImageReady::CreateWeakLambda(this, [this, ExpectedUrl](UTexture2D* Texture)
                {
                    if (!this || !Texture || !IMG_Plant)
                    {
                        return;
                    }

                    if (CurrentUrl != ExpectedUrl)
                    {
                        return;
                    }

                    IMG_Plant->SetBrushFromTexture(Texture, true);
                })
        );
    }
}