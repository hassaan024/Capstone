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

void UBackendApiSubsystem::DeleteSavedPlant(int32 PerenualId, const FBackendOperationResponse& Callback)
{
	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		Callback.ExecuteIfBound(false, Error);
		return;
	}

	const FString Route = FString::Printf(TEXT("/species/save/%d?userId=%d"), PerenualId, UserId);
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

void UBackendApiSubsystem::GetUserLocation(const FBackendUserLocationResponse& Callback)
{
	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		Callback.ExecuteIfBound(false, Error, FBackendUserLocationDto{});
		return;
	}

	const FString Route = FString::Printf(TEXT("/user/location/%d"), UserId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("GET"));

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			FBackendUserLocationDto Location;

			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"), Location);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to fetch user location")),
					Location
				);
				return;
			}

			const FString Body = Response->GetContentAsString();
			if (!FBackendJsonUtils::ParseUserLocation(Body, Location))
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid user location JSON"), Location);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("OK"), Location);
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), FBackendUserLocationDto{});
	}
}

void UBackendApiSubsystem::GetCurrentWeather(float Latitude, float Longitude, const FBackendWeatherResponse& Callback)
{
	const FString Route = FString::Printf(
		TEXT("/weather/current?latitude=%.6f&longitude=%.6f"),
		Latitude,
		Longitude
	);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("GET"));

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			FBackendWeatherDto Weather;

			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"), Weather);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to fetch current weather")),
					Weather
				);
				return;
			}

			const FString Body = Response->GetContentAsString();
			if (!FBackendJsonUtils::ParseWeather(Body, Weather))
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid weather JSON"), Weather);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("OK"), Weather);
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), FBackendWeatherDto{});
	}
}

void UBackendApiSubsystem::CreateGarden(const FString& Name, const FString& Description, float Latitude, float Longitude, const FString& Timezone, const FBackendGardenResponse& Callback)
{
	const FString TrimmedName = Name.TrimStartAndEnd();
	if (TrimmedName.IsEmpty())
	{
		Callback.ExecuteIfBound(false, TEXT("Garden name is empty"), FBackendGardenDto{});
		return;
	}

	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		Callback.ExecuteIfBound(false, Error, FBackendGardenDto{});
		return;
	}

	TSharedRef<FJsonObject> BodyObj = MakeShared<FJsonObject>();
	BodyObj->SetNumberField(TEXT("ownerId"), UserId);
	BodyObj->SetStringField(TEXT("name"), TrimmedName);
	BodyObj->SetStringField(TEXT("description"), Description);
	BodyObj->SetNumberField(TEXT("latitude"), Latitude);
	BodyObj->SetNumberField(TEXT("longitude"), Longitude);

	if (!Timezone.IsEmpty())
	{
		BodyObj->SetStringField(TEXT("timezone"), Timezone);
	}

	const FString BodyStr = FBackendJsonUtils::StringifyObject(BodyObj);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(TEXT("/garden"), TEXT("POST"), BodyStr);

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			FBackendGardenDto Garden;

			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"), Garden);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to create garden")),
					Garden
				);
				return;
			}

			TSharedPtr<FJsonObject> Obj;
			if (!FBackendJsonUtils::ParseObject(Response->GetContentAsString(), Obj) || !Obj.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid garden JSON"), Garden);
				return;
			}

			double Num = 0.0;

			if (Obj->TryGetNumberField(TEXT("id"), Num))
			{
				Garden.Id = static_cast<int32>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("ownerId"), Num))
			{
				Garden.OwnerId = static_cast<int32>(Num);
			}

			Obj->TryGetStringField(TEXT("name"), Garden.Name);
			Obj->TryGetStringField(TEXT("description"), Garden.Description);

			if (Obj->TryGetNumberField(TEXT("latitude"), Num))
			{
				Garden.Latitude = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("longitude"), Num))
			{
				Garden.Longitude = static_cast<float>(Num);
			}

			Obj->TryGetStringField(TEXT("timezone"), Garden.Timezone);
			Obj->TryGetStringField(TEXT("creationTimestamp"), Garden.CreationTimestamp);
			Obj->TryGetStringField(TEXT("lastUpdated"), Garden.LastUpdated);

			Callback.ExecuteIfBound(true, TEXT("Garden created"), Garden);
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), FBackendGardenDto{});
	}
}

void UBackendApiSubsystem::CreatePlantInstance(int32 GardenId, int32 SpeciesId, int32 SoilId, const float* HeightCm, const int32* AgeDays, const FString* HealthStatus, const FString* LastWateredIso8601, const FString& Notes, const FBackendPlantInstanceResponse& Callback)
{
	if (GardenId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("GardenId must be > 0"), FBackendPlantInstanceDto{});
		return;
	}

	if (SpeciesId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("SpeciesId must be > 0"), FBackendPlantInstanceDto{});
		return;
	}

	if (SoilId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("SoilId must be > 0"), FBackendPlantInstanceDto{});
		return;
	}

	TSharedRef<FJsonObject> BodyObj = MakeShared<FJsonObject>();
	BodyObj->SetNumberField(TEXT("gardenId"), GardenId);
	BodyObj->SetNumberField(TEXT("speciesId"), SpeciesId);
	BodyObj->SetNumberField(TEXT("soilId"), SoilId);

	if (HeightCm)
	{
		BodyObj->SetNumberField(TEXT("heightCm"), *HeightCm);
	}

	if (AgeDays)
	{
		BodyObj->SetNumberField(TEXT("ageDays"), *AgeDays);
	}

	if (HealthStatus && !HealthStatus->IsEmpty())
	{
		BodyObj->SetStringField(TEXT("healthStatus"), *HealthStatus);
	}

	if (LastWateredIso8601 && !LastWateredIso8601->IsEmpty())
	{
		BodyObj->SetStringField(TEXT("lastWatered"), *LastWateredIso8601);
	}

	if (!Notes.IsEmpty())
	{
		BodyObj->SetStringField(TEXT("notes"), Notes);
	}

	const FString BodyStr = FBackendJsonUtils::StringifyObject(BodyObj);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(TEXT("/plant-instance"), TEXT("POST"), BodyStr);

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			FBackendPlantInstanceDto PlantInstance;

			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"), PlantInstance);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to create plant instance")),
					PlantInstance
				);
				return;
			}

			TSharedPtr<FJsonObject> Obj;
			if (!FBackendJsonUtils::ParseObject(Response->GetContentAsString(), Obj) || !Obj.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid plant instance JSON"), PlantInstance);
				return;
			}

			double Num = 0.0;

			if (Obj->TryGetNumberField(TEXT("id"), Num))
			{
				PlantInstance.Id = static_cast<int32>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("gardenId"), Num))
			{
				PlantInstance.GardenId = static_cast<int32>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("speciesId"), Num))
			{
				PlantInstance.SpeciesId = static_cast<int32>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("soilId"), Num))
			{
				PlantInstance.SoilId = static_cast<int32>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("heightCm"), Num))
			{
				PlantInstance.HeightCm = static_cast<float>(Num);
				PlantInstance.bHasHeightCm = true;
			}

			if (Obj->TryGetNumberField(TEXT("ageDays"), Num))
			{
				PlantInstance.AgeDays = static_cast<int32>(Num);
				PlantInstance.bHasAgeDays = true;
			}

			if (Obj->TryGetStringField(TEXT("healthStatus"), PlantInstance.HealthStatus))
			{
				PlantInstance.bHasHealthStatus = !PlantInstance.HealthStatus.IsEmpty();
			}

			if (Obj->TryGetStringField(TEXT("lastWatered"), PlantInstance.LastWatered))
			{
				PlantInstance.bHasLastWatered = !PlantInstance.LastWatered.IsEmpty();
			}

			Obj->TryGetStringField(TEXT("notes"), PlantInstance.Notes);
			Obj->TryGetStringField(TEXT("creationTimestamp"), PlantInstance.CreationTimestamp);
			Obj->TryGetStringField(TEXT("lastUpdated"), PlantInstance.LastUpdated);

			Callback.ExecuteIfBound(true, TEXT("Plant instance created"), PlantInstance);
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), FBackendPlantInstanceDto{});
	}
}