// Fill out your copyright notice in the Description page of Project Settings.

#include "DisplayName.h"
#include "BackendApiSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/EditableTextBox.h"

bool UDisplayName::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (BTN_GoogleName)
	{
		BTN_GoogleName->OnClicked.AddDynamic(this, &UDisplayName::OnPressGoogleName);
	}

	if (BTN_SubmitName)
	{
		BTN_SubmitName->OnClicked.AddDynamic(this, &UDisplayName::OnPressSubmit);
	}

	if (BTN_Back)
	{
		BTN_Back->OnClicked.AddDynamic(this, &UDisplayName::OnPressBack);
	}

	return true;
}

void UDisplayName::NativeConstruct()
{
	Super::NativeConstruct();

	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("No GameInstance"));
		return;
	}

	UBackendApiSubsystem* Api = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Api)
	{
		UE_LOG(LogTemp, Error, TEXT("Backend API subsystem not found"));
		return;
	}

	Api->GetCurrentUser(FBackendCurrentUserResponse::CreateUObject(this, &UDisplayName::HandleGetCurrentUserResponse));
}

void UDisplayName::HandleGetCurrentUserResponse(bool bSuccess, const FString& Message, const FBackendUserDto& User)
{
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("GetCurrentUser failed: %s"), *Message);
		return;
	}

	CurrentUser = User;
	bHasCurrentUser = true;

	if (ET_DisplayName && !CurrentUser.DisplayName.IsEmpty())
	{
		ET_DisplayName->SetText(FText::FromString(CurrentUser.DisplayName));
	}
}

void UDisplayName::OnPressGoogleName()
{
	if (!bHasCurrentUser)
	{
		UE_LOG(LogTemp, Warning, TEXT("Current user not loaded yet"));
		return;
	}

	const FString NewName = CurrentUser.GoogleDisplayName.TrimStartAndEnd();
	if (NewName.IsEmpty())
	{
		return;
	}

	SubmitDisplayName(NewName);
}

void UDisplayName::OnPressSubmit()
{
	if (!ET_DisplayName)
	{
		UE_LOG(LogTemp, Error, TEXT("ET_DisplayName is not bound"));
		return;
	}

	const FString NewName = ET_DisplayName->GetText().ToString().TrimStartAndEnd();
	if (NewName.IsEmpty())
	{
		return;
	}

	SubmitDisplayName(NewName);
}

void UDisplayName::SubmitDisplayName(const FString& NewName)
{
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("No GameInstance"));
		return;
	}

	UBackendApiSubsystem* Api = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Api)
	{
		UE_LOG(LogTemp, Error, TEXT("Backend API subsystem not found"));
		return;
	}

	Api->UpdateDisplayName(
		NewName,
		FBackendOperationResponse::CreateUObject(this, &UDisplayName::HandleUpdateDisplayNameResponse)
	);
}

void UDisplayName::HandleUpdateDisplayNameResponse(bool bSuccess, const FString& Message)
{
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("UpdateDisplayName failed: %s"), *Message);
		return;
	}

	MenuController = Cast<AMenuController>(UGameplayStatics::GetPlayerController(this, 0));
	if (MenuController)
	{
		MenuController->ShowMainMenu();
	}
}

void UDisplayName::OnPressBack()
{
	MenuController = Cast<AMenuController>(UGameplayStatics::GetPlayerController(this, 0));
	if (MenuController)
	{
		MenuController->ShowMainMenu();
	}
}