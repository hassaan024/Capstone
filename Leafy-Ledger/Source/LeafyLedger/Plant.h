// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Plant.generated.h"


class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;


UCLASS()
class LEAFYLEDGER_API APlant : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PreviewMesh;
	
public:	
	// Sets default values for this actor's properties
	APlant();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UMaterialInterface* PreviewMaterial;
	UMaterialInstanceDynamic* DynamicPreviewMaterial;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
		FVector InvalidPlacementColor;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
		FVector ValidPlacementColor;

	UFUNCTION(BlueprintImplementableEvent)
		void SetPlacedMaterial();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 PlantedDayIndex;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 DaysToBloom;

	UFUNCTION()
		void HandleDayChanged(int32 NewDayIndex);

	void UpdateForDay(int32 DayIndex);
};
