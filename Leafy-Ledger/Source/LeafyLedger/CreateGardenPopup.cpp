// Fill out your copyright notice in the Description page of Project Settings.

#include "CreateGardenPopup.h"
#include "BackendApiSubsystem.h"
#include "BloomDateUtils.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GardenSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"

bool UCreateGardenPopup::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_Create)
    {
        BTN_Create->OnClicked.AddDynamic(this, &UCreateGardenPopup::OnPressCreate);
    }

    return true;
}

void UCreateGardenPopup::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureBloomDateField();

	if (ET_BloomDate)
	{
		ET_BloomDate->SetHintText(FText::FromString(TEXT("MM/DD/YYYY")));
		ET_BloomDate->SetError(FText::GetEmpty());
	}
}

void UCreateGardenPopup::OnPressCreate()
{
	if (!ET_GardenName || !ET_GardenDesc || !ET_BloomDate)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGardenPopup: required text boxes are not bound"));
		return;
	}

	const FString GardenName = ET_GardenName->GetText().ToString().TrimStartAndEnd();
	const FString GardenDesc = ET_GardenDesc->GetText().ToString().TrimStartAndEnd();
	const FBloomDateValidationResult BloomDateValidation = FBloomDateUtils::ValidateUserInput(ET_BloomDate->GetText().ToString(), false);

	if (GardenName.IsEmpty())
	{
		return;
	}

	if (!BloomDateValidation.bIsValid)
	{
		ET_BloomDate->SetError(FText::FromString(BloomDateValidation.ErrorMessage));
		return;
	}

	ET_BloomDate->SetText(FText::FromString(BloomDateValidation.DisplayText));
	ET_BloomDate->SetError(FText::GetEmpty());

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

	GardenSession->StartNewGardenDraft(GardenName, GardenDesc, BloomDateValidation.BackendText);

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

void UCreateGardenPopup::EnsureBloomDateField()
{
	if (ET_BloomDate || !WidgetTree)
	{
		return;
	}

	UWidget* SearchStart = ET_GardenDesc ? static_cast<UWidget*>(ET_GardenDesc) : static_cast<UWidget*>(ET_GardenName);
	UPanelWidget* ParentPanel = SearchStart ? SearchStart->GetParent() : nullptr;

	while (ParentPanel && !Cast<UVerticalBox>(ParentPanel))
	{
		ParentPanel = ParentPanel->GetParent();
	}

	UVerticalBox* FormLayout = Cast<UVerticalBox>(ParentPanel);
	if (!FormLayout)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateGardenPopup: unable to locate a vertical layout for bloom date input"));
		return;
	}

	UTextBlock* BloomDateLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TXT_BloomDate"));
	ET_BloomDate = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("ET_BloomDate"));

	if (!BloomDateLabel || !ET_BloomDate)
	{
		ET_BloomDate = nullptr;
		return;
	}

	BloomDateLabel->SetText(FText::FromString(TEXT("Bloom Date")));
	if (UVerticalBoxSlot* LabelSlot = FormLayout->AddChildToVerticalBox(BloomDateLabel))
	{
		LabelSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 4.0f));
		LabelSlot->SetHorizontalAlignment(HAlign_Center);
	}

	if (UVerticalBoxSlot* InputSlot = FormLayout->AddChildToVerticalBox(ET_BloomDate))
	{
		InputSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		InputSlot->SetHorizontalAlignment(HAlign_Center);
	}
}
