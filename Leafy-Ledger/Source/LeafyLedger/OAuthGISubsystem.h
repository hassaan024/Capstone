// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Sockets.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"
#include "SocketSubsystem.h"
#include "Networking.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Misc/Base64.h"
#include "Misc/Guid.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Misc/ScopedSlowTask.h"
#include "Kismet/KismetSystemLibrary.h"
#include "OAuthGISubsystem.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGoogleLoginResult, bool, bSuccess, const FString&, ErrorOrIdToken);
DECLARE_MULTICAST_DELEGATE(FOnLoginSucceeded);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoginFailed, const FString& /*Error*/);

UCLASS()
class LEAFYLEDGER_API UOAuthGISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FGoogleLoginResult OnGoogleLoginResult;

    UFUNCTION(BlueprintCallable)
    void BeginGoogleLogin();

    FOnLoginSucceeded OnLoginSucceeded;
    FOnLoginFailed OnLoginFailed;

    bool IsLoggedIn() const { return bLoggedIn; }

    // Call this once your OAuth callback exchange finishes successfully
    void MarkLoginSucceeded()
    {
        bLoggedIn = true;
        OnLoginSucceeded.Broadcast();
    }

    void MarkLoginFailed(const FString& Error)
    {
        bLoggedIn = false;
        OnLoginFailed.Broadcast(Error);
    }

private:
    // Config
    FString ClientId = TEXT("1083171967667-uepovjdmlhq1ah0dvjdkhefrteh4ujhj.apps.googleusercontent.com");
    FString ClientSecret = TEXT("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"); // OK for prototyping; see note below.
    FString Scope = TEXT("openid email profile");

    // Loopback listener state
    FSocket* ListenSocket = nullptr;
    FIPv4Endpoint LocalEndpoint;
    TAtomic<bool> bListening{ false };

    // Helpers
    bool StartLoopbackListener();
    void StopLoopbackListener();

    void OpenSystemBrowser(const FString& AuthUrl);

    void RunAcceptLoop(); // runs on background thread
    bool ParseRequestForCode(const FString& HttpRequest, FString& OutCode);

    void ExchangeCodeForTokens(const FString& Code, const FString& RedirectUri);

    bool bLoggedIn = false;
};



//UCLASS()
//class YOURPROJECT_API UYourGoogleAuthSubsystem : public UGameInstanceSubsystem
//{
//    GENERATED_BODY()
//
//public:
//    FOnLoginSucceeded OnLoginSucceeded;
//    FOnLoginFailed OnLoginFailed;
//
//    bool IsLoggedIn() const { return bLoggedIn; }
//
//    // Call this once your OAuth callback exchange finishes successfully
//    void MarkLoginSucceeded()
//    {
//        bLoggedIn = true;
//        OnLoginSucceeded.Broadcast();
//    }
//
//    void MarkLoginFailed(const FString& Error)
//    {
//        bLoggedIn = false;
//        OnLoginFailed.Broadcast(Error);
//    }
//
//private:
//    bool bLoggedIn = false;
//};
