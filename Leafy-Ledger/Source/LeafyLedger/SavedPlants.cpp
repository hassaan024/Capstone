// Fill out your copyright notice in the Description page of Project Settings.

#include "SavedPlants.h"
#include "PlantTile.h"
#include "PlantCardPopup.h"
#include "PlantObject.h"
#include "BackendApiSubsystem.h"
#include "SavedPlantCacheSubsystem.h"

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

	TV_PlantCards->OnEntryWidgetGenerated().AddUObject(this, &USavedPlants::HandleEntryGenerated);
	FetchSavedSpecies();
}

void USavedPlants::HandleEntryGenerated(UUserWidget& EntryWidget)
{
	if (UPlantTile* Tile = Cast<UPlantTile>(&EntryWidget))
	{
		Tile->OnRemoveClicked.AddUniqueDynamic(this, &USavedPlants::HandleRemoveClicked);
	}
}

void USavedPlants::HandleRemoveClicked(int32 PerenualId)
{
	DeleteSavedPlant(PerenualId);
}

void USavedPlants::FetchSavedSpecies()
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

	Cache->RefreshSavedPlants(
		FOnSavedPlantsRefreshed::CreateUObject(this, &USavedPlants::HandleFetchSavedSpeciesResponse)
	);
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
	if (!TV_PlantCards)
	{
		return;
	}

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