// Fill out your copyright notice in the Description page of Project Settings.

#include "ManageSave.h"
#include "BackendApiSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "CreateGardenPopup.h"
#include "SaveDestinationEntry.h"

bool UManageSave::Initialize()
{
	if (!Super::Initialize()) return false;

	if (BTN_CloseCard)
	{
		BTN_CloseCard->OnClicked.AddDynamic(this, &UManageSave::OnPressClose);
	}
	
	if (BTN_Changes)
	{
		BTN_Changes->OnClicked.AddDynamic(this, &UManageSave::OnPressApplyChanges);
	}

	return true;
}

void UManageSave::NativeConstruct()
{
	Super::NativeConstruct();

	ResolveWidgetReferences();
	ConfigureModalLayout();
	RebuildOverlayLayout();
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

void UManageSave::ResolveWidgetReferences()
{
	if (!WidgetTree) return;

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (!RootSizeBox)
		{
			RootSizeBox = Cast<USizeBox>(Widget);
		}

		if (!RootOverlay)
		{
			RootOverlay = Cast<UOverlay>(Widget);
		}

		if (!SaveOptionsScrollBox)
		{
			SaveOptionsScrollBox = Cast<UScrollBox>(Widget);
		}

		if (!CommonNameText)
		{
			if (UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
			{
				const FString CurrentText = TextBlock->GetText().ToString();
				if (CurrentText.Contains(TEXT("common_name")) || CurrentText.StartsWith(TEXT("<")))
				{
					CommonNameText = TextBlock;
					HeaderBox = Cast<UVerticalBox>(TextBlock->GetParent());
				}
			}
		}
	});
}

void UManageSave::ConfigureModalLayout()
{
	if (RootSizeBox)
	{
		RootSizeBox->SetWidthOverride(430.0f);
		RootSizeBox->SetHeightOverride(560.0f);
	}

	if (SaveOptionsScrollBox)
	{
		SaveOptionsScrollBox->SetAnimateWheelScrolling(true);
		SaveOptionsScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
		SaveOptionsScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
	}
}

void UManageSave::RebuildOverlayLayout()
{
	if (!WidgetTree || !RootOverlay || !HeaderBox || !SaveOptionsScrollBox || !BTN_Changes)
	{
		return;
	}

	if (SaveOptionsScrollBox->GetParent() == BTN_Changes->GetParent() && BTN_Changes->GetParent() != RootOverlay)
	{
		return;
	}

	UVerticalBox* ModalLayout = nullptr;
	for (int32 ChildIndex = 0; ChildIndex < RootOverlay->GetChildrenCount(); ++ChildIndex)
	{
		if (UWidget* Widget = RootOverlay->GetChildAt(ChildIndex))
		{
			ModalLayout = Cast<UVerticalBox>(Widget);
			if (ModalLayout)
			{
				break;
			}
		}
	}

	if (!ModalLayout || ModalLayout == HeaderBox)
	{
		ModalLayout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ModalLayout"));
		if (!ModalLayout)
		{
			return;
		}

		if (UOverlaySlot* LayoutSlot = RootOverlay->AddChildToOverlay(ModalLayout))
		{
			LayoutSlot->SetHorizontalAlignment(HAlign_Fill);
			LayoutSlot->SetVerticalAlignment(VAlign_Fill);
			LayoutSlot->SetPadding(FMargin(24.0f, 24.0f, 24.0f, 24.0f));
		}
	}

	auto MoveToModalLayout = [ModalLayout](UWidget* Widget, const FMargin& SlotPadding, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment, float FillHeight)
	{
		if (!Widget || !ModalLayout)
		{
			return;
		}

		if (UPanelWidget* CurrentParent = Widget->GetParent())
		{
			CurrentParent->RemoveChild(Widget);
		}

		if (UVerticalBoxSlot* Slot = ModalLayout->AddChildToVerticalBox(Widget))
		{
			Slot->SetPadding(SlotPadding);
			Slot->SetHorizontalAlignment(HorizontalAlignment);
			Slot->SetVerticalAlignment(VerticalAlignment);
			if (FillHeight > 0.0f)
			{
				Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
				Slot->Size.Value = FillHeight;
			}
			else
			{
				Slot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			}
		}
	};

	MoveToModalLayout(HeaderBox, FMargin(0.0f, 0.0f, 0.0f, 16.0f), HAlign_Fill, VAlign_Top, 0.0f);
	MoveToModalLayout(SaveOptionsScrollBox, FMargin(0.0f, 0.0f, 0.0f, 16.0f), HAlign_Fill, VAlign_Fill, 1.0f);
	MoveToModalLayout(BTN_Changes, FMargin(0.0f), HAlign_Right, VAlign_Bottom, 0.0f);
}

void UManageSave::UpdatePlantNameText() const
{
	if (CommonNameText)
	{
		CommonNameText->SetText(FText::FromString(CommonName.IsEmpty() ? TEXT("<common_name>") : CommonName));
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

	GlobalRow = CreateWidget<USaveDestinationEntry>(GetOwningPlayer(), USaveDestinationEntry::StaticClass());
	if (GlobalRow)
	{
		GlobalRow->Configure(true, 0, TEXT("Save Globally"), TEXT("Available across all gardens in the app"), bIsGloballySaved);
		GlobalRow->OnDestinationClicked.AddDynamic(this, &UManageSave::HandleDestinationClicked);
		SaveOptionsScrollBox->AddChild(GlobalRow);
	}

	AddCreateGardenRow();

	if (!bHasLoadedGardens)
	{
		AddMessageRow(TEXT("Loading your gardens..."));
		AddBottomScrollSpacer();
		return;
	}

	if (Gardens.Num() == 0)
	{
		AddMessageRow(TEXT("No gardens yet. Create one in Unreal to save plants to a specific garden."));
		AddBottomScrollSpacer();
		return;
	}

	AddMessageRow(TEXT("YOUR GARDENS"));

	for (const FBackendGardenSummaryDto& Garden : Gardens)
	{
		USaveDestinationEntry* GardenRow = CreateWidget<USaveDestinationEntry>(GetOwningPlayer(), USaveDestinationEntry::StaticClass());
		if (!GardenRow) continue;

		GardenRow->Configure(false, Garden.Id, Garden.Name, TEXT("Save to this garden"), PendingGardenSaveStates.FindRef(Garden.Id));
		GardenRow->OnDestinationClicked.AddDynamic(this, &UManageSave::HandleDestinationClicked);
		SaveOptionsScrollBox->AddChild(GardenRow);
		GardenRows.Add(Garden.Id, GardenRow);
	}

	AddBottomScrollSpacer();
}

void UManageSave::AddCreateGardenRow()
{
	if (!SaveOptionsScrollBox || !WidgetTree) return;

	CreateGardenButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("BTN_CreateGardenDestination"));
	UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TXT_CreateGardenDestination"));
	if (!CreateGardenButton || !ButtonText)
	{
		CreateGardenButton = nullptr;
		return;
	}

	ButtonText->SetText(FText::FromString(TEXT("+ Create Garden")));
	ButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	CreateGardenButton->SetContent(ButtonText);
	CreateGardenButton->OnClicked.AddDynamic(this, &UManageSave::OnPressCreateGarden);
	SaveOptionsScrollBox->AddChild(CreateGardenButton);
}

void UManageSave::AddMessageRow(const FString& Message)
{
	if (!SaveOptionsScrollBox || !WidgetTree) return;

	UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (!TextBlock) return;

	TextBlock->SetText(FText::FromString(Message));
	TextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.76f, 0.84f, 1.0f)));
	SaveOptionsScrollBox->AddChild(TextBlock);
}

void UManageSave::AddBottomScrollSpacer()
{
	if (!SaveOptionsScrollBox || !WidgetTree)
	{
		return;
	}

	USpacer* Spacer = WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass());
	if (!Spacer)
	{
		return;
	}

	Spacer->SetSize(FVector2D(1.0f, 96.0f));
	SaveOptionsScrollBox->AddChild(Spacer);
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
	if (TXT_Changes)
	{
		TXT_Changes->SetText(FText::FromString(HasPendingChanges() ? TEXT("Save Changes") : TEXT("No Changes")));
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
