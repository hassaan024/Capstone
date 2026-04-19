// Fill out your copyright notice in the Description page of Project Settings.

#include "SavedPlants.h"
#include "PlantTile.h"
#include "PlantCardPopup.h"
#include "PlantObject.h"
#include "BackendApiSubsystem.h"
#include "SavedPlantCacheSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"

void UGardenSelectorButtonProxy::Initialize(USavedPlants* InOwner, int32 InGardenId)
{
	Owner = InOwner;
	GardenId = InGardenId;
}

void UGardenSelectorButtonProxy::HandleClicked()
{
	if (Owner)
	{
		Owner->SelectGarden(GardenId);
	}
}

void USavedPlants::NativeConstruct()
{
	Super::NativeConstruct();

	if (!TV_PlantCards)
	{
		UE_LOG(LogTemp, Error, TEXT("TV_PlantCards is not bound"));
		return;
	}

	MenuController = Cast<AMenuController>(UGameplayStatics::GetPlayerController(this, 0));

	if (!MenuController)
	{
		return;
	}

	if (BTN_Back)
	{
		BTN_Back->OnClicked.AddDynamic(this, &USavedPlants::OnPressBack);
	}

	if (WB_Gardens)
	{
		WB_Gardens->SetInnerSlotPadding(FVector2D(12.0f, 12.0f));
	}

	TV_PlantCards->OnEntryWidgetGenerated().AddUObject(this, &USavedPlants::HandleEntryGenerated);
	FetchGardens();
	RefreshCurrentSource();
}

void USavedPlants::HandleEntryGenerated(UUserWidget& EntryWidget)
{
	if (UPlantTile* Tile = Cast<UPlantTile>(&EntryWidget))
	{
		Tile->OnRemoveClicked.AddUniqueDynamic(this, &USavedPlants::HandleRemoveClicked);
		Tile->OnSaveStateChanged.AddUniqueDynamic(this, &USavedPlants::HandleSaveStateChanged);
	}
}

void USavedPlants::HandleRemoveClicked(int32 PerenualId)
{
	if (SelectedGardenId > 0)
	{
		RefreshCurrentSource();
		return;
	}

	DeleteSavedPlant(PerenualId);
}

void USavedPlants::HandleSaveStateChanged()
{
	RefreshCurrentSource();
}

void USavedPlants::RefreshCurrentSource()
{
	if (SelectedGardenId > 0)
	{
		FetchGardenSavedSpecies(SelectedGardenId);
		return;
	}

	FetchGlobalSavedSpecies();
}

void USavedPlants::FetchGlobalSavedSpecies()
{
	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Error, TEXT("No GameInstance"));
		return;
	}

	USavedPlantCacheSubsystem* Cache = GetGameInstance()->GetSubsystem<USavedPlantCacheSubsystem>();
	if (!Cache)
	{
		UE_LOG(LogTemp, Error, TEXT("SavedPlantCacheSubsystem not found"));
		return;
	}

	if (Cache->HasCachedPlants())
	{
		UE_LOG(LogTemp, Log, TEXT("Using cached saved plants immediately"));
		PopulatePlants(Cache->GetCachedPlants());
	}

	Cache->RefreshSavedPlants(FOnSavedPlantsRefreshed::CreateUObject(this, &USavedPlants::HandleFetchSavedSpeciesResponse));
}

void USavedPlants::FetchGardenSavedSpecies(int32 GardenId)
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

	Api->GetSavedSpeciesForGarden(GardenId, FBackendPlantsResponse::CreateUObject(this, &USavedPlants::HandleFetchSavedSpeciesResponse));
}

void USavedPlants::FetchGardens()
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

	Api->GetGardensByUser(FBackendGardenSummariesResponse::CreateUObject(this, &USavedPlants::HandleFetchGardensResponse));
}

void USavedPlants::HandleFetchSavedSpeciesResponse(bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants)
{
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("FetchSavedSpecies failed: %s"), *Message);
		return;
	}

	PopulatePlants(Plants);
}

void USavedPlants::HandleFetchGardensResponse(bool bSuccess, const FString& Message, const TArray<FBackendGardenSummaryDto>& InGardens)
{
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetGardensByUser failed: %s"), *Message);
		Gardens.Reset();
		RebuildGardenTabs();
		return;
	}

	Gardens = InGardens;
	RebuildGardenTabs();
}

void USavedPlants::PopulatePlants(const TArray<FBackendPlantDto>& Plants)
{
	if (!TV_PlantCards)
	{
		return;
	}

	TV_PlantCards->ClearListItems();

	for (const FBackendPlantDto& Plant : Plants)
	{
		UPlantObject* PlantObject = NewObject<UPlantObject>(this);
		if (!PlantObject)
		{
			continue;
		}

		PlantObject->CommonName = Plant.CommonName;
		PlantObject->ScientificName = Plant.ScientificName;
		PlantObject->ImgSrcUrl = Plant.ImgSrcUrls.Regular;
		PlantObject->SpeciesId = Plant.Id;
		PlantObject->PerenualId = Plant.PerenualId;
		PlantObject->ModelCategory = Plant.ModelCategory;

		//UE_LOG(LogTemp, Log, TEXT("PopulatePlants: %s Plant.Id=%d Plant.PerenualId=%d"), *Plant.CommonName, Plant.Id, Plant.PerenualId);

		TV_PlantCards->AddItem(PlantObject);
	}
}

void USavedPlants::RebuildGardenTabs()
{
	if (!WB_Gardens || !WidgetTree) return;

	WB_Gardens->ClearChildren();
	GardenButtonToId.Reset();
	GardenButtonLabels.Reset();
	GardenButtonHandlers.Reset();

	TMap<FString, int32> NameCounts;
	for (const FBackendGardenSummaryDto& Garden : Gardens)
	{
		NameCounts.FindOrAdd(Garden.Name)++;
	}

	TSet<FString> DuplicateNames;
	for (const TPair<FString, int32>& Pair : NameCounts)
	{
		if (Pair.Value > 1)
		{
			DuplicateNames.Add(Pair.Key);
		}
	}

	CreateGardenTabButton(0, TEXT("Global"));

	for (const FBackendGardenSummaryDto& Garden : Gardens)
	{
		const FString Label = BuildGardenOptionLabel(Garden, DuplicateNames);
		CreateGardenTabButton(Garden.Id, Label);
	}

	SelectedGardenId = GetSelectedGardenId();
	UpdateGardenTabStyles();
}

void USavedPlants::CreateGardenTabButton(int32 GardenId, const FString& Label)
{
	if (!WB_Gardens || !WidgetTree) return;

	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (!Button || !LabelText) return;

	LabelText->SetText(FText::FromString(Label));
	LabelText->SetColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.76f, 0.84f, 1.0f)));
	Button->IsFocusable = true;
	Button->SetBackgroundColor(FLinearColor(0.12f, 0.16f, 0.25f, 1.0f));
	Button->AddChild(LabelText);

	UGardenSelectorButtonProxy* Handler = NewObject<UGardenSelectorButtonProxy>(this);
	if (Handler)
	{
		Handler->Initialize(this, GardenId);
		Button->OnClicked.AddDynamic(Handler, &UGardenSelectorButtonProxy::HandleClicked);
		GardenButtonHandlers.Add(Handler);
	}

	if (UWrapBoxSlot* WrapSlot = WB_Gardens->AddChildToWrapBox(Button))
	{
		WrapSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 12.f));
		WrapSlot->SetFillEmptySpace(false);
		WrapSlot->SetFillSpanWhenLessThan(0.0f);
	}

	GardenButtonToId.Add(Button, GardenId);
	GardenButtonLabels.Add(Button, LabelText);
}

void USavedPlants::UpdateGardenTabStyles()
{
	for (const TPair<UButton*, int32>& Pair : GardenButtonToId)
	{
		UButton* Button = Pair.Key;
		if (!Button) continue;

		const bool bIsSelected = Pair.Value == SelectedGardenId;
		Button->SetBackgroundColor(bIsSelected
			? FLinearColor(0.11f, 0.36f, 0.24f, 1.0f)
			: FLinearColor(0.12f, 0.16f, 0.25f, 1.0f));

		if (UTextBlock** LabelText = GardenButtonLabels.Find(Button))
		{
			(*LabelText)->SetColorAndOpacity(FSlateColor(
				bIsSelected
					? FLinearColor(1.f, 1.f, 1.f, 1.f)
					: FLinearColor(0.72f, 0.76f, 0.84f, 1.0f)));
		}
	}
}

FString USavedPlants::BuildGardenOptionLabel(const FBackendGardenSummaryDto& Garden, const TSet<FString>& DuplicateNames) const
{
	if (DuplicateNames.Contains(Garden.Name))
	{
		return FString::Printf(TEXT("%s (%d)"), *Garden.Name, Garden.Id);
	}

	return Garden.Name;
}

int32 USavedPlants::GetSelectedGardenId() const
{
	if (SelectedGardenId == 0)
	{
		return 0;
	}

	for (const FBackendGardenSummaryDto& Garden : Gardens)
	{
		if (Garden.Id == SelectedGardenId)
		{
			return SelectedGardenId;
		}
	}

	return 0;
}

void USavedPlants::SelectGarden(int32 GardenId)
{
	SelectedGardenId = GardenId;
	UpdateGardenTabStyles();
	RefreshCurrentSource();
}

void USavedPlants::DeleteSavedPlant(int32 PerenualId)
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

	Api->DeleteSavedPlant(
		PerenualId,
		FBackendOperationResponse::CreateLambda(
			[this, PerenualId](bool bSuccess, const FString& Message)
			{
				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("DeleteSavedPlant failed for %d: %s"), PerenualId, *Message);
					return;
				}

				USavedPlantCacheSubsystem* Cache = GetGameInstance()->GetSubsystem<USavedPlantCacheSubsystem>();
				if (Cache)
				{
					Cache->RemoveCachedPlantByPerenualId(PerenualId);
				}

				RemoveSavedPlantFromList(PerenualId);
			}
		)
	);
}

void USavedPlants::RemoveSavedPlantFromList(int32 PerenualId)
{
	if (!TV_PlantCards) return;

	TArray<UObject*> Items = TV_PlantCards->GetListItems();
	for (UObject* Obj : Items)
	{
		UPlantObject* Plant = Cast<UPlantObject>(Obj);
		if (Plant && Plant->PerenualId == PerenualId)
		{
			TV_PlantCards->RemoveItem(Obj);
			TV_PlantCards->RequestRefresh();
			return;
		}
	}
}

void USavedPlants::OnPressBack()
{
	MenuController = Cast<AMenuController>(UGameplayStatics::GetPlayerController(this, 0));
	if (MenuController)
	{
		MenuController->ShowMainMenu();
	}
}
