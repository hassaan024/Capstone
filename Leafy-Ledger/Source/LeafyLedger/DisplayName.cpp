// Fill out your copyright notice in the Description page of Project Settings.

#include "DisplayName.h"
#include "OAuthGISubsystem.h" 
#include "Kismet/GameplayStatics.h"
#include "Components/EditableTextBox.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "MenuController.h"

bool UDisplayName::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_GoogleName)
    {
        BTN_GoogleName->OnClicked.AddDynamic(this, &UDisplayName::OnPressGoogleName);
    }

    if (BTN_SubmitName)
    {
        BTN_SubmitName->OnClicked.AddDynamic(this, &UDisplayName::OnPressSubmit);
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UOAuthGISubsystem* Auth = GI->GetSubsystem<UOAuthGISubsystem>())
        {
            const FAuthSession& Session = Auth->GetSession();
            id = Session.id;
            googleDisplayName = Session.googleDisplayName;
        }
    }
    return true;
}

void UDisplayName::OnPressGoogleName()
{
    const FString NewName = googleDisplayName;
    if (NewName.IsEmpty())
    {
        return;
    }

    const FString Url = FString::Printf(TEXT("%s/%s"), *BackendBaseUrl, *id);

    // JSON body
    TSharedPtr<FJsonObject> BodyObj = MakeShared<FJsonObject>();
    BodyObj->SetStringField(TEXT("displayName"), NewName);

    FString BodyStr;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyStr);
    FJsonSerializer::Serialize(BodyObj.ToSharedRef(), Writer);

    //UE_LOG(LogTemp, Warning, TEXT("PATCH %s body=%s"), *Url, *BodyStr);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(Url);
    Req->SetVerb(TEXT("PATCH"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    //if (!BearerToken.IsEmpty())
    //{
    //    Req->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *BearerToken));
    //}

    Req->SetContentAsString(BodyStr);
    Req->OnProcessRequestComplete().BindUObject(this, &UDisplayName::OnPatchUserComplete);

    if (!Req->ProcessRequest())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to start PATCH request"));
    }
}

void UDisplayName::OnPressSubmit()
{
    const FString NewName = ET_DisplayName->GetText().ToString().TrimStartAndEnd();
    if (NewName.IsEmpty())
    {
        return;
    }

    const FString Url = FString::Printf(TEXT("%s/%s"), *BackendBaseUrl, *id);

    // JSON body
    TSharedPtr<FJsonObject> BodyObj = MakeShared<FJsonObject>();
    BodyObj->SetStringField(TEXT("displayName"), NewName);

    FString BodyStr;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyStr);
    FJsonSerializer::Serialize(BodyObj.ToSharedRef(), Writer);

    //UE_LOG(LogTemp, Warning, TEXT("PATCH %s body=%s"), *Url, *BodyStr);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(Url);
    Req->SetVerb(TEXT("PATCH"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

    //if (!BearerToken.IsEmpty())
    //{
    //    Req->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *BearerToken));
    //}

    Req->SetContentAsString(BodyStr);
    Req->OnProcessRequestComplete().BindUObject(this, &UDisplayName::OnPatchUserComplete);

    if (!Req->ProcessRequest())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to start PATCH request"));
    }
}

void UDisplayName::OnPatchUserComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("PATCH failed: no response"));
        return;
    }

    const int32 Code = Response->GetResponseCode();
    const FString RespBody = Response->GetContentAsString();

    //UE_LOG(LogTemp, Log, TEXT("PATCH response code=%d body=%s"), Code, *RespBody);

    AMenuController* MenuController = Cast<AMenuController>(UGameplayStatics::GetPlayerController(this, 0));

    if (Code >= 200 && Code < 300)
    {
        MenuController->ShowMainMenu();
    }
}
