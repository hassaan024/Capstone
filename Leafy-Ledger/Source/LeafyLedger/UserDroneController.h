// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UserDroneController.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API AUserDroneController : public APlayerController
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUserDroneController();

protected:
	virtual void BeginPlay() override;
};
