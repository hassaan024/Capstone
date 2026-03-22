// Fill out your copyright notice in the Description page of Project Settings.

#include "BackendJsonUtils.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

bool FBackendJsonUtils::ParseObject(const FString& JsonString, TSharedPtr<FJsonObject>& OutObject)
{
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
}

bool FBackendJsonUtils::ParseValue(const FString& JsonString, TSharedPtr<FJsonValue>& OutValue)
{
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	return FJsonSerializer::Deserialize(Reader, OutValue) && OutValue.IsValid();
}

FString FBackendJsonUtils::StringifyObject(const TSharedRef<FJsonObject>& JsonObject)
{
	FString OutString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutString);
	FJsonSerializer::Serialize(JsonObject, Writer);
	return OutString;
}

bool FBackendJsonUtils::TryGetErrorMessage(const FString& JsonString, FString& OutErrorMessage)
{
	OutErrorMessage = JsonString;

	TSharedPtr<FJsonObject> Obj;
	if (!ParseObject(JsonString, Obj) || !Obj.IsValid())
	{
		return false;
	}

	if (Obj->TryGetStringField(TEXT("message"), OutErrorMessage))
	{
		return true;
	}

	if (Obj->TryGetStringField(TEXT("error"), OutErrorMessage))
	{
		return true;
	}

	return false;
}

bool FBackendJsonUtils::ParsePlantArray(const FString& JsonString, TArray<FBackendPlantDto>& OutPlants)
{
	OutPlants.Reset();

	TSharedPtr<FJsonValue> RootValue;
	if (!ParseValue(JsonString, RootValue) || !RootValue.IsValid() || RootValue->Type != EJson::Array)
	{
		return false;
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

		FBackendPlantDto Plant;
		Obj->TryGetStringField(TEXT("commonName"), Plant.CommonName);
		Obj->TryGetStringField(TEXT("scientificName"), Plant.ScientificName);
		Obj->TryGetStringField(TEXT("imgSrcUrl"), Plant.ImgSrcUrl);
		Obj->TryGetNumberField(TEXT("trefleId"), Plant.TrefleId);

		OutPlants.Add(MoveTemp(Plant));
	}

	return true;
}

bool FBackendJsonUtils::ParseCurrentUser(const FString& JsonString, FBackendUserDto& OutUser)
{
	OutUser = FBackendUserDto{};

	TSharedPtr<FJsonObject> Obj;
	if (!ParseObject(JsonString, Obj) || !Obj.IsValid())
	{
		return false;
	}

	double IdNumber = 0.0;
	if (Obj->TryGetNumberField(TEXT("id"), IdNumber))
	{
		OutUser.Id = static_cast<int32>(IdNumber);
	}
	else
	{
		FString IdString;
		if (Obj->TryGetStringField(TEXT("id"), IdString))
		{
			OutUser.Id = FCString::Atoi(*IdString);
		}
	}

	Obj->TryGetStringField(TEXT("displayName"), OutUser.DisplayName);
	Obj->TryGetStringField(TEXT("googleDisplayName"), OutUser.GoogleDisplayName);
	Obj->TryGetBoolField(TEXT("confirmedName"), OutUser.bConfirmedName);

	return true;
}