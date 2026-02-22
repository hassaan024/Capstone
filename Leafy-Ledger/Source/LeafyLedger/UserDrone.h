// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "UserDrone.generated.h"

class APlant;
class AUserDroneController;

UCLASS()
class LEAFYLEDGER_API AUserDrone : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUserDrone();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UCameraComponent* CameraComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

#pragma region Base Input
private:
	void LeftMousePressed();
	void LeftMouseReleased();
	void RightMousePressed();
	void RightMouseReleased();
	void MouseScroll(float Axis);
#pragma endregion
	
#pragma region Real-Time Variables
protected:
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<APlant> SelectedPlant;
private:
	AUserDroneController* PC;
	bool bRightClickHeld = false;
	FVector MouseDragStart;
#pragma endregion

#pragma region Tick Functions
private:
	void UpdatePan();
#pragma endregion

#pragma region Helper Functions
private:
	bool GetMouseGroundHit(FHitResult& OutHit);
	bool ValidPlantPlacement();
#pragma endregion


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
