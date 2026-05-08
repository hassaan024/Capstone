// Fill out your copyright notice in the Description page of Project Settings.

#include "ManageSave.h"
#include "BackendApiSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "CreateGardenPopup.h"
#include "SaveDestinationEntry.h"

bool UManageSave::Initialize()
{
	if (!Super::Initialize()) return false;

	if (BTN_CloseCard)
	{
		BTN_CloseCard->OnClicked.AddDynamic(this, &UManageSave::OnPressClose);
	}
	
	if (UButton* ChangesResolvedButton = ResolveButton(BTN_Changes))
	{
		ChangesResolvedButton->OnClicked.AddDynamic(this, &UManageSave::OnPressApplyChanges);
	}

	if (UButton* CreateGardenResolvedButton = ResolveButton(BTN_CreateGarden))
	{
		CreateGardenResolvedButton->OnClicked.AddDynamic(this, &UManageSave::OnPressCreateGarden);
	}

	return true;
}

void UManageSave::NativeConstruct()
{
	Super::NativeConstruct();

	ResolveWidgetReferences();
	ConfigureModalLayout();

	if (UButton* ChangesResolvedButton = ResolveButton(BTN_Changes))
	{
		ChangesResolvedButton->OnClicked.RemoveDynamic(this, &UManageSave::OnPressApplyChanges);
		ChangesResolvedButton->OnClicked.AddDynamic(this, &UManageSave::OnPressApplyChanges);
	}

	if (UButton* CreateGardenResolvedButton = ResolveButton(BTN_CreateGarden))
	{
		CreateGardenResolvedButton->OnClicked.RemoveDynamic(this, &UManageSave::OnPressCreateGarden);
		CreateGardenResolvedButton->OnClicked.AddDynamic(this, &UManageSave::OnPressCreateGarden);
	}

	UpdatePlantNameText();
	UpdateChangesText();
	RefreshDestinationList();
}

void UManageSave::ConfigureForPlant(int32 InPerenualId, const FString& InCommonName)
{
	PerenualId = InPerenualId;
	CommonName = InCommonName;
	UpdatePlantNameText();
}

void UManageSave::OnPressClose()
{
	RemoveFromParent();
}

void UManageSave::HandleDestinationClicked(bool bIsGlobalDestination, int32 GardenId)
{
	if (bIsApplyingToggle)
	{
		return;
	}

	if (bIsGlobalDestination)
	{
		bPendingGlobalSaved = !bPendingGlobalSaved;
	}
	else
	{
		const bool bCurrentValue = PendingGardenSaveStates.FindRef(GardenId);
		PendingGardenSaveStates.Add(GardenId, !bCurrentValue);
	}

	UpdateExistingRowStates();
	UpdateChangesText();
}

void UManageSave::OnPressApplyChanges()
{
	if (bIsApplyingToggle || !GetGameInstance() || !HasPendingChanges()) return;

	UBackendApiSubsystem* Api = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Api) return;

	TArray<TFunction<void(const FBackendOperationResponse&)>> Operations;

	if (bPendingGlobalSaved != bIsGloballySaved)
	{
		const bool bShouldSaveGlobal = bPendingGlobalSaved;
		Operations.Add(
			[this, Api, bShouldSaveGlobal](const FBackendOperationResponse& Callback)
			{
				if (bShouldSaveGlobal)
				{
					Api->SavePlant(PerenualId, Callback);
				}
				else
				{
					Api->DeleteSavedPlant(PerenualId, Callback);
				}
			}
		);
	}

	for (const FBackendGardenSummaryDto& Garden : Gardens)
	{
		const bool bOriginalSaved = GardenSaveStates.FindRef(Garden.Id);
		const bool bPendingSaved = PendingGardenSaveStates.FindRef(Garden.Id);
		if (bOriginalSaved == bPendingSaved) continue;

		const int32 GardenId = Garden.Id;
		Operations.Add(
			[this, Api, GardenId, bPendingSaved](const FBackendOperationResponse& Callback)
			{
				if (bPendingSaved)
				{
					Api->SavePlantToGarden(PerenualId, GardenId, Callback);
				}
				else
				{
					Api->DeleteSavedPlantFromGarden(PerenualId, GardenId, Callback);
				}
			}
		);
	}

	if (Operations.Num() == 0)
	{
		UpdateChangesText();
		return;
	}

	bIsApplyingToggle = true;

	TSharedRef<int32, ESPMode::ThreadSafe> RemainingOperations = MakeShared<int32, ESPMode::ThreadSafe>(Operations.Num());
	TSharedRef<bool, ESPMode::ThreadSafe> bHadFailure = MakeShared<bool, ESPMode::ThreadSafe>(false);

	for (const TFunction<void(const FBackendOperationResponse&)>& Operation : Operations)
	{
		Operation(
			FBackendOperationResponse::CreateWeakLambda(
				this,
				[this, RemainingOperations, bHadFailure](bool bSuccess, const FString& Message)
				{
					if (!bSuccess)
					{
						*bHadFailure = true;
						UE_LOG(LogTemp, Warning, TEXT("ManageSave: apply changes failed: %s"), *Message);
					}

					--(*RemainingOperations);
					if (*RemainingOperations > 0)
					{
						return;
					}

					bIsApplyingToggle = false;

					if (*bHadFailure)
					{
						UpdateChangesText();
						return;
					}

					bIsGloballySaved = bPendingGlobalSaved;
					GardenSaveStates = PendingGardenSaveStates;
					UpdateExistingRowStates();
					UpdateChangesText();
					OnSaveApplied.Broadcast(PerenualId, bIsGloballySaved);
					RemoveFromParent();
				}
			)
		);
	}
}

void UManageSave::OnPressCreateGarden()
{
	if (!CreateGardenPopupClass)
	{
		CreateGardenPopupClass = LoadClass<UCreateGardenPopup>(nullptr, TEXT("/Game/UI/WB_CreateGarden.WB_CreateGarden_C"));
		if (!CreateGardenPopupClass)
		{
			UE_LOG(LogTemp, Error, TEXT("ManageSave: failed to load /Game/UI/WB_CreateGarden.WB_CreateGarden_C"));
			return;
		}
	}

	if (CreateGardenPopupInstance)
	{
		CreateGardenPopupInstance->RemoveFromParent();
		CreateGardenPopupInstance = nullptr;
	}

	CreateGardenPopupInstance = CreateWidget<UCreateGardenPopup>(GetOwningPlayer(), CreateGardenPopupClass);
	if (!CreateGardenPopupInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("ManageSave: failed to create CreateGarden popup"));
		return;
	}

	CreateGardenPopupInstance->OnGardenCreated.AddUniqueDynamic(this, &UManageSave::HandleGardenCreated);
	CreateGardenPopupInstance->AddToViewport(40);
}

void UManageSave::HandleGardenCreated()
{
	CreateGardenPopupInstance = nullptr;
	RefreshDestinationList();
}

UButton* UManageSave::ResolveButton(UWidget* ButtonWidget) const
{
	if (!ButtonWidget)
	{
		return nullptr;
	}

	if (UButton* DirectButton = Cast<UButton>(ButtonWidget))
	{
		return DirectButton;
	}

	const UUserWidget* ButtonUserWidget = Cast<UUserWidget>(ButtonWidget);
	if (!ButtonUserWidget || !ButtonUserWidget->WidgetTree)
	{
		return nullptr;
	}

	UButton* ResolvedButton = nullptr;
	ButtonUserWidget->WidgetTree->ForEachWidget([&ResolvedButton](UWidget* Widget)
	{
		if (!ResolvedButton)
		{
			ResolvedButton = Cast<UButton>(Widget);
		}
	});

	return ResolvedButton;
}

void UManageSave::SetButtonLabel(UWidget* ButtonWidget, const FString& Label) const
{
	if (!ButtonWidget)
	{
		return;
	}

	if (UButton* DirectButton = Cast<UButton>(ButtonWidget))
	{
		if (UTextBlock* TextBlock = Cast<UTextBlock>(DirectButton->GetContent()))
		{
			TextBlock->SetText(FText::FromString(Label));
		}
		return;
	}

	const UUserWidget* ButtonUserWidget = Cast<UUserWidget>(ButtonWidget);
	if (!ButtonUserWidget || !ButtonUserWidget->WidgetTree)
	{
		return;
	}

	UTextBlock* ResolvedText = nullptr;
	ButtonUserWidget->WidgetTree->ForEachWidget([&ResolvedText](UWidget* Widget)
	{
		if (ResolvedText)
		{
			return;
		}

		if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			if (TextBlock->GetName().Contains(TEXT("TXT_ButtonText")) || TextBlock->GetName().Contains(TEXT("ButtonText")))
			{
				ResolvedText = TextBlock;
			}
		}
	});

	if (ResolvedText)
	{
		ResolvedText->SetText(FText::FromString(Label));
	}
}

USaveDestinationEntry* UManageSave::CreateDestinationEntry(const FName& EntryName) const
{
	TSubclassOf<USaveDestinationEntry> EntryClass = SaveDestinationEntryClass;
	if (!EntryClass)
	{
		EntryClass = LoadClass<USaveDestinationEntry>(nullptr, TEXT("/Game/UI/WB_SaveDestinationEntry.WB_SaveDestinationEntry_C"));
	}

	if (!EntryClass)
	{
		EntryClass = USaveDestinationEntry::StaticClass();
	}

	return CreateWidget<USaveDestinationEntry>(GetOwningPlayer(), EntryClass, EntryName);
}

void UManageSave::ResolveWidgetReferences()
{
	if (!WidgetTree) return;

	if (!SaveOptionsScrollBox)
	{
		SaveOptionsScrollBox = SB_SaveDestinations;
	}

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (!SaveOptionsScrollBox)
		{
			SaveOptionsScrollBox = Cast<UScrollBox>(Widget);
		}

		if (!TXT_PlantName)
		{
			if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
			{
				const FString CurrentText = TextBlock->GetText().ToString();
				if (CurrentText.Contains(TEXT("common_name")) || CurrentText.StartsWith(TEXT("<")))
				{
					TXT_PlantName = TextBlock;
				}
			}
		}
	});
}

void UManageSave::ConfigureModalLayout()
{
	if (SaveOptionsScrollBox)
	{
		SaveOptionsScrollBox->SetAnimateWheelScrolling(true);
		SaveOptionsScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
		SaveOptionsScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
	}
}

void UManageSave::UpdatePlantNameText() const
{
	if (TXT_PlantName)
	{
		TXT_PlantName->SetText(FText::FromString(CommonName.IsEmpty() ? TEXT("<common_name>") : CommonName));
	}
}

void UManageSave::RefreshDestinationList()
{
	Gardens.Reset();
	GardenSaveStates.Reset();
	PendingGardenSaveStates.Reset();
	GardenRows.Reset();
	GlobalRow = nullptr;
	bHasLoadedGardens = false;
	bPendingGlobalSaved = false;

	PopulateDestinationList();
	UpdateChangesText();

	if (!GetGameInstance())
	{
		AddMessageRow(TEXT("Game instance is unavailable."));
		return;
	}

	UBackendApiSubsystem* Api = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Api)
	{
		AddMessageRow(TEXT("Backend API is unavailable."));
		return;
	}

	Api->GetSavedSpecies(FBackendPlantsResponse::CreateUObject(this, &UManageSave::HandleGlobalStateResponse));
	Api->GetGardensByUser(FBackendGardenSummariesResponse::CreateUObject(this, &UManageSave::HandleGardenListResponse));
}

void UManageSave::PopulateDestinationList()
{
	if (!SaveOptionsScrollBox) return;

	SaveOptionsScrollBox->ClearChildren();
	GardenRows.Reset();

	if (TXT_StatusMessage)
	{
		TXT_StatusMessage->SetText(FText::GetEmpty());
		TXT_StatusMessage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (TXT_GardenSection)
	{
		TXT_GardenSection->SetVisibility(ESlateVisibility::Collapsed);
	}

	GlobalRow = CreateDestinationEntry(TEXT("GlobalSaveDestination"));
	if (GlobalRow)
	{
		GlobalRow->Configure(true, 0, TEXT("Save Globally"), TEXT("Available across all gardens in the app"), bIsGloballySaved);
		GlobalRow->OnDestinationClicked.AddDynamic(this, &UManageSave::HandleDestinationClicked);
		SaveOptionsScrollBox->AddChild(GlobalRow);
	}

	if (!bHasLoadedGardens)
	{
		AddMessageRow(TEXT("Loading your gardens..."));
		return;
	}

	if (Gardens.Num() == 0)
	{
		AddMessageRow(TEXT("No gardens yet. Create one in Unreal to save plants to a specific garden."));
		return;
	}

	if (TXT_GardenSection)
	{
		TXT_GardenSection->SetText(FText::FromString(TEXT("Your Gardens")));
		TXT_GardenSection->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		AddMessageRow(TEXT("Your Gardens"));
	}

	for (const FBackendGardenSummaryDto& Garden : Gardens)
	{
		USaveDestinationEntry* GardenRow = CreateDestinationEntry(*FString::Printf(TEXT("GardenSaveDestination_%d"), Garden.Id));
		if (!GardenRow) continue;

		GardenRow->Configure(false, Garden.Id, Garden.Name, TEXT("Save to this garden"), PendingGardenSaveStates.FindRef(Garden.Id));
		GardenRow->OnDestinationClicked.AddDynamic(this, &UManageSave::HandleDestinationClicked);
		SaveOptionsScrollBox->AddChild(GardenRow);
		GardenRows.Add(Garden.Id, GardenRow);
	}
}

void UManageSave::AddMessageRow(const FString& Message)
{
	if (TXT_StatusMessage)
	{
		TXT_StatusMessage->SetText(FText::FromString(Message));
		TXT_StatusMessage->SetVisibility(Message.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		return;
	}

	if (!SaveOptionsScrollBox || !WidgetTree) return;

	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (!TextBlock) return;

	TextBlock->SetText(FText::FromString(Message));
	TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.76f, 0.84f, 1.0f)));
	SaveOptionsScrollBox->AddChild(TextBlock);
}

void UManageSave::HandleGlobalStateResponse(bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
{
	bIsGloballySaved = false;

	if (bSuccess)
	{
		for (const FBackendPlantDto& Plant : Plants)
		{
			if (Plant.PerenualId == PerenualId)
			{
				bIsGloballySaved = true;
				break;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ManageSave: failed to load global saved state: %s"), *Message);
	}

	bPendingGlobalSaved = bIsGloballySaved;
	UpdateExistingRowStates();
	UpdateChangesText();
}

void UManageSave::HandleGardenListResponse(bool bSuccess, const FString& Message, const TArray<FBackendGardenSummaryDto>& InGardens)
{
	bHasLoadedGardens = true;
	Gardens = bSuccess ? InGardens : TArray<FBackendGardenSummaryDto>();

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("ManageSave: failed to load gardens: %s"), *Message);
	}

	for (const FBackendGardenSummaryDto& Garden : Gardens)
	{
		GardenSaveStates.Add(Garden.Id, false);
		PendingGardenSaveStates.Add(Garden.Id, false);
	}

	PopulateDestinationList();

	if (Gardens.Num() > 0)
	{
		RequestGardenStateAtIndex(0);
	}
}

void UManageSave::RequestGardenStateAtIndex(int32 GardenIndex)
{
	if (!Gardens.IsValidIndex(GardenIndex) || !GetGameInstance())
	{
		UpdateExistingRowStates();
		return;
	}

	UBackendApiSubsystem* Api = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Api) return;

	const FBackendGardenSummaryDto Garden = Gardens[GardenIndex];
	Api->GetSavedSpeciesForGarden(
		Garden.Id,
		FBackendPlantsResponse::CreateWeakLambda(
			this,
			[this, GardenId = Garden.Id, GardenIndex](bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
			{
				HandleGardenStateResponse(GardenId, bSuccess, Message, Plants);
				RequestGardenStateAtIndex(GardenIndex + 1);
			}
		)
	);
}

void UManageSave::HandleGardenStateResponse(int32 GardenId, bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
{
	bool bSaved = false;

	if (bSuccess)
	{
		for (const FBackendPlantDto& Plant : Plants)
		{
			if (Plant.PerenualId == PerenualId)
			{
				bSaved = true;
				break;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ManageSave: failed to load state for garden %d: %s"), GardenId, *Message);
	}

	GardenSaveStates.Add(GardenId, bSaved);
	PendingGardenSaveStates.Add(GardenId, bSaved);
	UpdateExistingRowStates();
	UpdateChangesText();
}

bool UManageSave::HasPendingChanges() const
{
	if (bPendingGlobalSaved != bIsGloballySaved)
	{
		return true;
	}

	for (const FBackendGardenSummaryDto& Garden : Gardens)
	{
		if (PendingGardenSaveStates.FindRef(Garden.Id) != GardenSaveStates.FindRef(Garden.Id))
		{
			return true;
		}
	}

	return false;
}

void UManageSave::UpdateChangesText()
{
	const FString ChangesLabel = HasPendingChanges() ? TEXT("Save changes") : TEXT("No changes");

	SetButtonLabel(BTN_Changes, ChangesLabel);

	if (TXT_Changes)
	{
		TXT_Changes->SetText(FText::FromString(ChangesLabel));
	}
}

void UManageSave::UpdateExistingRowStates()
{
	if (GlobalRow)
	{
		GlobalRow->SetChecked(bPendingGlobalSaved);
	}

	for (const TPair<int32, USaveDestinationEntry*>& Pair : GardenRows)
	{
		if (Pair.Value)
		{
			Pair.Value->SetChecked(PendingGardenSaveStates.FindRef(Pair.Key));
		}
	}
}
