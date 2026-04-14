// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GardenDirector.generated.h"

class UPlantSelect;
class UGardenExit;
struct FEditablePlantPlacement;

UCLASS()
class LEAFYLEDGER_API AGardenDirector : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGardenDirector();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION()
	void MakePlantList();

	UFUNCTION()
	void AddButtons();

	UFUNCTION()
	void SpawnLoadedPlants();

	UFUNCTION(BlueprintImplementableEvent)
	void GrabPlants();

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UPlantSelect> PlantSelectClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UGardenExit> GardenExitClass;
};
