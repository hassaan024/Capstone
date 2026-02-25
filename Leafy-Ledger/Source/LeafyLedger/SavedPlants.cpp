// Fill out your copyright notice in the Description page of Project Settings.

#include "SavedPlants.h"
#include "SavedPlantTile.h"
#include "PlantObject.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void USavedPlants::NativeConstruct()
{
    Super::NativeConstruct();

    if (!TV_PlantCards)
    {
        UE_LOG(LogTemp, Error, TEXT("SavedPlantsList is not bound. Make sure ListView is named SavedPlantsList and 'Is Variable' is checked."));
        return;
    }
    TV_PlantCards->OnEntryWidgetGenerated().AddUObject(this, &USavedPlants::HandleEntryGenerated);

    // Pull when page is constructed/shown (recommended)
    FetchSavedSpecies();
}

void USavedPlants::HandleEntryGenerated(UUserWidget& EntryWidget)
{
    if (USavedPlantTile* Tile = Cast<USavedPlantTile>(&EntryWidget))
    {
        Tile->OnRemoveClicked.AddUniqueDynamic(this, &USavedPlants::HandleRemoveClicked);
    }
}

void USavedPlants::HandleRemoveClicked(int32 TrefleId)
{
    UE_LOG(LogTemp, Warning, TEXT("Page received remove trefleId=%d"), TrefleId);
    DeleteSavedPlant(TrefleId);              // your HTTP DELETE call
}

void USavedPlants::FetchSavedSpecies()
{
    UOAuthGISubsystem* OAuth = GetGameInstance()->GetSubsystem<UOAuthGISubsystem>();

    if (!OAuth)
    {
        UE_LOG(LogTemp, Error, TEXT("OAuth subsystem not found"));
        return;
    }

    UserId = FCString::Atoi(*OAuth->Session.id);

    //UserId = 1;
    const FString Url = FString::Printf(TEXT("%s/species/saved?userId=%d"), *BackendBaseUrl, UserId);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(Url);
    Req->SetVerb(TEXT("GET"));
    Req->SetHeader(TEXT("Accept"), TEXT("application/json"));

    //if (!AuthToken.IsEmpty())
    //{
    //    Req->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
    //}

    Req->OnProcessRequestComplete().BindUObject(this, &USavedPlants::OnFetchSavedSpeciesComplete);
    Req->ProcessRequest();

    // Optional: show loading UI here
}

static bool TryGetSpeciesArray(const TSharedPtr<FJsonValue>& RootValue, TArray<TSharedPtr<FJsonValue>>& OutArray)
{
    if (!RootValue.IsValid()) return false;

    if (RootValue->Type == EJson::Array)
    {
        OutArray = RootValue->AsArray();
        return true;
    }

    return false;
}

void USavedPlants::OnFetchSavedSpeciesComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    UE_LOG(LogTemp, Warning, TEXT("HTTP callback triggered. Success: %s"),
        bWasSuccessful ? TEXT("true") : TEXT("false"));

    if (Response.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Response Code: %d"), Response->GetResponseCode());
        UE_LOG(LogTemp, Warning, TEXT("Raw Body: %s"), *Response->GetContentAsString());
    }

    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("FetchSavedSpecies failed: no response"));
        return;
    }

    const int32 Code = Response->GetResponseCode();
    const FString Body = Response->GetContentAsString();

    if (Code < 200 || Code >= 300)
    {
        UE_LOG(LogTemp, Error, TEXT("FetchSavedSpecies HTTP %d: %s"), Code, *Body);
        return;
    }

    // Parse JSON
    TSharedPtr<FJsonValue> RootValue;
    {
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
        if (!FJsonSerializer::Deserialize(Reader, RootValue) || !RootValue.IsValid())
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON: %s"), *Body);
            return;
        }
    }

    TArray<TSharedPtr<FJsonValue>> SpeciesArray;
    if (!TryGetSpeciesArray(RootValue, SpeciesArray))
    {
        UE_LOG(LogTemp, Error, TEXT("JSON was not an array and did not contain data[]: %s"), *Body);
        return;
    }

    // Clear and repopulate TileView
    TV_PlantCards->ClearListItems();

    for (const TSharedPtr<FJsonValue>& ItemVal : SpeciesArray)
    {
        if (!ItemVal.IsValid() || ItemVal->Type != EJson::Object) continue;

        const TSharedPtr<FJsonObject> Obj = ItemVal->AsObject();
        if (!Obj.IsValid()) continue;

        FString CommonName, ScientificName, ImgUrl;
        int32 TrefleId = 0;
        Obj->TryGetStringField(TEXT("commonName"), CommonName);
        Obj->TryGetStringField(TEXT("scientificName"), ScientificName);
        Obj->TryGetStringField(TEXT("imgSrcUrl"), ImgUrl);
        Obj->TryGetNumberField(TEXT("trefleId"), TrefleId);

        UPlantObject* PlantObject = NewObject<UPlantObject>(this);
        PlantObject->CommonName = CommonName;
        PlantObject->ScientificName = ScientificName;
        PlantObject->ImgSrcUrl = ImgUrl;
        PlantObject->TrefleId = TrefleId;

        TV_PlantCards->AddItem(PlantObject);

        UE_LOG(LogTemp, Warning, TEXT("Added PlantObject"));
    }

    // Optional: hide loading UI here
}

void USavedPlants::DeleteSavedPlant(int32 TrefleId)
{
    UOAuthGISubsystem* OAuth = GetGameInstance()->GetSubsystem<UOAuthGISubsystem>();

    if (!OAuth)
    {
        UE_LOG(LogTemp, Error, TEXT("OAuth subsystem not found"));
        return;
    }

    UserId = FCString::Atoi(*OAuth->Session.id);

    //UserId = 1;
    const FString Url = FString::Printf(
        TEXT("%s/species/save/%d?userId=%d"),
        *BackendBaseUrl,
        TrefleId,
        UserId
    );

    UE_LOG(LogTemp, Warning, TEXT("DELETE URL: %s"), *Url);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(Url);
    Req->SetVerb(TEXT("DELETE"));
    Req->SetHeader(TEXT("Accept"), TEXT("application/json"));

    //if (!AuthToken.IsEmpty())
    //    Req->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));

    Req->OnProcessRequestComplete().BindLambda(
        [this, TrefleId](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
        {
            if (!bSuccess || !Response.IsValid())
            {
                UE_LOG(LogTemp, Error, TEXT("Delete failed (no response) trefleId=%d"), TrefleId);
                return;
            }

            UE_LOG(LogTemp, Warning, TEXT("Delete code=%d body=%s"),
                Response->GetResponseCode(),
                *Response->GetContentAsString());

            if (Response->GetResponseCode() >= 200 && Response->GetResponseCode() < 300)
            {
                RemoveSavedPlantFromList(TrefleId); // or FetchSavedSpecies()
            }
        }
    );

    Req->ProcessRequest();
}

void USavedPlants::RemoveSavedPlantFromList(int32 TrefleId)
{
    if (!TV_PlantCards) return;

    TArray<UObject*> Items = TV_PlantCards->GetListItems();
    for (UObject* Obj : Items)
    {
        UPlantObject* Plant = Cast<UPlantObject>(Obj);
        if (Plant && Plant->TrefleId == TrefleId)
        {
            TV_PlantCards->RemoveItem(Obj);
            TV_PlantCards->RequestRefresh();
            return;
        }
    }
}