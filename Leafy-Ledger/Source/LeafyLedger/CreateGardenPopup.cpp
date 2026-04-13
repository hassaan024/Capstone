// Fill out your copyright notice in the Description page of Project Settings.


#include "CreateGardenPopup.h"
#include "BackendApiSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GardenSessionSubsystem.h"

bool UCreateGardenPopup::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_Create)
    {
        BTN_Create->OnClicked.AddDynamic(this, &UCreateGardenPopup::OnPressCreate);
    }

    return true;
}

void UCreateGardenPopup::OnPressCreate()
{
	if (!ET_GardenName || !ET_GardenDesc)
	{
		UE_LOG(LogTemp, Error, TEXT("ET_GardenName and/or ET_GardenDesc is not bound"));
		return;
	}

	const FString GardenName = ET_GardenName->GetText().ToString().TrimStartAndEnd();
	const FString GardenDesc = ET_GardenDesc->GetText().ToString().TrimStartAndEnd();

	if (GardenName.IsEmpty())
	{
		return;
	}

	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("GameInstance is null"));
		return;
	}

	UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();

	if (!GardenSession)
	{
		UE_LOG(LogTemp, Error, TEXT("GardenSessionSubsystem is missing"));
		return;
	}

	GardenSession->StartNewGardenDraft(GardenName, GardenDesc);

	UBackendApiSubsystem* BackendApi = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!BackendApi)
	{
		UE_LOG(LogTemp, Warning, TEXT("BackendApiSubsystem missing, opening garden without stored location"));
		UGameplayStatics::OpenLevel(GetWorld(), FName("Garden"));
		return;
	}

	BackendApi->GetUserLocation(FBackendUserLocationResponse::CreateUObject(this, &UCreateGardenPopup::HandleGardenLocationResponse));
}

void UCreateGardenPopup::HandleGardenLocationResponse(bool bSuccess, const FString& Message, const FBackendUserLocationDto& Location)
{
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("HandleGardenLocationResponse: GameInstance is null"));
		return;
	}

	UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();
	if (!GardenSession)
	{
		UE_LOG(LogTemp, Error, TEXT("HandleGardenLocationResponse: GardenSessionSubsystem is missing"));
		return;
	}

	if (bSuccess && Location.bHasLatitude && Location.bHasLongitude)
	{
		GardenSession->SetGardenLocation(
			static_cast<float>(Location.Latitude),
			static_cast<float>(Location.Longitude),
			TEXT("")
		);
		UE_LOG(LogTemp, Log, TEXT("Garden draft location set from user profile: lat=%f lon=%f"), Location.Latitude, Location.Longitude);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Garden draft location unavailable: %s"), *Message);
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName("Garden"));
}