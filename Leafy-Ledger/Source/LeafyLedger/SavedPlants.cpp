// Fill out your copyright notice in the Description page of Project Settings.


#include "SavedPlants.h"
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

    // Pull when page is constructed/shown (recommended)
    FetchSavedSpecies();
}

void USavedPlants::FetchSavedSpecies()
{
    // Your guess endpoint; adjust if needed
    UserId = 1;
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

    // Case A: root is array
    if (RootValue->Type == EJson::Array)
    {
        OutArray = RootValue->AsArray();
        return true;
    }

    // Case B: root is object with "data" array
    if (RootValue->Type == EJson::Object)
    {
        TSharedPtr<FJsonObject> RootObj = RootValue->AsObject();
        if (!RootObj.IsValid()) return false;

        const TArray<TSharedPtr<FJsonValue>>* DataArrayPtr = nullptr;
        if (RootObj->TryGetArrayField(TEXT("data"), DataArrayPtr) && DataArrayPtr)
        {
            OutArray = *DataArrayPtr;
            return true;
        }
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
    else {
        UE_LOG(LogTemp, Warning, TEXT("Response was NOT valid"));
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

    // Clear and repopulate ListView
    TV_PlantCards->ClearListItems();

    for (const TSharedPtr<FJsonValue>& ItemVal : SpeciesArray)
    {
        if (!ItemVal.IsValid() || ItemVal->Type != EJson::Object) continue;

        const TSharedPtr<FJsonObject> Obj = ItemVal->AsObject();
        if (!Obj.IsValid()) continue;

        FString CommonName;
        Obj->TryGetStringField(TEXT("commonName"), CommonName);

        UPlantObject* PlantObject = NewObject<UPlantObject>(this);
        PlantObject->CommonName = CommonName;

        TV_PlantCards->AddItem(PlantObject);
    }

    // Optional: hide loading UI here
}