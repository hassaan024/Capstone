// Fill out your copyright notice in the Description page of Project Settings.

#include "SavedPlants.h"
#include "SavedPlantTile.h"
#include "PlantObject.h"
#include "BackendApiSubsystem.h"

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
	if (USavedPlantTile* Tile = Cast<USavedPlantTile>(&EntryWidget))
	{
		Tile->OnRemoveClicked.AddUniqueDynamic(this, &USavedPlants::HandleRemoveClicked);
	}
}

void USavedPlants::HandleRemoveClicked(int32 TrefleId)
{
	DeleteSavedPlant(TrefleId);
}

void USavedPlants::FetchSavedSpecies()
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

	Api->GetSavedSpecies(
		FBackendPlantsResponse::CreateUObject(this, &USavedPlants::HandleFetchSavedSpeciesResponse)
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
		PlantObject->ImgSrcUrl = Plant.ImgSrcUrl;
		PlantObject->TrefleId = Plant.TrefleId;

		TV_PlantCards->AddItem(PlantObject);
	}
}

void USavedPlants::DeleteSavedPlant(int32 TrefleId)
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
		TrefleId,
		FBackendOperationResponse::CreateLambda(
			[this, TrefleId](bool bSuccess, const FString& Message)
			{
				if (!bSuccess)
				{
					UE_LOG(LogTemp, Error, TEXT("DeleteSavedPlant failed for %d: %s"), TrefleId, *Message);
					return;
				}

				RemoveSavedPlantFromList(TrefleId);
			}
		)
	);
}

void USavedPlants::RemoveSavedPlantFromList(int32 TrefleId)
{
	if (!TV_PlantCards)
	{
		return;
	}

	TArray<UObject*> Items = TV_PlantCards->GetListItems();
	for (UObject* Obj : Items)
	{
		UPlantObject* Plant = Cast<UPlantObject>(Obj);
		if (Plant && Plant->TrefleId == TrefleId)
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