// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void PanUp(float Value);
	void PanRight(float Value);
	void LeftMouse();
	void RightMousePressed();
	void RightMouseReleased();
	//void MouseScroll(float Axis);

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<APlant> SelectedPlant;
	AUserDroneController* PC;
	bool bRightClickHeld = false;
	FVector MouseDragStart;

	bool GetMouseGroundHit(FVector& OutVector);

	void UpdatePan();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
