// Fill out your copyright notice in the Description page of Project Settings.

#include "SavedPlants.h"
#include "SavedPlantTile.h"
#include "PlantCardPopup.h"
#include "PlantObject.h"
#include "BackendApiSubsystem.h"
#include "PlantImageCacheSubsystem.h"

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

	PrefetchPlantImages(Plants);
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
		if (!Plant.ImgSrcUrls.Regular.IsEmpty())
		{
			PlantObject->ImgSrcUrl = Plant.ImgSrcUrls.Regular;
		}
		PlantObject->PerenualId = Plant.PerenualId;
		PlantObject->ModelCategory = Plant.ModelCategory;

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

void USavedPlants::PrefetchPlantImages(const TArray<FBackendPlantDto>& Plants)
{
	if (!GetGameInstance())
	{
		return;
	}

	UPlantImageCacheSubsystem* ImageCache = GetGameInstance()->GetSubsystem<UPlantImageCacheSubsystem>();
	if (!ImageCache)
	{
		return;
	}

	for (const FBackendPlantDto& Plant : Plants)
	{
		FString UrlToPrefetch;

		if (!Plant.ImgSrcUrls.Regular.IsEmpty())
		{
			UrlToPrefetch = Plant.ImgSrcUrls.Regular;
		}

		if (UrlToPrefetch.IsEmpty())
		{
			continue;
		}

		ImageCache->GetOrLoadImage(
			UrlToPrefetch,
			FOnPlantImageReady::CreateLambda(
				[Plant](UTexture2D* Texture)
				{
					UE_LOG(
						LogTemp,
						Log,
						TEXT("Prefetch image for %s (%d): %s"),
						*Plant.CommonName,
						Plant.PerenualId,
						Texture ? TEXT("cached") : TEXT("failed")
					);
				}
			)
		);
	}
}