// Fill out your copyright notice in the Description page of Project Settings.


#include "UserDrone.h"

// Sets default values
AUserDrone::AUserDrone()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUserDrone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AUserDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AUserDrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Horizontal Movement bindings
	PlayerInputComponent->BindAxis("MoveForward", this, &AUserDrone::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AUserDrone::MoveRight);
}

void AUserDrone::MoveForward(float Value) {
	if (Value != 0.f) {
		AddMovementInput(FVector(1,0,0), Value);
	}
}

void AUserDrone::MoveRight(float Value) {
	if (Value != 0.f) {
		AddMovementInput(FVector(0, 1, 0), Value);
	}
}

