// Fill out your copyright notice in the Description page of Project Settings.

#include "BackendApiSubsystem.h"
#include "BackendJsonUtils.h"
#include "OAuthGISubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "GenericPlatform/GenericPlatformHttp.h"

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

void UBackendApiSubsystem::SearchPerenualPlants(const FString& Query, const FBackendPlantSearchResponse& Callback)
{
	const FString TrimmedQuery = Query.TrimStartAndEnd();
	if (TrimmedQuery.IsEmpty())
	{
		const TArray<FBackendPlantSearchResultDto> EmptyPlants;
		Callback.ExecuteIfBound(false, TEXT("Search query is empty"), EmptyPlants);
		return;
	}

	const FString Route = FString::Printf(
		TEXT("/perenual/search?query=%s"),
		*FGenericPlatformHttp::UrlEncode(TrimmedQuery)
	);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("GET"));

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			TArray<FBackendPlantSearchResultDto> Plants;

			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"), Plants);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to search plants")),
					Plants
				);
				return;
			}

			TSharedPtr<FJsonObject> RootObject;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid search response"), Plants);
				return;
			}

			const TArray<TSharedPtr<FJsonValue>>* Data = nullptr;
			if (!RootObject->TryGetArrayField(TEXT("data"), Data) || !Data)
			{
				Callback.ExecuteIfBound(true, TEXT("OK"), Plants);
				return;
			}

			for (const TSharedPtr<FJsonValue>& ItemValue : *Data)
			{
				if (!ItemValue.IsValid() || ItemValue->Type != EJson::Object)
				{
					continue;
				}

				const TSharedPtr<FJsonObject> Item = ItemValue->AsObject();
				if (!Item.IsValid())
				{
					continue;
				}

				FBackendPlantSearchResultDto Plant;
				double NumberValue = 0.0;

				if (Item->TryGetNumberField(TEXT("id"), NumberValue))
				{
					Plant.PerenualId = static_cast<int32>(NumberValue);
				}

				Item->TryGetStringField(TEXT("common_name"), Plant.CommonName);

				const TArray<TSharedPtr<FJsonValue>>* ScientificNames = nullptr;
				if (Item->TryGetArrayField(TEXT("scientific_name"), ScientificNames) && ScientificNames && ScientificNames->Num() > 0)
				{
					const TSharedPtr<FJsonValue>& FirstName = (*ScientificNames)[0];
					if (FirstName.IsValid())
					{
						Plant.ScientificName = FirstName->AsString();
					}
				}
				else
				{
					Item->TryGetStringField(TEXT("scientific_name"), Plant.ScientificName);
				}

				const TSharedPtr<FJsonObject>* DefaultImage = nullptr;
				if (Item->TryGetObjectField(TEXT("default_image"), DefaultImage) && DefaultImage && DefaultImage->IsValid())
				{
					if (!(*DefaultImage)->TryGetStringField(TEXT("regular_url"), Plant.ImageUrl))
					{
						(*DefaultImage)->TryGetStringField(TEXT("original_url"), Plant.ImageUrl);
					}
				}

				if (Plant.CommonName.TrimStartAndEnd().IsEmpty()
					|| Plant.ScientificName.TrimStartAndEnd().IsEmpty()
					|| Plant.ImageUrl.TrimStartAndEnd().IsEmpty())
				{
					continue;
				}

				Plants.Add(Plant);
			}

			Callback.ExecuteIfBound(true, TEXT("OK"), Plants);
		}
	);

	if (!Req->ProcessRequest())
	{
		const TArray<FBackendPlantSearchResultDto> EmptyPlants;
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), EmptyPlants);
	}
}

void UBackendApiSubsystem::GetPerenualPlantDetails(int32 PerenualId, const FBackendPlantDetailsResponse& Callback)
{
	if (PerenualId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("PerenualId must be > 0"), FBackendPlantDto{});
		return;
	}

	const FString Route = FString::Printf(TEXT("/perenual/details/%d"), PerenualId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("GET"));

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			FBackendPlantDto Plant;

			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"), Plant);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to fetch plant details")),
					Plant
				);
				return;
			}

			if (!FBackendJsonUtils::ParsePerenualPlantDetails(Response->GetContentAsString(), Plant))
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid plant details JSON"), Plant);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("OK"), Plant);
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), FBackendPlantDto{});
	}
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

void UBackendApiSubsystem::GetSavedSpeciesForGarden(int32 GardenId, const FBackendPlantsResponse& Callback)
{
	if (GardenId <= 0)
	{
		const TArray<FBackendPlantDto> EmptyPlants;
		Callback.ExecuteIfBound(false, TEXT("GardenId must be > 0"), EmptyPlants);
		return;
	}

	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		const TArray<FBackendPlantDto> EmptyPlants;
		Callback.ExecuteIfBound(false, Error, EmptyPlants);
		return;
	}

	const FString Route = FString::Printf(TEXT("/species/saved?userId=%d&gardenId=%d"), UserId, GardenId);
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
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to fetch garden saved species")),
					Plants
				);
				return;
			}

			if (!FBackendJsonUtils::ParsePlantArray(Response->GetContentAsString(), Plants))
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

void UBackendApiSubsystem::SavePlant(int32 PerenualId, const FBackendOperationResponse& Callback)
{
	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		Callback.ExecuteIfBound(false, Error);
		return;
	}

	const FString Route = FString::Printf(TEXT("/species/save/%d?userId=%d"), PerenualId, UserId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("POST"), TEXT("{}"));

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
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to save plant"))
				);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("Plant saved"));
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"));
	}
}

void UBackendApiSubsystem::SavePlantToGarden(int32 PerenualId, int32 GardenId, const FBackendOperationResponse& Callback)
{
	if (GardenId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("GardenId must be > 0"));
		return;
	}

	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		Callback.ExecuteIfBound(false, Error);
		return;
	}

	const FString Route = FString::Printf(TEXT("/species/save/%d?userId=%d&gardenId=%d"), PerenualId, UserId, GardenId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("POST"), TEXT("{}"));

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
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to save plant to garden"))
				);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("Plant saved to garden"));
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"));
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

void UBackendApiSubsystem::DeleteSavedPlantFromGarden(int32 PerenualId, int32 GardenId, const FBackendOperationResponse& Callback)
{
	if (GardenId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("GardenId must be > 0"));
		return;
	}

	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		Callback.ExecuteIfBound(false, Error);
		return;
	}

	const FString Route = FString::Printf(TEXT("/species/save/%d?userId=%d&gardenId=%d"), PerenualId, UserId, GardenId);
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
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to delete garden saved plant"))
				);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("Garden saved plant deleted"));
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

void UBackendApiSubsystem::ResolveZipCodeLocation(const FString& ZipCode, const FBackendZipLocationResponse& Callback)
{
	const FString TrimmedZip = ZipCode.TrimStartAndEnd();
	if (TrimmedZip.IsEmpty())
	{
		Callback.ExecuteIfBound(false, TEXT("ZIP code is empty"), 0.0f, 0.0f);
		return;
	}

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetURL(FString::Printf(TEXT("https://api.zippopotam.us/us/%s"), *FGenericPlatformHttp::UrlEncode(TrimmedZip)));
	Req->SetVerb(TEXT("GET"));
	Req->SetHeader(TEXT("Accept"), TEXT("application/json"));

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response resolving ZIP code"), 0.0f, 0.0f);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(false, TEXT("Could not find coordinates for that ZIP code"), 0.0f, 0.0f);
				return;
			}

			TSharedPtr<FJsonObject> Obj;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (!FJsonSerializer::Deserialize(Reader, Obj) || !Obj.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid ZIP location JSON"), 0.0f, 0.0f);
				return;
			}

			const TArray<TSharedPtr<FJsonValue>>* Places = nullptr;
			if (!Obj->TryGetArrayField(TEXT("places"), Places) || !Places || Places->Num() == 0)
			{
				Callback.ExecuteIfBound(false, TEXT("No coordinates found for that ZIP code"), 0.0f, 0.0f);
				return;
			}

			const TSharedPtr<FJsonObject> PlaceObj = (*Places)[0]->AsObject();
			if (!PlaceObj.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid ZIP location place JSON"), 0.0f, 0.0f);
				return;
			}

			FString LatitudeText;
			FString LongitudeText;
			if (!PlaceObj->TryGetStringField(TEXT("latitude"), LatitudeText) || !PlaceObj->TryGetStringField(TEXT("longitude"), LongitudeText))
			{
				Callback.ExecuteIfBound(false, TEXT("ZIP location coordinates missing"), 0.0f, 0.0f);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("OK"), FCString::Atof(*LatitudeText), FCString::Atof(*LongitudeText));
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start ZIP location request"), 0.0f, 0.0f);
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

void UBackendApiSubsystem::GetGardensByUser(const FBackendGardenSummariesResponse& Callback)
{
	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		const TArray<FBackendGardenSummaryDto> EmptyGardens;
		Callback.ExecuteIfBound(false, Error, EmptyGardens);
		return;
	}

	const FString Route = FString::Printf(TEXT("/garden/by-user/%d"), UserId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("GET"));

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			TArray<FBackendGardenSummaryDto> Gardens;

			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"), Gardens);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to fetch gardens")),
					Gardens
				);
				return;
			}

			if (!FBackendJsonUtils::ParseGardenSummaryArray(Response->GetContentAsString(), Gardens))
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid gardens JSON"), Gardens);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("OK"), Gardens);
		}
	);

	if (!Req->ProcessRequest())
	{
		const TArray<FBackendGardenSummaryDto> EmptyGardens;
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), EmptyGardens);
	}
}

void UBackendApiSubsystem::GetGardenDetail(int32 GardenId, const FBackendGardenDetailResponse& Callback)
{
	if (GardenId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("GardenId must be > 0"), FBackendGardenDetailDto{});
		return;
	}

	int32 UserId = 0;
	FString Error;
	if (!TryGetUserId(UserId, Error))
	{
		Callback.ExecuteIfBound(false, Error, FBackendGardenDetailDto{});
		return;
	}

	const FString Route = FString::Printf(TEXT("/garden/%d?userId=%d"), GardenId, UserId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("GET"));

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			FBackendGardenDetailDto Garden;

			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"), Garden);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to fetch garden detail")),
					Garden
				);
				return;
			}

			if (!FBackendJsonUtils::ParseGardenDetail(Response->GetContentAsString(), Garden))
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid garden detail JSON"), Garden);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("OK"), Garden);
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), FBackendGardenDetailDto{});
	}
}

void UBackendApiSubsystem::CreateGarden(
	const FString& Name,
	const FString& Description,
	const FString& BloomDate,
	const FString& PaintMaskData,
	float Latitude,
	float Longitude,
	const FString& Timezone,
	const FBackendGardenResponse& Callback
)
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
	BodyObj->SetStringField(TEXT("paintMaskData"), PaintMaskData);
	BodyObj->SetNumberField(TEXT("latitude"), Latitude);
	BodyObj->SetNumberField(TEXT("longitude"), Longitude);

	if (!Timezone.IsEmpty())
	{
		BodyObj->SetStringField(TEXT("timezone"), Timezone);
	}

	if (!BloomDate.TrimStartAndEnd().IsEmpty())
	{
		BodyObj->SetStringField(TEXT("bloomDate"), BloomDate.TrimStartAndEnd());
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
			Obj->TryGetStringField(TEXT("bloomDate"), Garden.BloomDate);
			Obj->TryGetStringField(TEXT("paintMaskData"), Garden.PaintMaskData);

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

void UBackendApiSubsystem::UpdateGarden(
	int32 GardenId,
	const FString& Name,
	const FString& Description,
	const FString& BloomDate,
	const FString& PaintMaskData,
	float Latitude,
	float Longitude,
	const FString& Timezone,
	const FBackendGardenResponse& Callback
)
{
	if (GardenId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("GardenId must be > 0"), FBackendGardenDto{});
		return;
	}

	const FString TrimmedName = Name.TrimStartAndEnd();
	if (TrimmedName.IsEmpty())
	{
		Callback.ExecuteIfBound(false, TEXT("Garden name is empty"), FBackendGardenDto{});
		return;
	}

	TSharedRef<FJsonObject> BodyObj = MakeShared<FJsonObject>();
	BodyObj->SetStringField(TEXT("name"), TrimmedName);
	BodyObj->SetStringField(TEXT("description"), Description);
	BodyObj->SetStringField(TEXT("paintMaskData"), PaintMaskData);
	BodyObj->SetNumberField(TEXT("latitude"), Latitude);
	BodyObj->SetNumberField(TEXT("longitude"), Longitude);

	if (!Timezone.IsEmpty())
	{
		BodyObj->SetStringField(TEXT("timezone"), Timezone);
	}

	if (!BloomDate.TrimStartAndEnd().IsEmpty())
	{
		BodyObj->SetStringField(TEXT("bloomDate"), BloomDate.TrimStartAndEnd());
	}

	const FString BodyStr = FBackendJsonUtils::StringifyObject(BodyObj);
	const FString Route = FString::Printf(TEXT("/garden/%d"), GardenId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("PATCH"), BodyStr);

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
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to update garden")),
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
			Obj->TryGetStringField(TEXT("bloomDate"), Garden.BloomDate);
			Obj->TryGetStringField(TEXT("paintMaskData"), Garden.PaintMaskData);

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

			Callback.ExecuteIfBound(true, TEXT("Garden updated"), Garden);
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), FBackendGardenDto{});
	}
}

void UBackendApiSubsystem::DeleteGarden(int32 GardenId, const FBackendOperationResponse& Callback)
{
	if (GardenId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("GardenId must be > 0"));
		return;
	}

	const FString Route = FString::Printf(TEXT("/garden/%d"), GardenId);
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
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to delete garden"))
				);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("Garden deleted"));
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"));
	}
}

void UBackendApiSubsystem::EnsureGenericSoil(const FBackendSoilIdResponse& Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> GetReq = CreateRequest(TEXT("/soil"), TEXT("GET"));

	GetReq->OnProcessRequestComplete().BindLambda(
		[this, Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response while fetching soil"), 0);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to fetch soil")),
					0
				);
				return;
			}

			TSharedPtr<FJsonValue> RootValue;
			if (!FBackendJsonUtils::ParseValue(Response->GetContentAsString(), RootValue) || !RootValue.IsValid() || RootValue->Type != EJson::Array)
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid soil JSON"), 0);
				return;
			}

			const TArray<TSharedPtr<FJsonValue>>& Items = RootValue->AsArray();
			for (const TSharedPtr<FJsonValue>& ItemVal : Items)
			{
				if (!ItemVal.IsValid() || ItemVal->Type != EJson::Object)
				{
					continue;
				}

				const TSharedPtr<FJsonObject> Obj = ItemVal->AsObject();
				if (!Obj.IsValid())
				{
					continue;
				}

				double SoilIdNumber = 0.0;
				if (Obj->TryGetNumberField(TEXT("id"), SoilIdNumber))
				{
					const int32 SoilId = static_cast<int32>(SoilIdNumber);
					if (SoilId > 0)
					{
						Callback.ExecuteIfBound(true, TEXT("Using existing soil"), SoilId);
						return;
					}
				}
			}

			TSharedRef<FJsonObject> BodyObj = MakeShared<FJsonObject>();
			BodyObj->SetNumberField(TEXT("nitrogen"), 1.0);
			BodyObj->SetNumberField(TEXT("phosphorus"), 1.0);
			BodyObj->SetNumberField(TEXT("potassium"), 1.0);
			BodyObj->SetNumberField(TEXT("pH"), 6.5);
			BodyObj->SetNumberField(TEXT("organicPercentage"), 5.0);
			BodyObj->SetStringField(TEXT("type"), TEXT("LOAM"));

			const FString BodyStr = FBackendJsonUtils::StringifyObject(BodyObj);
			TSharedRef<IHttpRequest, ESPMode::ThreadSafe> PostReq = CreateRequest(TEXT("/soil"), TEXT("POST"), BodyStr);

			PostReq->OnProcessRequestComplete().BindLambda(
				[Callback](FHttpRequestPtr PostRequest, FHttpResponsePtr PostResponse, bool bPostSuccess)
				{
					if (!bPostSuccess || !PostResponse.IsValid())
					{
						Callback.ExecuteIfBound(false, TEXT("No response while creating soil"), 0);
						return;
					}

					if (!UBackendApiSubsystem::IsHttpSuccess(PostResponse))
					{
						Callback.ExecuteIfBound(
							false,
							UBackendApiSubsystem::BuildErrorMessage(PostResponse, TEXT("Failed to create generic soil")),
							0
						);
						return;
					}

					TSharedPtr<FJsonObject> Obj;
					if (!FBackendJsonUtils::ParseObject(PostResponse->GetContentAsString(), Obj) || !Obj.IsValid())
					{
						Callback.ExecuteIfBound(false, TEXT("Invalid generic soil JSON"), 0);
						return;
					}

					double SoilIdNumber = 0.0;
					if (Obj->TryGetNumberField(TEXT("id"), SoilIdNumber))
					{
						const int32 SoilId = static_cast<int32>(SoilIdNumber);
						if (SoilId > 0)
						{
							Callback.ExecuteIfBound(true, TEXT("Created generic soil"), SoilId);
							return;
						}
					}

					Callback.ExecuteIfBound(false, TEXT("Generic soil response missing id"), 0);
				}
			);

			if (!PostReq->ProcessRequest())
			{
				Callback.ExecuteIfBound(false, TEXT("Failed to start soil creation request"), 0);
			}
		}
	);

	if (!GetReq->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start soil fetch request"), 0);
	}
}

void UBackendApiSubsystem::CreatePlantInstance(int32 GardenId, int32 SpeciesId, const FString& SoilType, const FVector& Location, const FRotator& Rotation, const FVector& Scale, const float* HeightCm, const int32* AgeDays, const FString* HealthStatus, const FString* LastWateredIso8601, const FString* PlantedDateIso8601, const FString* BloomDateIso8601, const FString& Notes, const FBackendPlantInstanceResponse& Callback)
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

	const FString NormalizedSoilType = SoilType.TrimStartAndEnd().ToUpper();
	if (NormalizedSoilType.IsEmpty())
	{
		Callback.ExecuteIfBound(false, TEXT("SoilType is empty"), FBackendPlantInstanceDto{});
		return;
	}

	TSharedRef<FJsonObject> BodyObj = MakeShared<FJsonObject>();
	BodyObj->SetNumberField(TEXT("gardenId"), GardenId);
	BodyObj->SetNumberField(TEXT("speciesId"), SpeciesId);
	BodyObj->SetStringField(TEXT("soilType"), NormalizedSoilType);
	BodyObj->SetNumberField(TEXT("locationX"), Location.X);
	BodyObj->SetNumberField(TEXT("locationY"), Location.Y);
	BodyObj->SetNumberField(TEXT("locationZ"), Location.Z);
	BodyObj->SetNumberField(TEXT("rotationPitch"), Rotation.Pitch);
	BodyObj->SetNumberField(TEXT("rotationYaw"), Rotation.Yaw);
	BodyObj->SetNumberField(TEXT("rotationRoll"), Rotation.Roll);
	BodyObj->SetNumberField(TEXT("scaleX"), Scale.X);
	BodyObj->SetNumberField(TEXT("scaleY"), Scale.Y);
	BodyObj->SetNumberField(TEXT("scaleZ"), Scale.Z);

	if (HeightCm)
	{
		BodyObj->SetNumberField(TEXT("heightCm"), *HeightCm);
	}

	if (HealthStatus && !HealthStatus->IsEmpty())
	{
		BodyObj->SetStringField(TEXT("healthStatus"), *HealthStatus);
	}

	if (LastWateredIso8601 && !LastWateredIso8601->IsEmpty())
	{
		BodyObj->SetStringField(TEXT("lastWatered"), *LastWateredIso8601);
	}

	if (PlantedDateIso8601 && !PlantedDateIso8601->IsEmpty())
	{
		BodyObj->SetStringField(TEXT("plantedDate"), *PlantedDateIso8601);
	}

	if (BloomDateIso8601 && !BloomDateIso8601->IsEmpty())
	{
		BodyObj->SetStringField(TEXT("bloomDate"), *BloomDateIso8601);
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

			if (Obj->TryGetNumberField(TEXT("locationX"), Num))
			{
				PlantInstance.Location.X = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("locationY"), Num))
			{
				PlantInstance.Location.Y = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("locationZ"), Num))
			{
				PlantInstance.Location.Z = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("rotationPitch"), Num))
			{
				PlantInstance.Rotation.Pitch = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("rotationYaw"), Num))
			{
				PlantInstance.Rotation.Yaw = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("rotationRoll"), Num))
			{
				PlantInstance.Rotation.Roll = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("scaleX"), Num))
			{
				PlantInstance.Scale.X = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("scaleY"), Num))
			{
				PlantInstance.Scale.Y = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("scaleZ"), Num))
			{
				PlantInstance.Scale.Z = static_cast<float>(Num);
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

			if (Obj->TryGetStringField(TEXT("plantedDate"), PlantInstance.PlantedDate))
			{
				PlantInstance.bHasPlantedDate = !PlantInstance.PlantedDate.IsEmpty();
			}

			if (Obj->TryGetStringField(TEXT("bloomDate"), PlantInstance.BloomDate))
			{
				PlantInstance.bHasBloomDate = !PlantInstance.BloomDate.IsEmpty();
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

void UBackendApiSubsystem::UpdatePlantInstance(int32 PlantInstanceId, const FVector& Location, const FRotator& Rotation, const FVector& Scale, float HeightCm, int32 AgeDays, const FString& HealthStatus, const FString& LastWateredIso8601, const FString& PlantedDateIso8601, const FString& BloomDateIso8601, const FString& Notes, const FBackendPlantInstanceResponse& Callback)
{
	if (PlantInstanceId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("PlantInstanceId must be > 0"), FBackendPlantInstanceDto{});
		return;
	}

	TSharedRef<FJsonObject> BodyObj = MakeShared<FJsonObject>();
	BodyObj->SetNumberField(TEXT("locationX"), Location.X);
	BodyObj->SetNumberField(TEXT("locationY"), Location.Y);
	BodyObj->SetNumberField(TEXT("locationZ"), Location.Z);
	BodyObj->SetNumberField(TEXT("rotationPitch"), Rotation.Pitch);
	BodyObj->SetNumberField(TEXT("rotationYaw"), Rotation.Yaw);
	BodyObj->SetNumberField(TEXT("rotationRoll"), Rotation.Roll);
	BodyObj->SetNumberField(TEXT("scaleX"), Scale.X);
	BodyObj->SetNumberField(TEXT("scaleY"), Scale.Y);
	BodyObj->SetNumberField(TEXT("scaleZ"), Scale.Z);
	BodyObj->SetNumberField(TEXT("heightCm"), HeightCm);
	BodyObj->SetStringField(TEXT("healthStatus"), HealthStatus);
	BodyObj->SetStringField(TEXT("lastWatered"), LastWateredIso8601);

	if (!PlantedDateIso8601.IsEmpty())
	{
		BodyObj->SetStringField(TEXT("plantedDate"), PlantedDateIso8601);
	}

	if (!BloomDateIso8601.IsEmpty())
	{
		BodyObj->SetStringField(TEXT("bloomDate"), BloomDateIso8601);
	}

	if (!Notes.IsEmpty())
	{
		BodyObj->SetStringField(TEXT("notes"), Notes);
	}

	const FString BodyStr = FBackendJsonUtils::StringifyObject(BodyObj);
	const FString Route = FString::Printf(TEXT("/plant-instance/%d"), PlantInstanceId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("PATCH"), BodyStr);

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
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to update plant instance")),
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

			if (Obj->TryGetNumberField(TEXT("locationX"), Num))
			{
				PlantInstance.Location.X = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("locationY"), Num))
			{
				PlantInstance.Location.Y = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("locationZ"), Num))
			{
				PlantInstance.Location.Z = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("rotationPitch"), Num))
			{
				PlantInstance.Rotation.Pitch = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("rotationYaw"), Num))
			{
				PlantInstance.Rotation.Yaw = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("rotationRoll"), Num))
			{
				PlantInstance.Rotation.Roll = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("scaleX"), Num))
			{
				PlantInstance.Scale.X = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("scaleY"), Num))
			{
				PlantInstance.Scale.Y = static_cast<float>(Num);
			}

			if (Obj->TryGetNumberField(TEXT("scaleZ"), Num))
			{
				PlantInstance.Scale.Z = static_cast<float>(Num);
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

			if (Obj->TryGetStringField(TEXT("plantedDate"), PlantInstance.PlantedDate))
			{
				PlantInstance.bHasPlantedDate = !PlantInstance.PlantedDate.IsEmpty();
			}

			if (Obj->TryGetStringField(TEXT("bloomDate"), PlantInstance.BloomDate))
			{
				PlantInstance.bHasBloomDate = !PlantInstance.BloomDate.IsEmpty();
			}

			Obj->TryGetStringField(TEXT("notes"), PlantInstance.Notes);
			Obj->TryGetStringField(TEXT("creationTimestamp"), PlantInstance.CreationTimestamp);
			Obj->TryGetStringField(TEXT("lastUpdated"), PlantInstance.LastUpdated);

			Callback.ExecuteIfBound(true, TEXT("Plant instance updated"), PlantInstance);
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), FBackendPlantInstanceDto{});
	}
}

void UBackendApiSubsystem::DeletePlantInstance(int32 PlantInstanceId, const FBackendOperationResponse& Callback)
{
	if (PlantInstanceId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("PlantInstanceId must be > 0"));
		return;
	}

	const FString Route = FString::Printf(TEXT("/plant-instance/%d"), PlantInstanceId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("DELETE"));

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (!bWasSuccessful)
			{
				Callback.ExecuteIfBound(false, TEXT("Request failed"));
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to delete plant instance"))
				);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("Plant instance deleted"));
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"));
	}
}

void UBackendApiSubsystem::GenerateGardenTimeline(int32 GardenId, const FString& BloomDate, const FBackendGardenTimelineResponse& Callback)
{
	if (GardenId <= 0)
	{
		Callback.ExecuteIfBound(false, TEXT("GardenId must be > 0"), FBackendGardenTimelineDto{});
		return;
	}

	const FString NormalizedBloomDate = BloomDate.TrimStartAndEnd();
	if (NormalizedBloomDate.IsEmpty())
	{
		Callback.ExecuteIfBound(false, TEXT("Bloom date is empty"), FBackendGardenTimelineDto{});
		return;
	}

	TSharedRef<FJsonObject> BodyObj = MakeShared<FJsonObject>();
	BodyObj->SetStringField(TEXT("bloomDate"), NormalizedBloomDate);

	const FString BodyStr = FBackendJsonUtils::StringifyObject(BodyObj);
	const FString Route = FString::Printf(TEXT("/prediction/garden/%d/timeline"), GardenId);
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = CreateRequest(Route, TEXT("POST"), BodyStr);

	Req->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
		{
			FBackendGardenTimelineDto Timeline;

			if (!bSuccess || !Response.IsValid())
			{
				Callback.ExecuteIfBound(false, TEXT("No response"), Timeline);
				return;
			}

			if (!UBackendApiSubsystem::IsHttpSuccess(Response))
			{
				Callback.ExecuteIfBound(
					false,
					UBackendApiSubsystem::BuildErrorMessage(Response, TEXT("Failed to generate garden timeline")),
					Timeline
				);
				return;
			}

			const FString Body = Response->GetContentAsString();
			UE_LOG(LogTemp, Warning, TEXT("GenerateGardenTimeline response from backend/model to Unreal (HTTP %d): %s"), Response->GetResponseCode(), *Body);

			if (!FBackendJsonUtils::ParseGardenTimeline(Body, Timeline))
			{
				Callback.ExecuteIfBound(false, TEXT("Invalid garden timeline JSON"), Timeline);
				return;
			}

			Callback.ExecuteIfBound(true, TEXT("Garden timeline generated"), Timeline);
		}
	);

	if (!Req->ProcessRequest())
	{
		Callback.ExecuteIfBound(false, TEXT("Failed to start request"), FBackendGardenTimelineDto{});
	}
}
