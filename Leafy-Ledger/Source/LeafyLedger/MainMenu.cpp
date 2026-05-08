// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"
#include "BackendApiSubsystem.h"
#include "GardenSessionSubsystem.h"
#include "Blueprint/WidgetTree.h"
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

	if (TXT_CurrentTemp)
	{
		TXT_CurrentTemp->SetText(FText::FromString(TEXT("--")));
	}

	if (TXT_WeatherDesc)
	{
		TXT_WeatherDesc->SetText(FText::FromString(TEXT("Loading weather...")));
	}

	if (IMG_WeatherIcon)
	{
		IMG_WeatherIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	EnsureWeatherIconDefaults();
	RequestCurrentUser();
	RequestWeatherFromStoredLocation();

	return true;
}

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();
	BindMenuButtons();
}

void UMainMenu::BindMenuButtons()
{
	if (UButton* CreateGardenButton = ResolveMenuButton(BTN_CreateGarden))
	{
		CreateGardenButton->OnClicked.RemoveDynamic(this, &UMainMenu::OnPressCreateGarden);
		CreateGardenButton->OnClicked.AddDynamic(this, &UMainMenu::OnPressCreateGarden);
		UE_LOG(LogTemp, Log, TEXT("MainMenu: bound BTN_CreateGarden to inner button %s"), *CreateGardenButton->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: failed to resolve BTN_CreateGarden as a clickable menu button"));
	}

	if (UButton* LoadGardenButton = ResolveMenuButton(BTN_LoadGarden))
	{
		LoadGardenButton->OnClicked.RemoveDynamic(this, &UMainMenu::OnPressLoadGarden);
		LoadGardenButton->OnClicked.AddDynamic(this, &UMainMenu::OnPressLoadGarden);
		UE_LOG(LogTemp, Log, TEXT("MainMenu: bound BTN_LoadGarden to inner button %s"), *LoadGardenButton->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: failed to resolve BTN_LoadGarden as a clickable menu button"));
	}

	if (UButton* DeleteGardenButton = ResolveMenuButton(BTN_DeleteGarden))
	{
		DeleteGardenButton->OnClicked.RemoveDynamic(this, &UMainMenu::OnPressDeleteGarden);
		DeleteGardenButton->OnClicked.AddDynamic(this, &UMainMenu::OnPressDeleteGarden);
		UE_LOG(LogTemp, Log, TEXT("MainMenu: bound BTN_DeleteGarden to inner button %s"), *DeleteGardenButton->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: failed to resolve BTN_DeleteGarden as a clickable menu button"));
	}

	if (UButton* UpdateDisplayNameButton = ResolveMenuButton(BTN_UpdateDisplayName))
	{
		UpdateDisplayNameButton->OnClicked.RemoveDynamic(this, &UMainMenu::OnPressUpdateDisplayName);
		UpdateDisplayNameButton->OnClicked.AddDynamic(this, &UMainMenu::OnPressUpdateDisplayName);
		UE_LOG(LogTemp, Log, TEXT("MainMenu: bound BTN_UpdateDisplayName to inner button %s"), *UpdateDisplayNameButton->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: failed to resolve BTN_UpdateDisplayName as a clickable menu button"));
	}

	if (UButton* SavedPlantsButton = ResolveMenuButton(BTN_SavedPlants))
	{
		SavedPlantsButton->OnClicked.RemoveDynamic(this, &UMainMenu::OnPressSavedPlants);
		SavedPlantsButton->OnClicked.AddDynamic(this, &UMainMenu::OnPressSavedPlants);
		UE_LOG(LogTemp, Log, TEXT("MainMenu: bound BTN_SavedPlants to inner button %s"), *SavedPlantsButton->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: failed to resolve BTN_SavedPlants as a clickable menu button"));
	}

	if (UButton* BrowseSpeciesButton = ResolveMenuButton(BTN_BrowseSpecies))
	{
		BrowseSpeciesButton->OnClicked.RemoveDynamic(this, &UMainMenu::OnPressBrowseSpecies);
		BrowseSpeciesButton->OnClicked.AddDynamic(this, &UMainMenu::OnPressBrowseSpecies);
		UE_LOG(LogTemp, Log, TEXT("MainMenu: bound BTN_BrowseSpecies to inner button %s"), *BrowseSpeciesButton->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: failed to resolve BTN_BrowseSpecies as a clickable menu button"));
	}
}

UButton* UMainMenu::ResolveMenuButton(UWidget* MenuButtonWidget) const
{
	if (!MenuButtonWidget)
	{
		return nullptr;
	}

	if (UButton* DirectButton = Cast<UButton>(MenuButtonWidget))
	{
		return DirectButton;
	}

	UUserWidget* MenuButtonUserWidget = Cast<UUserWidget>(MenuButtonWidget);
	if (!MenuButtonUserWidget || !MenuButtonUserWidget->WidgetTree)
	{
		return nullptr;
	}

	UButton* ResolvedButton = nullptr;
	MenuButtonUserWidget->WidgetTree->ForEachWidget([&ResolvedButton](UWidget* Widget)
	{
		if (!ResolvedButton)
		{
			ResolvedButton = Cast<UButton>(Widget);
		}
	});

	if (!ResolvedButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: no inner UButton found in %s"), *MenuButtonWidget->GetName());
	}

	return ResolvedButton;
}

void UMainMenu::OnPressUpdateDisplayName()
{
	MenuController->ShowDisplayName();
}

void UMainMenu::OnPressSavedPlants()
{
	MenuController->ShowSavedPlants();
}

void UMainMenu::OnPressBrowseSpecies()
{
	MenuController->ShowBrowseSpecies();
}

void UMainMenu::OnPressCreateGarden()
{
	if (!CreateGardenPopupClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGardenPopupClass is null"));
		return;
	}

	CreateGardenPopupInstance = CreateWidget<UCreateGardenPopup>(GetOwningPlayer(), CreateGardenPopupClass);
	if (!CreateGardenPopupInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create popup"));
		return;
	}

	CreateGardenPopupInstance->AddToViewport();
}

void UMainMenu::OnPressLoadGarden()
{
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressLoadGarden: GameInstance is null"));
		return;
	}

	UBackendApiSubsystem* Backend = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Backend)
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressLoadGarden: BackendApiSubsystem missing"));
		return;
	}

	if (!LoadGardenPopupClass)
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressLoadGarden: LoadGardenPopupClass is null"));
		return;
	}

	Backend->GetGardensByUser(
		FBackendGardenSummariesResponse::CreateWeakLambda(
			this,
			[this](bool bSuccess, const FString& Message, const TArray<FBackendGardenSummaryDto>& Gardens)
			{
				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("GetGardensByUser failed: %s"), *Message);
					return;
				}

				if (Gardens.Num() == 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("No gardens available to load"));
					return;
				}

				if (LoadGardenPopupInstance)
				{
					LoadGardenPopupInstance->RemoveFromParent();
					LoadGardenPopupInstance = nullptr;
				}

				LoadGardenPopupInstance = CreateWidget<ULoadGardenPopup>(GetOwningPlayer(), LoadGardenPopupClass);
				if (!LoadGardenPopupInstance)
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to create load garden popup"));
					return;
				}

				LoadGardenPopupInstance->SetGardens(Gardens);
				LoadGardenPopupInstance->OnGardenChosen = FOnGardenChosen::CreateUObject(this, &UMainMenu::HandleLoadGardenSelected);
				LoadGardenPopupInstance->AddToViewport(20);
			}
		)
	);
}

void UMainMenu::OnPressDeleteGarden()
{
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressDeleteGarden: GameInstance is null"));
		return;
	}

	UBackendApiSubsystem* Backend = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Backend)
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressDeleteGarden: BackendApiSubsystem missing"));
		return;
	}

	if (!DeleteGardenPopupClass)
	{
		UE_LOG(LogTemp, Error, TEXT("OnPressDeleteGarden: DeleteGardenPopupClass is null"));
		return;
	}

	Backend->GetGardensByUser(
		FBackendGardenSummariesResponse::CreateWeakLambda(
			this,
			[this](bool bSuccess, const FString& Message, const TArray<FBackendGardenSummaryDto>& Gardens)
			{
				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("GetGardensByUser failed: %s"), *Message);
					return;
				}

				if (Gardens.Num() == 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("No gardens available to delete"));
					return;
				}

				if (DeleteGardenPopupInstance)
				{
					DeleteGardenPopupInstance->RemoveFromParent();
					DeleteGardenPopupInstance = nullptr;
				}

				DeleteGardenPopupInstance = CreateWidget<UDeleteGardenPopup>(GetOwningPlayer(), DeleteGardenPopupClass);
				if (!DeleteGardenPopupInstance)
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to create delete garden popup"));
					return;
				}

				DeleteGardenPopupInstance->SetGardens(Gardens);
				DeleteGardenPopupInstance->AddToViewport(20);
			}
		)
	);
}

void UMainMenu::RequestCurrentUser()
{
	if (TXT_Welcome)
	{
		TXT_Welcome->SetText(FText::FromString(TEXT("Welcome back!")));
	}

	if (!GetGameInstance())
	{
		return;
	}

	UBackendApiSubsystem* Backend = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Backend)
	{
		return;
	}

	Backend->GetCurrentUser(FBackendCurrentUserResponse::CreateUObject(this, &UMainMenu::HandleCurrentUserResponse));
}

void UMainMenu::HandleCurrentUserResponse(bool bSuccess, const FString& Message, const FBackendUserDto& User)
{
	if (!TXT_Welcome)
	{
		return;
	}

	FString Name;
	if (bSuccess)
	{
		Name = User.DisplayName.TrimStartAndEnd();
		if (Name.IsEmpty())
		{
			Name = User.GoogleDisplayName.TrimStartAndEnd();
		}
	}

	if (Name.IsEmpty())
	{
		Name = TEXT("gardener");
	}

	TXT_Welcome->SetText(FText::FromString(FString::Printf(TEXT("Welcome back, %s!"), *Name)));
}

void UMainMenu::HandleLoadGardenSelected(int32 GardenId)
{
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("HandleLoadGardenSelected: GameInstance is null"));
		return;
	}

	UBackendApiSubsystem* Backend = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();

	if (!Backend || !GardenSession)
	{
		UE_LOG(LogTemp, Error, TEXT("HandleLoadGardenSelected: missing subsystem(s)"));
		return;
	}

	Backend->GetGardenDetail(
		GardenId,
		FBackendGardenDetailResponse::CreateWeakLambda(
			this,
			[this, GardenSession](bool bSuccess, const FString& Message, const FBackendGardenDetailDto& Garden)
			{
				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("GetGardenDetail failed: %s"), *Message);
					return;
				}

				GardenSession->LoadGardenDraft(Garden);
				UGameplayStatics::OpenLevel(GetWorld(), FName("Garden"));
			}
		)
	);
}

void UMainMenu::RequestWeatherFromStoredLocation()
{
	if (!GetGameInstance())
	{
		if (TXT_WeatherDesc)
		{
			TXT_WeatherDesc->SetText(FText::FromString(TEXT("")));
		}

		if (IMG_WeatherIcon)
		{
			IMG_WeatherIcon->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	UBackendApiSubsystem* Backend = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Backend)
	{
		if (TXT_WeatherDesc)
		{
			TXT_WeatherDesc->SetText(FText::FromString(TEXT("")));
		}

		if (IMG_WeatherIcon)
		{
			IMG_WeatherIcon->SetVisibility(ESlateVisibility::Hidden);
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

		if (IMG_WeatherIcon)
		{
			IMG_WeatherIcon->SetVisibility(ESlateVisibility::Hidden);
		}

		return;
	}

	if (!Location.bHasLatitude || !Location.bHasLongitude)
	{
		if (TXT_CurrentTemp)
		{
			TXT_CurrentTemp->SetText(FText::FromString(TEXT("")));
		}

		if (TXT_WeatherDesc)
		{
			TXT_WeatherDesc->SetText(FText::FromString(TEXT("")));
		}

		if (IMG_WeatherIcon)
		{
			IMG_WeatherIcon->SetVisibility(ESlateVisibility::Hidden);
		}

		return;
	}

	UBackendApiSubsystem* Backend = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Backend)
	{
		if (TXT_WeatherDesc)
		{
			TXT_WeatherDesc->SetText(FText::FromString(TEXT("")));
		}

		UpdateWeatherIcon(TEXT(""));
		return;
	}

	if (IMG_WeatherIcon)
	{
		IMG_WeatherIcon->SetVisibility(ESlateVisibility::Visible);
		IMG_WeatherIcon->SetRenderOpacity(1.0f);
		IMG_WeatherIcon->SetColorAndOpacity(FLinearColor::White);
		if (IconDefault)
		{
			IMG_WeatherIcon->SetBrushFromTexture(IconDefault);
			IMG_WeatherIcon->SetBrushSize(FVector2D(40.0f, 40.0f));
		}
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

	return IconDefault ? IconDefault : WiDaySunny;
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
		IMG_WeatherIcon->SetBrushSize(FVector2D(40.0f, 40.0f));
		IMG_WeatherIcon->SetColorAndOpacity(FLinearColor::White);
		IMG_WeatherIcon->SetRenderOpacity(1.0f);
		IMG_WeatherIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		IMG_WeatherIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainMenu::EnsureWeatherIconDefaults()
{
	if (!WiDaySunny)
	{
		WiDaySunny = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/WeatherIcons/WiDaySunny.WiDaySunny"));
	}

	if (!WiCloudy)
	{
		WiCloudy = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/WeatherIcons/WiCloudy.WiCloudy"));
	}

	if (!WiCloud)
	{
		WiCloud = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/WeatherIcons/WiCloud.WiCloud"));
	}

	if (!WiFog)
	{
		WiFog = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/WeatherIcons/WiFog.WiFog"));
	}

	if (!WiRain)
	{
		WiRain = LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/WeatherIcons/WiRain.WiRain"));
	}

	if (!IconDefault)
	{
		IconDefault = WiDaySunny;
	}
}
