// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Sockets.h"
#include "OAuthGISubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOAuthLoginPushed, bool, bSuccess, const FString&, ErrorOrSessionToken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoginSucceeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginFailed, const FString&, ErrorMessage);

UCLASS()
class LEAFYLEDGER_API UOAuthGISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnLoginSucceeded OnLoginSucceeded;

	UPROPERTY(BlueprintAssignable)
	FOnLoginFailed OnLoginFailed;

	UFUNCTION(BlueprintCallable)
	bool IsLoggedIn() const { return bLoggedIn; }

	UFUNCTION(BlueprintCallable)
	const FString& GetSessionToken() const { return SessionToken; }

	UPROPERTY(BlueprintAssignable)
	FOAuthLoginPushed OnOAuthLoginPushed;

	UFUNCTION(BlueprintCallable)
	void BeginLoginViaBackendPush();

	UFUNCTION(BlueprintCallable)
	void CancelLoginListener();

private:
	UPROPERTY()
	bool bLoggedIn = false;

	UPROPERTY()
	FString SessionToken;

	// Listener state
	FSocket* ListenSocket = nullptr;
	TAtomic<bool> bListening{ false };
	FIPv4Endpoint BoundEndpoint;

	// Login state
	FString ExpectedSid;
	double StartTimeSeconds = 0.0;
	double TimeoutSeconds = 90.0;

	// Helpers
	bool StartListener();
	void StopListener();

	void AcceptLoop(); // runs on background thread

	// Minimal HTTP parsing helpers
	static bool ReadAllAvailable(FSocket* Socket, TArray<uint8>& OutBytes, double MaxSeconds);
	static bool SplitHeadersBody(const FString& Request, FString& OutHeaders, FString& OutBody);
	static bool ParseRequestLine(const FString& Headers, FString& OutMethod, FString& OutPath);
	static int32 FindContentLength(const FString& Headers);
	static FString MakeHttpResponse(int32 Code, const FString& Body);
};
