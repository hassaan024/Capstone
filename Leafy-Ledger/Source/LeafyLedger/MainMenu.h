// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "MenuController.h"
#include "BackendApiTypes.h"
#include "MainMenu.generated.h"

class UBackendApiSubsystem;

UCLASS()
class LEAFYLEDGER_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

protected:
	UFUNCTION()
	void OnPressUpdateDisplayName();

	UFUNCTION()
	void OnPressSavedPlants();

	void RequestWeatherFromStoredLocation();
	void HandleUserLocationResponse(bool bSuccess, const FString& Message, const FBackendUserLocationDto& Location);
	void HandleWeatherResponse(bool bSuccess, const FString& Message, const FBackendWeatherDto& Weather);

	void UpdateWeatherIcon(const FString& Description);
	UTexture2D* GetWeatherIconForDescription(const FString& Description) const;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_UpdateDisplayName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_SavedPlants;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TXT_CurrentTemp;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TXT_WeatherDesc;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* IMG_WeatherIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather Icons")
	UTexture2D* WiDaySunny;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather Icons")
	UTexture2D* WiCloudy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather Icons")
	UTexture2D* WiCloud;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather Icons")
	UTexture2D* WiFog;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather Icons")
	UTexture2D* WiRain;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather Icons")
	//UTexture2D* IconSnow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather Icons")
	UTexture2D* IconDefault;

	UPROPERTY()
	AMenuController* MenuController;
};