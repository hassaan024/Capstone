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

    if (BTN_StartPlanting)
    {
		BTN_StartPlanting->OnClicked.AddDynamic(this, &UCreateGardenPopup::OnPressStartPlanting);
    }

	if (BTN_UseLocation)
	{
		BTN_UseLocation->OnClicked.AddDynamic(this, &UCreateGardenPopup::OnPressUseLocation);
	}

	if (ET_Location)
	{
		ET_Location->OnTextChanged.AddDynamic(this, &UCreateGardenPopup::HandleLocationTextChanged);
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

	if (ET_Location)
	{
		ET_Location->SetHintText(FText::FromString(TEXT("ZIP Code")));
		ET_Location->SetError(FText::GetEmpty());
	}
}

void UCreateGardenPopup::OnPressCreate()
{
	if (!ReadValidatedGardenInput(PendingCreateGardenName, PendingCreateGardenDesc, PendingCreateBloomDate))
	{
		return;
	}

	ResolveSelectedLocation(ECreateGardenSubmitAction::CreateOnly);
}

void UCreateGardenPopup::OnPressStartPlanting()
{
	if (!ReadValidatedGardenInput(PendingCreateGardenName, PendingCreateGardenDesc, PendingCreateBloomDate))
	{
		return;
	}

	ResolveSelectedLocation(ECreateGardenSubmitAction::StartPlanting);
}

void UCreateGardenPopup::OnPressUseLocation()
{
	SelectedLocationSource = ECreateGardenLocationSource::BackendUserLocation;
	if (ET_Location)
	{
		ET_Location->SetText(FText::GetEmpty());
		ET_Location->SetError(FText::GetEmpty());
	}
}

void UCreateGardenPopup::HandleLocationTextChanged(const FText& NewText)
{
	if (!NewText.ToString().TrimStartAndEnd().IsEmpty())
	{
		SelectedLocationSource = ECreateGardenLocationSource::ZipCode;
	}
	else if (SelectedLocationSource == ECreateGardenLocationSource::ZipCode)
	{
		SelectedLocationSource = ECreateGardenLocationSource::None;
	}

	if (ET_Location)
	{
		ET_Location->SetError(FText::GetEmpty());
	}
}

void UCreateGardenPopup::ResolveSelectedLocation(ECreateGardenSubmitAction SubmitAction)
{
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("ResolveSelectedLocation: GameInstance is null"));
		return;
	}

	UBackendApiSubsystem* BackendApi = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!BackendApi)
	{
		UE_LOG(LogTemp, Error, TEXT("ResolveSelectedLocation: BackendApiSubsystem missing"));
		return;
	}

	const bool bHasZipCode = ET_Location && !ET_Location->GetText().ToString().TrimStartAndEnd().IsEmpty();
	if (bHasZipCode)
	{
		SelectedLocationSource = ECreateGardenLocationSource::ZipCode;
	}

	PendingSubmitAction = SubmitAction;

	if (SelectedLocationSource == ECreateGardenLocationSource::BackendUserLocation)
	{
		BackendApi->GetUserLocation(FBackendUserLocationResponse::CreateUObject(this, &UCreateGardenPopup::HandleGardenLocationResponse));
		return;
	}

	if (SelectedLocationSource == ECreateGardenLocationSource::ZipCode)
	{
		FString ZipCode;
		if (!ReadValidatedZipCode(ZipCode))
		{
			PendingSubmitAction = ECreateGardenSubmitAction::None;
			return;
		}

		BackendApi->ResolveZipCodeLocation(ZipCode, FBackendZipLocationResponse::CreateUObject(this, &UCreateGardenPopup::HandleZipLocationResponse));
		return;
	}

	if (ET_Location)
	{
		ET_Location->SetError(FText::FromString(TEXT("Use your saved location or enter a ZIP code.")));
	}
}

bool UCreateGardenPopup::ReadValidatedGardenInput(FString& OutGardenName, FString& OutGardenDesc, FString& OutBloomDate)
{
	if (!ET_GardenName || !ET_GardenDesc || !ET_BloomDate)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGardenPopup: required text boxes are not bound"));
		return false;
	}

	OutGardenName = ET_GardenName->GetText().ToString().TrimStartAndEnd();
	OutGardenDesc = ET_GardenDesc->GetText().ToString().TrimStartAndEnd();
	const FBloomDateValidationResult BloomDateValidation = FBloomDateUtils::ValidateUserInput(ET_BloomDate->GetText().ToString(), false);

	if (OutGardenName.IsEmpty())
	{
		return false;
	}

	if (!BloomDateValidation.bIsValid)
	{
		ET_BloomDate->SetError(FText::FromString(BloomDateValidation.ErrorMessage));
		return false;
	}

	ET_BloomDate->SetText(FText::FromString(BloomDateValidation.DisplayText));
	ET_BloomDate->SetError(FText::GetEmpty());
	OutBloomDate = BloomDateValidation.BackendText;
	return true;
}

bool UCreateGardenPopup::ReadValidatedZipCode(FString& OutZipCode)
{
	if (!ET_Location)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGardenPopup: ET_Location is not bound"));
		return false;
	}

	OutZipCode = ET_Location->GetText().ToString().TrimStartAndEnd();
	if (OutZipCode.IsEmpty())
	{
		ET_Location->SetError(FText::FromString(TEXT("Enter a ZIP code or use your saved location.")));
		return false;
	}

	return true;
}

void UCreateGardenPopup::HandleCreateGardenLocationResponse(bool bSuccess, const FString& Message, const FBackendUserLocationDto& Location)
{
	HandleGardenLocationResponse(bSuccess, Message, Location);
}

void UCreateGardenPopup::HandleZipLocationResponse(bool bSuccess, const FString& Message, float Latitude, float Longitude)
{
	if (!bSuccess)
	{
		if (ET_Location)
		{
			ET_Location->SetError(FText::FromString(Message));
		}
		UE_LOG(LogTemp, Warning, TEXT("CreateGardenPopup: ZIP location unavailable: %s"), *Message);
		PendingSubmitAction = ECreateGardenSubmitAction::None;
		return;
	}

	if (ET_Location)
	{
		ET_Location->SetError(FText::GetEmpty());
	}

	ContinueWithResolvedLocation(Latitude, Longitude, TEXT(""));
}

void UCreateGardenPopup::CreateGardenFromInput(float Latitude, float Longitude, const FString& Timezone)
{
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGardenFromInput: GameInstance is null"));
		return;
	}

	UBackendApiSubsystem* BackendApi = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!BackendApi)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGardenFromInput: BackendApiSubsystem missing"));
		return;
	}

	BackendApi->CreateGarden(
		PendingCreateGardenName,
		PendingCreateGardenDesc,
		PendingCreateBloomDate,
		TEXT(""),
		Latitude,
		Longitude,
		Timezone,
		FBackendGardenResponse::CreateWeakLambda(
			this,
			[this](bool bSuccess, const FString& Message, const FBackendGardenDto& Garden)
			{
				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("CreateGardenPopup: CreateGarden failed: %s"), *Message);
					return;
				}

				UE_LOG(LogTemp, Log, TEXT("CreateGardenPopup: garden created. BackendGardenId=%d"), Garden.Id);
				OnGardenCreated.Broadcast();
				RemoveFromParent();
			}
		)
	);
}

void UCreateGardenPopup::HandleGardenLocationResponse(bool bSuccess, const FString& Message, const FBackendUserLocationDto& Location)
{
	if (!bSuccess || !Location.bHasLatitude || !Location.bHasLongitude)
	{
		if (ET_Location)
		{
			ET_Location->SetError(FText::FromString(TEXT("Saved location is not set. Enter a ZIP code instead.")));
		}
		UE_LOG(LogTemp, Warning, TEXT("Garden location unavailable: %s"), *Message);
		PendingSubmitAction = ECreateGardenSubmitAction::None;
		return;
	}

	if (ET_Location)
	{
		ET_Location->SetError(FText::GetEmpty());
	}

	UE_LOG(LogTemp, Log, TEXT("Garden location set from user profile: lat=%f lon=%f"), Location.Latitude, Location.Longitude);
	ContinueWithResolvedLocation(static_cast<float>(Location.Latitude), static_cast<float>(Location.Longitude), TEXT(""));
}

void UCreateGardenPopup::ContinueWithResolvedLocation(float Latitude, float Longitude, const FString& Timezone)
{
	const ECreateGardenSubmitAction SubmitAction = PendingSubmitAction;
	PendingSubmitAction = ECreateGardenSubmitAction::None;

	if (SubmitAction == ECreateGardenSubmitAction::CreateOnly)
	{
		CreateGardenFromInput(Latitude, Longitude, Timezone);
		return;
	}

	if (SubmitAction == ECreateGardenSubmitAction::StartPlanting)
	{
		if (!GetGameInstance())
		{
			UE_LOG(LogTemp, Error, TEXT("ContinueWithResolvedLocation: GameInstance is null"));
			return;
		}

		UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();
		if (!GardenSession)
		{
			UE_LOG(LogTemp, Error, TEXT("ContinueWithResolvedLocation: GardenSessionSubsystem is missing"));
			return;
		}

		GardenSession->StartNewGardenDraft(PendingCreateGardenName, PendingCreateGardenDesc, PendingCreateBloomDate);
		GardenSession->SetGardenLocation(Latitude, Longitude, Timezone);
		UGameplayStatics::OpenLevel(GetWorld(), FName("Garden"));
	}
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
