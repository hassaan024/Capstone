// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"
#include "BackendApiSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "Components/SlateWrapperTypes.h"
#include "GameFramework/PlayerController.h"

bool UMainMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	MenuController = Cast<AMenuController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!MenuController)
	{
		return false;
	}

	if (BTN_UpdateDisplayName)
	{
		BTN_UpdateDisplayName->OnClicked.AddDynamic(this, &UMainMenu::OnPressUpdateDisplayName);
	}

	if (BTN_SavedPlants)
	{
		BTN_SavedPlants->OnClicked.AddDynamic(this, &UMainMenu::OnPressSavedPlants);
	}

	if (BTN_CreateGarden)
	{
		BTN_CreateGarden->OnClicked.AddDynamic(this, &UMainMenu::OnPressCreateGarden);
	}

	if (BTN_LoadGarden)
	{
		BTN_LoadGarden->OnClicked.AddDynamic(this, &UMainMenu::OnPressLoadGarden);
	}

	if (TXT_CurrentTemp)
	{
		TXT_CurrentTemp->SetText(FText::FromString(TEXT("--")));
	}

	if (TXT_WeatherDesc)
	{
		TXT_WeatherDesc->SetText(FText::FromString(TEXT("Loading weather...")));
	}

	RequestWeatherFromStoredLocation();

	return true;
}

void UMainMenu::OnPressUpdateDisplayName()
{
	MenuController->ShowDisplayName();
}

void UMainMenu::OnPressSavedPlants()
{
	MenuController->ShowSavedPlants();
}

void UMainMenu::OnPressCreateGarden()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName("Garden"));
}

void UMainMenu::OnPressLoadGarden()
{

}

void UMainMenu::RequestWeatherFromStoredLocation()
{
	if (!GetGameInstance())
	{
		if (TXT_WeatherDesc)
		{
			TXT_WeatherDesc->SetText(FText::FromString(TEXT("No GameInstance")));
		}
		return;
	}

	UBackendApiSubsystem* Backend = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Backend)
	{
		if (TXT_WeatherDesc)
		{
			TXT_WeatherDesc->SetText(FText::FromString(TEXT("Backend unavailable")));
		}
		return;
	}

	Backend->GetUserLocation(FBackendUserLocationResponse::CreateUObject(this, &UMainMenu::HandleUserLocationResponse));
}

void UMainMenu::HandleUserLocationResponse(bool bSuccess, const FString& Message, const FBackendUserLocationDto& Location)
{
	if (!bSuccess)
	{
		if (TXT_CurrentTemp)
		{
			TXT_CurrentTemp->SetText(FText::FromString(TEXT("")));
		}

		if (TXT_WeatherDesc)
		{
			TXT_WeatherDesc->SetText(FText::FromString(TEXT("")));
		}

		return;
	}

	UBackendApiSubsystem* Backend = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Backend)
	{
		if (TXT_WeatherDesc)
		{
			TXT_WeatherDesc->SetText(FText::FromString(TEXT("Backend unavailable")));
		}
		return;
	}

	Backend->GetCurrentWeather(
		static_cast<float>(Location.Latitude),
		static_cast<float>(Location.Longitude),
		FBackendWeatherResponse::CreateUObject(this, &UMainMenu::HandleWeatherResponse)
	);
}

void UMainMenu::HandleWeatherResponse(bool bSuccess, const FString& Message, const FBackendWeatherDto& Weather)
{
	if (!bSuccess)
	{
		if (TXT_CurrentTemp)
		{
			TXT_CurrentTemp->SetText(FText::FromString(TEXT("")));
		}

		if (TXT_WeatherDesc)
		{
			TXT_WeatherDesc->SetText(FText::FromString(TEXT("")));
		}

		UpdateWeatherIcon(TEXT(""));
		return;
	}

	if (TXT_CurrentTemp)
	{
		if (Weather.bHasTemperature2m)
		{
			const FString TempString = FString::Printf(TEXT("%.0f°"), Weather.Temperature2m);
			TXT_CurrentTemp->SetText(FText::FromString(TempString));
		}
		else
		{
			TXT_CurrentTemp->SetText(FText::FromString(TEXT("--")));
		}
	}

	if (TXT_WeatherDesc)
	{
		if (!Weather.Description.IsEmpty())
		{
			TXT_WeatherDesc->SetText(FText::FromString(Weather.Description));
		}
		else
		{
			TXT_WeatherDesc->SetText(FText::FromString(TEXT("No description")));
		}
	}

	UpdateWeatherIcon(Weather.Description);
}

UTexture2D* UMainMenu::GetWeatherIconForDescription(const FString& Description) const
{
	const FString Desc = Description.ToLower();

	if (Desc.Contains(TEXT("clear")))
	{
		return WiDaySunny;
	}

	if (Desc.Contains(TEXT("partly cloudy")))
	{
		return WiCloudy;
	}

	if (Desc.Contains(TEXT("overcast")))
	{
		return WiCloud;
	}

	if (Desc.Contains(TEXT("fog")))
	{
		return WiFog;
	}

	if (Desc.Contains(TEXT("rain")) || Desc.Contains(TEXT("drizzle")))
	{
		return WiRain;
	}

	//if (Desc.Contains(TEXT("snow")))
	//{
	//	return WiSnow;
	//}

	return IconDefault;
}

void UMainMenu::UpdateWeatherIcon(const FString& Description)
{
	if (!IMG_WeatherIcon)
	{
		return;
	}

	if (UTexture2D* ChosenIcon = GetWeatherIconForDescription(Description))
	{
		IMG_WeatherIcon->SetBrushFromTexture(ChosenIcon);
		IMG_WeatherIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		IMG_WeatherIcon->SetBrushFromTexture(nullptr);
	}
}