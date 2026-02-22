// Fill out your copyright notice in the Description page of Project Settings.


#include "UserDrone.h"
#include "Plant.h"
#include "UserDroneController.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

#pragma region Setup
// Sets default values
AUserDrone::AUserDrone()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(RootComponent);
	CameraComp->SetRelativeLocation(FVector(0, 0, 290));
	CameraComp->SetRelativeRotation(FRotator(0, -40, 0));
}

// Called when the game starts or when spawned
void AUserDrone::BeginPlay()
{
	Super::BeginPlay();
	PC = Cast<AUserDroneController>(Controller);
	check(PC);
}

// Called to bind functionality to input
void AUserDrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MouseWheel", this, &AUserDrone::MouseScroll);

	PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &AUserDrone::LeftMousePressed);
	PlayerInputComponent->BindAction("LeftClick", IE_Released, this, &AUserDrone::LeftMouseReleased);
	PlayerInputComponent->BindAction("RightClick", IE_Pressed, this, &AUserDrone::RightMousePressed);
	PlayerInputComponent->BindAction("RightClick", IE_Released, this, &AUserDrone::RightMouseReleased);
}
#pragma endregion

#pragma region Tick Functions
// Called every frame
void AUserDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bRightClickHeld) {
		UpdatePan();
	}
	if (SelectedPlant) UE_LOG(LogTemp, Warning, TEXT("Plant is selected"));
	ValidPlantPlacement();
}

void AUserDrone::UpdatePan() {
	if (!PC) return;

	FHitResult CurrentHit;
	if (!GetMouseGroundHit(CurrentHit))
	{
		// If we lose the surface for a frame, do nothing.
		return;
	}
	FVector CurrentLoc = CurrentHit.Location;
	const FVector Delta = MouseDragStart - CurrentLoc;

	AddActorWorldOffset(Delta, true); // sweep=true is optional; can help prevent clipping
	// IMPORTANT: do NOT refresh MouseDragStart here
}
#pragma endregion

#pragma region Base Input
void AUserDrone::LeftMousePressed() {
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

void AUserDrone::LeftMouseReleased() {
	if (SelectedPlant) {
		UE_LOG(LogTemp, Warning, TEXT("blah blah"));
		SelectedPlant = NULL;
	}
}

void AUserDrone::RightMousePressed() {
	// Mark our rmb flag
	bRightClickHeld = true;

	// And grab our start loc
	FHitResult Temp;
	if (GetMouseGroundHit(Temp)) MouseDragStart = Temp.Location;
}

void AUserDrone::RightMouseReleased() {
	bRightClickHeld = false;
}

void AUserDrone::MouseScroll(float Axis) {
	if (Axis > 0) {
		FVector NewLoc = GetActorLocation();
		FVector CameraVector = CameraComp->GetForwardVector();
		NewLoc += CameraVector * 50;
		SetActorLocation(NewLoc);
	}
	else if (Axis < 0) {
		FVector NewLoc = GetActorLocation();
		FVector CameraVector = CameraComp->GetForwardVector();
		NewLoc -= CameraVector * 50;
		SetActorLocation(NewLoc);
	}
}
#pragma endregion

#pragma region Helper Functions
bool AUserDrone::GetMouseGroundHit(FHitResult& OutHit) {
	float MouseX, MouseY;
	PC->GetMousePosition(MouseX, MouseY);

	FVector WorldLocation, WorldDirection;
	PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);

	// trace down to ground plane (or terrain)
	FVector End = WorldLocation + WorldDirection * 100000.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(this);
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, End, ECC_WorldStatic, Params);

	if (!bHit) {
		return false;
	}

	OutHit = Hit;
	return bHit;
}

bool AUserDrone::ValidPlantPlacement() {
	FHitResult GroundHit;
	if (GetMouseGroundHit(GroundHit)) {
		if (GroundHit.PhysMaterial.IsValid()) {
			UPhysicalMaterial* PhysMat = GroundHit.PhysMaterial.Get();
			UE_LOG(LogTemp, Warning, TEXT("Phys material name is: %s"), *PhysMat->GetName());
		}
	}
	return true;
}
#pragma endregion

