// Fill out your copyright notice in the Description page of Project Settings.


#include "CreateGardenPopup.h"
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

	UGameplayStatics::OpenLevel(GetWorld(), FName("Garden"));
}