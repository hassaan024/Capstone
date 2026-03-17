// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Components/TileView.h"
#include "Http.h"
#include "OAuthGISubsystem.h"
#include "SavedPlants.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API USavedPlants : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Plants")
	void DeleteSavedPlant(int32 TrefleId);

	UFUNCTION(BlueprintCallable, Category = "Plants")
	void RemoveSavedPlantFromList(int32 TrefleId);

	UFUNCTION()
	void HandleRemoveClicked(int32 TrefleId);

	void HandleEntryGenerated(UUserWidget& EntryWidget);

	// Bind this in the UMG designer by checking "Is Variable"
	UPROPERTY(meta = (BindWidget))
	UTileView* TV_PlantCards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backend")
	FString BackendBaseUrl = TEXT("http://localhost:4000/backend");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backend")
	int32 UserId = 0;

protected:
	virtual void NativeConstruct() override;

private:
	void FetchSavedSpecies();
	void OnFetchSavedSpeciesComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
