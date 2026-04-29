// Fill out your copyright notice in the Description page of Project Settings.

#include "PlantCardPopup.h"
#include "BackendApiSubsystem.h"
#include "PlantObject.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/TextWidgetTypes.h"
#include "SavedPlantCacheSubsystem.h"
#include "TimerManager.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

bool UPlantCardPopup::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_ManageSave)
    {
        BTN_ManageSave->OnClicked.AddDynamic(this, &UPlantCardPopup::OnPressManageSave);
    }

    ConfigureDetailTextWrapping();

    return true;
}

void UPlantCardPopup::OnPressManageSave()
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

    LoadPlantImage(PlantCard->ImgSrcUrl);
    PopulateKnownDetails();

    if (PlantCard->PerenualId > 0 && GetGameInstance())
    {
        USavedPlantCacheSubsystem* Cache = GetGameInstance()->GetSubsystem<USavedPlantCacheSubsystem>();
        if (Cache)
        {
            Cache->GetOrLoadPlantDetails(
                PlantCard->PerenualId,
                FBackendPlantDetailsResponse::CreateUObject(this, &UPlantCardPopup::HandlePlantDetailsLoaded)
            );
        }
    }
}

void UPlantCardPopup::HandleManageSaveApplied(int32 PerenualId, bool bIsGloballySaved)
{
    OnSaveStateChanged.Broadcast();

    if (!bIsGloballySaved)
    {
        OnGlobalSaveRemoved.Broadcast(PerenualId);
        RemoveFromParent();
    }
}

void UPlantCardPopup::HandlePlantDetailsLoaded(bool bSuccess, const FString& Message, const FBackendPlantDto& Plant)
{
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Plant details fetch failed: %s"), *Message);
        return;
    }

    ApplyPlantDetails(Plant);
}

void UPlantCardPopup::PopulateKnownDetails()
{
    if (!PlantCard) return;

    SetTextOrNA(TXT_Sunlight, PlantCard->Sunlight);
    SetTextOrNA(TXT_Watering, PlantCard->Watering);
    SetTextOrNA(TXT_Maintenance, PlantCard->Maintenance);
    SetTextOrNA(TXT_Type, PlantCard->Type);
    SetTextOrNA(TXT_Hardiness, PlantCard->HardinessZones);
    SetTextOrNA(TXT_BloomDays, PlantCard->DaysToBloom > 0 ? FString::Printf(TEXT("%d Days"), PlantCard->DaysToBloom) : TEXT(""));
    SetTextOrNA(TXT_LifeCycle, PlantCard->LifeCycle);
    SetTextOrNA(TXT_GrowthRate, PlantCard->GrowthRate);

    RefreshTextLayout();
}

void UPlantCardPopup::SetTextOrNA(UTextBlock* TextBlock, const FString& Value)
{
    if (!TextBlock) return;

    const FString TrimmedValue = Value.TrimStartAndEnd();
    ApplyWrapTextAt(TextBlock);
    TextBlock->SetText(FText::FromString(!TrimmedValue.IsEmpty() ? TrimmedValue : TEXT("N/A")));
    TextBlock->InvalidateLayoutAndVolatility();
}

void UPlantCardPopup::ConfigureDetailTextWrapping()
{
    UTextBlock* DetailTextBlocks[] =
    {
        TXT_Sunlight,
        TXT_Watering,
        TXT_Maintenance,
        TXT_Type,
        TXT_Hardiness,
        TXT_BloomDays,
        TXT_LifeCycle,
        TXT_GrowthRate
    };

    for (UTextBlock* TextBlock : DetailTextBlocks)
    {
        if (!TextBlock)
        {
            continue;
        }

        ApplyWrapTextAt(TextBlock);
        TextBlock->InvalidateLayoutAndVolatility();
    }
}

void UPlantCardPopup::ApplyWrapTextAt(UTextBlock* TextBlock)
{
    if (!TextBlock)
    {
        return;
    }

    TextBlock->SetAutoWrapText(true);

    if (FFloatProperty* WrapTextAtProperty = FindFProperty<FFloatProperty>(UTextLayoutWidget::StaticClass(), TEXT("WrapTextAt")))
    {
        WrapTextAtProperty->SetPropertyValue_InContainer(TextBlock, DetailValueWrapTextAt);
    }

    TextBlock->SynchronizeProperties();
}

void UPlantCardPopup::RefreshTextLayout()
{
    RefreshTextLayoutNow();
    QueueDeferredTextLayoutRefresh();
}

void UPlantCardPopup::RefreshTextLayoutNow()
{
    InvalidateLayoutAndVolatility();
    ForceLayoutPrepass();
}

void UPlantCardPopup::QueueDeferredTextLayoutRefresh()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    World->GetTimerManager().SetTimerForNextTick(
        FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                RefreshTextLayoutNow();

                if (UWorld* InnerWorld = GetWorld())
                {
                    InnerWorld->GetTimerManager().SetTimerForNextTick(
                        FTimerDelegate::CreateWeakLambda(this, [this]()
                            {
                                RefreshTextLayoutNow();
                            })
                    );
                }
            })
    );
}

void UPlantCardPopup::ApplyPlantDetails(const FBackendPlantDto& Plant)
{
    if (PlantCard)
    {
        if (!Plant.CommonName.IsEmpty())
        {
            PlantCard->CommonName = Plant.CommonName;
        }
        if (!Plant.ScientificName.IsEmpty())
        {
            PlantCard->ScientificName = Plant.ScientificName;
        }

        PlantCard->Sunlight = Plant.SunlightText;
        PlantCard->Watering = Plant.WateringFreq;
        PlantCard->HardinessZones = Plant.HardinessZones;
        PlantCard->Maintenance = !Plant.Maintenance.IsEmpty() ? Plant.Maintenance : Plant.CareLevel;
        PlantCard->Type = Plant.Type;
        PlantCard->LifeCycle = Plant.Cycle;
        PlantCard->GrowthRate = Plant.GrowthRateText;
        PlantCard->DaysToBloom = Plant.DaysToBloom;

        const FString ImageUrl = GetPreferredImageUrl(Plant);
        if (!ImageUrl.IsEmpty())
        {
            PlantCard->ImgSrcUrl = ImageUrl;
        }
    }

    if (TXT_CommonName && !Plant.CommonName.IsEmpty())
    {
        TXT_CommonName->SetText(FText::FromString(Plant.CommonName));
    }

    if (TXT_ScientificName && !Plant.ScientificName.IsEmpty())
    {
        TXT_ScientificName->SetText(FText::FromString(Plant.ScientificName));
    }

    PopulateKnownDetails();
    if (PlantCard)
    {
        LoadPlantImage(PlantCard->ImgSrcUrl);
    }
}

void UPlantCardPopup::LoadPlantImage(const FString& ImageUrl)
{
    if (ImageUrl.IsEmpty() || ImageUrl == CurrentUrl) return;

    if (!GetGameInstance()) return;

    USavedPlantCacheSubsystem* ImageCache = GetGameInstance()->GetSubsystem<USavedPlantCacheSubsystem>();
    if (!ImageCache) return;

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

void UPlantCardPopup::ApplyPlantImageTexture(UTexture2D* Texture)
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

void UPlantCardPopup::QueueDeferredPlantImageCrop(UTexture2D* Texture)
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

FString UPlantCardPopup::GetPreferredImageUrl(const FBackendPlantDto& Plant)
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
