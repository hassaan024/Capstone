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

		double NumberValue = 0.0;

		if (Obj->TryGetNumberField(TEXT("id"), NumberValue))
		{
			Plant.Id = static_cast<int32>(NumberValue);
		}

		if (Obj->TryGetNumberField(TEXT("perenualId"), NumberValue))
		{
			Plant.PerenualId = static_cast<int32>(NumberValue);
		}

		Obj->TryGetStringField(TEXT("commonName"), Plant.CommonName);
		Obj->TryGetStringField(TEXT("scientificName"), Plant.ScientificName);

		if (Obj->TryGetNumberField(TEXT("growthRate"), NumberValue))
		{
			Plant.GrowthRate = static_cast<int32>(NumberValue);
		}

		Obj->TryGetStringField(TEXT("cycle"), Plant.Cycle);
		Obj->TryGetStringField(TEXT("type"), Plant.Type);
		Obj->TryGetStringField(TEXT("maintenance"), Plant.Maintenance);
		Obj->TryGetStringField(TEXT("careLevel"), Plant.CareLevel);

		if (Obj->TryGetNumberField(TEXT("avgHoursSun"), NumberValue))
		{
			Plant.AvgHoursSun = static_cast<int32>(NumberValue);
		}

		if (Obj->TryGetNumberField(TEXT("minTemp"), NumberValue))
		{
			Plant.MinTemp = static_cast<float>(NumberValue);
		}

		if (Obj->TryGetNumberField(TEXT("maxTemp"), NumberValue))
		{
			Plant.MaxTemp = static_cast<float>(NumberValue);
		}

		if (Obj->TryGetNumberField(TEXT("minHeight"), NumberValue))
		{
			Plant.MinHeight = static_cast<float>(NumberValue);
		}

		if (Obj->TryGetNumberField(TEXT("maxHeight"), NumberValue))
		{
			Plant.MaxHeight = static_cast<float>(NumberValue);
		}

		Obj->TryGetStringField(TEXT("wateringFreq"), Plant.WateringFreq);

		if (Obj->TryGetNumberField(TEXT("wateringMinDays"), NumberValue))
		{
			Plant.WateringMinDays = static_cast<int32>(NumberValue);
		}

		if (Obj->TryGetNumberField(TEXT("wateringMaxDays"), NumberValue))
		{
			Plant.WateringMaxDays = static_cast<int32>(NumberValue);
		}

		Obj->TryGetStringField(TEXT("family"), Plant.Family);
		Obj->TryGetStringField(TEXT("genus"), Plant.Genus);
		Obj->TryGetStringField(TEXT("speciesEpithet"), Plant.SpeciesEpithet);

		Obj->TryGetBoolField(TEXT("flowers"), Plant.bFlowers);
		Obj->TryGetStringField(TEXT("floweringSeason"), Plant.FloweringSeason);
		Obj->TryGetBoolField(TEXT("fruits"), Plant.bFruits);
		Obj->TryGetBoolField(TEXT("edibleFruit"), Plant.bEdibleFruit);
		Obj->TryGetStringField(TEXT("harvestSeason"), Plant.HarvestSeason);
		Obj->TryGetBoolField(TEXT("leaf"), Plant.bLeaf);
		Obj->TryGetBoolField(TEXT("edibleLeaf"), Plant.bEdibleLeaf);
		Obj->TryGetBoolField(TEXT("cuisine"), Plant.bCuisine);
		Obj->TryGetBoolField(TEXT("medicinal"), Plant.bMedicinal);
		Obj->TryGetBoolField(TEXT("droughtTolerant"), Plant.bDroughtTolerant);
		Obj->TryGetBoolField(TEXT("saltTolerant"), Plant.bSaltTolerant);
		Obj->TryGetBoolField(TEXT("tropical"), Plant.bTropical);
		Obj->TryGetBoolField(TEXT("indoor"), Plant.bIndoor);
		Obj->TryGetBoolField(TEXT("thorny"), Plant.bThorny);
		Obj->TryGetBoolField(TEXT("invasive"), Plant.bInvasive);

		Obj->TryGetStringField(TEXT("pruningFrequency"), Plant.PruningFrequency);
		Obj->TryGetStringField(TEXT("pruningInterval"), Plant.PruningInterval);

		const TArray<TSharedPtr<FJsonValue>>* StringArray = nullptr;

		if (Obj->TryGetArrayField(TEXT("otherNames"), StringArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *StringArray)
			{
				Plant.OtherNames.Add(Value->AsString());
			}
		}

		if (Obj->TryGetArrayField(TEXT("origin"), StringArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *StringArray)
			{
				Plant.Origin.Add(Value->AsString());
			}
		}

		if (Obj->TryGetArrayField(TEXT("pruningMonths"), StringArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *StringArray)
			{
				Plant.PruningMonths.Add(Value->AsString());
			}
		}

		const TArray<TSharedPtr<FJsonValue>>* AnatomyArray = nullptr;
		if (Obj->TryGetArrayField(TEXT("plantAnatomy"), AnatomyArray))
		{
			for (const TSharedPtr<FJsonValue>& Value : *AnatomyArray)
			{
				if (!Value.IsValid() || Value->Type != EJson::Object)
				{
					continue;
				}

				const TSharedPtr<FJsonObject> AnatomyObj = Value->AsObject();
				if (!AnatomyObj.IsValid())
				{
					continue;
				}

				FBackendPlantAnatomyDto Anatomy;
				AnatomyObj->TryGetStringField(TEXT("part"), Anatomy.Part);
				AnatomyObj->TryGetStringField(TEXT("description"), Anatomy.Description);
				Plant.PlantAnatomy.Add(MoveTemp(Anatomy));
			}
		}

		const TSharedPtr<FJsonObject>* ImgObj = nullptr;
		if (Obj->TryGetObjectField(TEXT("imgSrcUrls"), ImgObj) && ImgObj && ImgObj->IsValid())
		{
			(*ImgObj)->TryGetStringField(TEXT("regular"), Plant.ImgSrcUrls.Regular);
		}

		Obj->TryGetStringField(TEXT("modelCategory"), Plant.ModelCategory);

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

bool FBackendJsonUtils::ParseUserLocation(const FString& JsonString, FBackendUserLocationDto& OutLocation)
{
	OutLocation = FBackendUserLocationDto{};

	TSharedPtr<FJsonObject> Obj;
	if (!ParseObject(JsonString, Obj) || !Obj.IsValid())
	{
		return false;
	}

	double NumberValue = 0.0;

	if (Obj->TryGetNumberField(TEXT("latitude"), NumberValue))
	{
		OutLocation.Latitude = NumberValue;
		OutLocation.bHasLatitude = true;
	}

	if (Obj->TryGetNumberField(TEXT("longitude"), NumberValue))
	{
		OutLocation.Longitude = NumberValue;
		OutLocation.bHasLongitude = true;
	}

	Obj->TryGetStringField(TEXT("updatedAt"), OutLocation.UpdatedAt);

	return true;
}

bool FBackendJsonUtils::ParseWeather(const FString& JsonString, FBackendWeatherDto& OutWeather)
{
	OutWeather = FBackendWeatherDto{};

	TSharedPtr<FJsonObject> Obj;
	if (!ParseObject(JsonString, Obj) || !Obj.IsValid())
	{
		return false;
	}

	double NumberValue = 0.0;

	if (Obj->TryGetNumberField(TEXT("temperature_2m"), NumberValue))
	{
		OutWeather.Temperature2m = static_cast<float>(NumberValue);
		OutWeather.bHasTemperature2m = true;
	}

	Obj->TryGetStringField(TEXT("description"), OutWeather.Description);

	return true;
}