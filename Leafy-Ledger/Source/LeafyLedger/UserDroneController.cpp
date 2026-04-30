// Fill out your copyright notice in the Description page of Project Settings.


#include "UserDroneController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/EngineTypes.h"

AUserDroneController::AUserDroneController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
}

void AUserDroneController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	UGameplayStatics::SetViewportMouseCaptureMode(GetWorld(), EMouseCaptureMode::CaptureDuringMouseDown);
}