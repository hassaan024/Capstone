// Fill out your copyright notice in the Description page of Project Settings.

#include "PlantTile.h"
#include "BackendApiSubsystem.h"
#include "SavedPlants.h"
#include "PlantObject.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "SavedPlantCacheSubsystem.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UObjectGlobals.h"

bool UPlantTile::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_ManageSave)
    {
        BTN_ManageSave->OnClicked.AddDynamic(this, &UPlantTile::OnPressManageSave);
    }

    if (BTN_OpenPlantCard)
    {
        BTN_OpenPlantCard->OnClicked.AddDynamic(this, &UPlantTile::OnPressOpenPlantCard);
    }

    return true;
}

void UPlantTile::OnPressManageSave()
{
    if (!ManageSaveClass)
    {
        ManageSaveClass = LoadClass<UManageSave>(nullptr, TEXT("/Game/UI/WB_ManageSave.WB_ManageSave_C"));
        if (!ManageSaveClass)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load /Game/UI/WB_ManageSave.WB_ManageSave_C"));
            return;
        }
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
    ManageSaveInstance->OnSaveApplied.AddUniqueDynamic(this, &UPlantTile::HandleManageSaveApplied);
    ManageSaveInstance->AddToViewport(30);
}

void UPlantTile::OnPressOpenPlantCard()
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

    if (CardPopupInstance) {
        CardPopupInstance->RemoveFromParent();
        CardPopupInstance = nullptr;
    }

    CardPopupInstance = CreateWidget<UPlantCardPopup>(GetOwningPlayer(), CardPopupClass);
    if (!CardPopupInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create popup"));
        return;
    }

    CardPopupInstance->OnGlobalSaveRemoved.AddUniqueDynamic(this, &UPlantTile::HandlePopupRemoveClicked);
    CardPopupInstance->AddToViewport();
    CardPopupInstance->PopulateInfo(PlantCard);
}

void UPlantTile::HandlePopupRemoveClicked(int32 PerenualId)
{
    //UE_LOG(LogTemp, Warning, TEXT("Tile received popup remove click for %d"), PerenualId);

    OnRemoveClicked.Broadcast(PerenualId);

    if (CardPopupInstance)
    {
        CardPopupInstance->RemoveFromParent();
        CardPopupInstance = nullptr;
    }
} 

void UPlantTile::HandleManageSaveApplied(int32 PerenualId, bool bIsGloballySaved)
{
    OnSaveStateChanged.Broadcast();

    if (!bIsGloballySaved)
    {
        OnRemoveClicked.Broadcast(PerenualId);
    }
}

void UPlantTile::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    PlantCard = Cast<UPlantObject>(ListItemObject);
    if (!PlantCard) return;

    if (TXT_CommonName)
        TXT_CommonName->SetText(FText::FromString(PlantCard->CommonName));

    if (TXT_ScientificName)
        TXT_ScientificName->SetText(FText::FromString(PlantCard->ScientificName));

    LoadPlantImage(PlantCard->ImgSrcUrl);

    if (PlantCard->ImgSrcUrl.IsEmpty() && PlantCard->PerenualId > 0 && GetGameInstance())
    {
        USavedPlantCacheSubsystem* Cache = GetGameInstance()->GetSubsystem<USavedPlantCacheSubsystem>();
        if (Cache)
        {
            Cache->GetOrLoadPlantDetails(
                PlantCard->PerenualId,
                FBackendPlantDetailsResponse::CreateUObject(this, &UPlantTile::HandlePlantDetailsLoaded)
            );
        }
    }
}

void UPlantTile::HandlePlantDetailsLoaded(bool bSuccess, const FString& Message, const FBackendPlantDto& Plant)
{
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Plant tile details fetch failed: %s"), *Message);
        return;
    }

    if (!PlantCard || PlantCard->PerenualId != Plant.PerenualId)
    {
        return;
    }

    PlantCard->ImgSrcUrl = GetPreferredImageUrl(Plant);
    LoadPlantImage(PlantCard->ImgSrcUrl);
}

void UPlantTile::LoadPlantImage(const FString& ImageUrl)
{
    if (ImageUrl.IsEmpty() || ImageUrl == CurrentUrl)
    {
        return;
    }

    if (!GetGameInstance())
    {
        return;
    }

    USavedPlantCacheSubsystem* ImageCache = GetGameInstance()->GetSubsystem<USavedPlantCacheSubsystem>();
    if (!ImageCache)
    {
        return;
    }

    CurrentUrl = ImageUrl;
    const FString ExpectedUrl = CurrentUrl;

    ImageCache->GetOrLoadImage(
        ExpectedUrl,
        FOnPlantImageReady::CreateWeakLambda(this, [this, ExpectedUrl](UTexture2D* Texture)
            {
                if (!this || !Texture || !IMG_Plant) return;

                if (CurrentUrl != ExpectedUrl) return;

                ApplyPlantImageTexture(Texture);
                QueueDeferredPlantImageCrop(Texture);
            })
    );
}

void UPlantTile::ApplyPlantImageTexture(UTexture2D* Texture)
{
    if (!Texture || !IMG_Plant)
    {
        return;
    }

    ForceLayoutPrepass();

    const FVector2D TargetSize = IMG_Plant->GetCachedGeometry().GetLocalSize();
    if (TargetSize.X <= 0.0f || TargetSize.Y <= 0.0f || Texture->GetSizeX() <= 0 || Texture->GetSizeY() <= 0)
    {
        FSlateBrush Brush = IMG_Plant->Brush;
        Brush.SetResourceObject(Texture);
        Brush.SetUVRegion(FBox2D());
        IMG_Plant->SetBrush(Brush);
        return;
    }

    const float TextureAspect = static_cast<float>(Texture->GetSizeX()) / static_cast<float>(Texture->GetSizeY());
    const float TargetAspect = TargetSize.X / TargetSize.Y;

    FVector2D MinUV(0.0f, 0.0f);
    FVector2D MaxUV(1.0f, 1.0f);

    if (TextureAspect > TargetAspect)
    {
        const float VisibleWidth = TargetAspect / TextureAspect;
        MinUV.X = (1.0f - VisibleWidth) * 0.5f;
        MaxUV.X = MinUV.X + VisibleWidth;
    }
    else if (TextureAspect < TargetAspect)
    {
        const float VisibleHeight = TextureAspect / TargetAspect;
        MinUV.Y = (1.0f - VisibleHeight) * 0.5f;
        MaxUV.Y = MinUV.Y + VisibleHeight;
    }

    FSlateBrush Brush = IMG_Plant->Brush;
    Brush.SetResourceObject(Texture);
    Brush.DrawAs = ESlateBrushDrawType::Image;
    Brush.ImageSize = TargetSize;
    Brush.SetUVRegion(FBox2D(MinUV, MaxUV));
    IMG_Plant->SetBrush(Brush);
}

void UPlantTile::QueueDeferredPlantImageCrop(UTexture2D* Texture)
{
    UWorld* World = GetWorld();
    if (!World || !Texture)
    {
        return;
    }

    World->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [this, Texture]()
            {
                ApplyPlantImageTexture(Texture);

                if (UWorld* InnerWorld = GetWorld())
                {
                    InnerWorld->GetTimerManager().SetTimerForNextTick(
                        FTimerDelegate::CreateWeakLambda(this, [this, Texture]()
                            {
                                ApplyPlantImageTexture(Texture);
                            })
                    );
                }
            })
    );
}

FString UPlantTile::GetPreferredImageUrl(const FBackendPlantDto& Plant)
{
    if (!Plant.ImgSrcUrls.Regular.IsEmpty())
    {
        return Plant.ImgSrcUrls.Regular;
    }

    if (!Plant.ImgSrcUrls.Medium.IsEmpty())
    {
        return Plant.ImgSrcUrls.Medium;
    }

    if (!Plant.ImgSrcUrls.Small.IsEmpty())
    {
        return Plant.ImgSrcUrls.Small;
    }

    if (!Plant.ImgSrcUrls.Thumbnail.IsEmpty())
    {
        return Plant.ImgSrcUrls.Thumbnail;
    }

    return Plant.ImgSrcUrls.Original;
}
