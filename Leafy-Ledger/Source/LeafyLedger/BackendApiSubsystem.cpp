// Fill out your copyright notice in the Description page of Project Settings.

#include "BackendApiSubsystem.h"
#include "BackendJsonUtils.h"
#include "OAuthGISubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> UBackendApiSubsystem::CreateRequest(
	const FString& Route,
	const FString& Verb,
	const FString& JsonBody
)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(BaseUrl + Route);
	Req->SetVerb(Verb);

	AddDefaultHeaders(Req);
	AddAuthHeader(Req);

	if (!JsonBody.IsEmpty())
	{
		Req->SetContentAsString(JsonBody);
	}

	return Req;
}

void UBackendApiSubsystem::AddDefaultHeaders(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request)
{
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
}

void UBackendApiSubsystem::AddAuthHeader(TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request)
{
	if (!GetGameInstance())
	{
		return;
	}

	UOAuthGISubsystem* OAuth = GetGameInstance()->GetSubsystem<UOAuthGISubsystem>();
	if (!OAuth)
	{
		return;
	}

	const FString Token = OAuth->GetSessionToken();
	if (!Token.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Token));
	}
}

bool UBackendApiSubsystem::TryGetUserId(int32& OutUserId, FString& OutError) const
{
	OutUserId = 0;
	OutError = TEXT("");

	if (!GetGameInstance())
	{
		OutError = TEXT("No GameInstance");
		return false;
	}

	UOAuthGISubsystem* OAuth = GetGameInstance()->GetSubsystem<UOAuthGISubsystem>();
	if (!OAuth)
	{
		OutError = TEXT("OAuth subsystem missing");
		return false;
	}

	const FString SessionId = OAuth->GetSession().id;
	if (SessionId.IsEmpty())
	{
		OutError = TEXT("Session id is empty");
		return false;
	}

	OutUserId = FCString::Atoi(*SessionId);
	if (OutUserId <= 0)
	{
		OutError = FString::Printf(TEXT("Invalid session id: %s"), *SessionId);
		return false;
	}

	return true;
}

bool UBackendApiSubsystem::IsHttpSuccess(FHttpResponsePtr Response)
{
	return Response.IsValid()
		&& Response->GetResponseCode() >= 200
		&& Response->GetResponseCode() < 300;
}

FString UBackendApiSubsystem::BuildErrorMessage(FHttpResponsePtr Response, const FString& Fallback)
{
	if (!Response.IsValid())
	{
		return Fallback;
	}

	FString ErrorMessage;
	const FString Body = Response->GetContentAsString();

	if (FBackendJsonUtils::TryGetErrorMessage(Body, ErrorMessage) && !ErrorMessage.IsEmpty())
	{
		return ErrorMessage;
	}

	if (!Body.IsEmpty())
	{
		return Body;
	}

	return FString::Printf(TEXT("%s (HTTP %d)"), *Fallback, Response->GetResponseCode());
}

void UBackendApiSubsystem::GetSavedSpecies(const FBackendPlantsResponse& Callback)
{
	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		const TArray<FBackendPlantDto> EmptyPlants;
		Callback.ExecuteIfBound(false, Error, EmptyPlants);
		return;
	}

	const FString Route = FString::Printf(TEXT("/species/saved?userId=%d"), UserId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("GET"));

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			TArray<FBackendPlantDto> Plants;

			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"), Plants);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to fetch saved species")),
					Plants
				);
				return;
			}

			const FString Body = Response->GetContentAsString();
			if (!FBackendJsonUtils::ParsePlantArray(Body, Plants))
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid saved species JSON"), Plants);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("OK"), Plants);
		}
	);

	if (!Req->ProcessRequest())
	{
		const TArray<FBackendPlantDto> EmptyPlants;
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), EmptyPlants);
	}
}

void UBackendApiSubsystem::DeleteSavedPlant(int32 TrefleId, const FBackendOperationResponse& Callback)
{
	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		Callback.ExecuteIfBound(false, Error);
		return;
	}

	const FString Route = FString::Printf(TEXT("/species/save/%d?userId=%d"), TrefleId, UserId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("DELETE"));

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"));
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to delete saved plant"))
				);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("Saved plant deleted"));
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"));
	}
}

void UBackendApiSubsystem::UpdateDisplayName(const FString& NewDisplayName, const FBackendOperationResponse& Callback)
{
	const FString TrimmedName = NewDisplayName.TrimStartAndEnd();
	if (TrimmedName.IsEmpty())
	{
		Callback.ExecuteIfBound(false, TEXT("Display name is empty"));
		return;
	}

	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		Callback.ExecuteIfBound(false, Error);
		return;
	}

	const FString Route = FString::Printf(TEXT("/user/%d"), UserId);

	TSharedRef<FJsonObject> BodyObj = MakeShared<FJsonObject>();
	BodyObj->SetStringField(TEXT("displayName"), TrimmedName);
	const FString BodyStr = FBackendJsonUtils::StringifyObject(BodyObj);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("PATCH"), BodyStr);

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"));
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to update display name"))
				);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("Display name updated"));
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"));
	}
}

void UBackendApiSubsystem::GetCurrentUser(const FBackendCurrentUserResponse& Callback)
{
	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		Callback.ExecuteIfBound(false, Error, FBackendUserDto{});
		return;
	}

	const FString Route = FString::Printf(TEXT("/user/%d"), UserId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("GET"));

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			FBackendUserDto User;

			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"), User);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to fetch current user")),
					User
				);
				return;
			}

			const FString Body = Response->GetContentAsString();
			if (!FBackendJsonUtils::ParseCurrentUser(Body, User))
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid current user JSON"), User);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("OK"), User);
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), FBackendUserDto{});
	}
}