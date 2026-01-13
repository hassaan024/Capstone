// Fill out your copyright notice in the Description page of Project Settings.


#include "UserDrone.h"
#include "Plant.h"
#include "UserDroneController.h"

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
	PC = Cast<AUserDroneController>(Controller);
	check(PC);
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

	PlayerInputComponent->BindAction("Click", IE_Pressed, this, &AUserDrone::LeftMouse);
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

void AUserDrone::LeftMouse() {
	if (SelectedPlant) {
		UE_LOG(LogTemp, Warning, TEXT("valid plant"));
		FVector MouseWorldLocation;
		FVector MouseWorldDirection;

		if (!PC->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection)) return;

		FVector StartLoc = MouseWorldLocation;
		FVector EndLoc = StartLoc + MouseWorldDirection * 10000;

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_WorldStatic, CollisionParams);

		if (bHit) {
			FActorSpawnParameters SpawnParams;
			SpawnParams.Instigator = this;
			GetWorld()->SpawnActor<APlant>(SelectedPlant, HitResult.Location, FRotator(0, 0, 0), SpawnParams);
		}
	}
}

// This function treats -99999, -9999, -999 as a miss
FVector AUserDrone::GetMouseRaycast() {
	/*FVector MouseWorldLocation;
	FVector MouseWorldDirection;

	if (!PC->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection)) return FVector(-99999, -9999, -999);
	
	FVector StartLoc = MouseWorldLocation;
	FVector EndLoc = StartLoc + MouseWorldDirection * 10000;

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_WorldStatic, CollisionParams);

	if (bHit) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = this;
		GetWorld()->SpawnActor<APlant>(SelectedPlant, HitResult.Location, FRotator(0,0,0), SpawnParams);
	}*/
	return FVector(0, 0, 0);
}

