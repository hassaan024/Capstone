// Fill out your copyright notice in the Description page of Project Settings.

#include "SavedPlantTile.h"
#include "SavedPlants.h"
#include "PlantObject.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

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

static EImageFormat DetectImageFormat(const TArray<uint8>& Bytes)
{
    // JPEG: FF D8 FF
    if (Bytes.Num() >= 3 && Bytes[0] == 0xFF && Bytes[1] == 0xD8 && Bytes[2] == 0xFF)
        return EImageFormat::JPEG;

    // PNG: 89 50 4E 47 0D 0A 1A 0A
    if (Bytes.Num() >= 8 &&
        Bytes[0] == 0x89 && Bytes[1] == 0x50 && Bytes[2] == 0x4E && Bytes[3] == 0x47 &&
        Bytes[4] == 0x0D && Bytes[5] == 0x0A && Bytes[6] == 0x1A && Bytes[7] == 0x0A)
        return EImageFormat::PNG;

    return EImageFormat::Invalid;
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
        DownloadImage(CurrentUrl);
    }
}

void USavedPlantTile::DownloadImage(const FString& Url)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(Url);
    Req->SetVerb(TEXT("GET"));
    Req->SetHeader(TEXT("Accept"), TEXT("*/*"));
    Req->OnProcessRequestComplete().BindUObject(this, &USavedPlantTile::OnImageDownloaded);
    Req->ProcessRequest();
}

void USavedPlantTile::OnImageDownloaded(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
{
    if (!bSuccess || !Resp.IsValid() || !IMG_Plant) return;

    const TArray<uint8>& Bytes = Resp->GetContent();
    if (Bytes.Num() == 0) return;

    const EImageFormat Format = DetectImageFormat(Bytes);
    if (Format == EImageFormat::Invalid) return;

    IImageWrapperModule& ImageWrapperModule =
        FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

    TSharedPtr<IImageWrapper> Wrapper = ImageWrapperModule.CreateImageWrapper(Format);
    if (!Wrapper.IsValid()) return;

    if (!Wrapper->SetCompressed(Bytes.GetData(), Bytes.Num())) return;

    TArray<uint8> RawBGRA;
    if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, RawBGRA)) return;

    const int32 Width = Wrapper->GetWidth();
    const int32 Height = Wrapper->GetHeight();

    UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
    if (!Texture) return;

    Texture->SRGB = true;

    FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
    void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(Data, RawBGRA.GetData(), RawBGRA.Num());
    Mip.BulkData.Unlock();

    Texture->UpdateResource();

    IMG_Plant->SetBrushFromTexture(Texture, true);
}