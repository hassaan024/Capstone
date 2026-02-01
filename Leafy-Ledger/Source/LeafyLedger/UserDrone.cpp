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
	if (bRightClickHeld) {
		UpdatePan();
	}
}

void AUserDrone::UpdatePan() {
	if (!PC) return;

	FVector CurrentHit;
	if (!GetMouseGroundHit(CurrentHit))
	{
		// If we lose the surface for a frame, do nothing.
		return;
	}

	const FVector Delta = MouseDragStart - CurrentHit;

	AddActorWorldOffset(Delta, true); // sweep=true is optional; can help prevent clipping
	// IMPORTANT: do NOT refresh MouseDragStart here
}

// Called to bind functionality to input
void AUserDrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("PanUp", this, &AUserDrone::PanUp);
	PlayerInputComponent->BindAxis("PanRight", this, &AUserDrone::PanRight);
	PlayerInputComponent->BindAxis("MouseWheel", this, &AUserDrone::MouseScroll);

	PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &AUserDrone::LeftMouse);
	PlayerInputComponent->BindAction("RightClick", IE_Pressed, this, &AUserDrone::RightMousePressed);
	PlayerInputComponent->BindAction("RightClick", IE_Released, this, &AUserDrone::RightMouseReleased);
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

void AUserDrone::RightMousePressed() {
	// Mark our rmb flag
	bRightClickHeld = true;

	// And grab our start loc
	FVector Temp = FVector(0, 0, 0);
	if (GetMouseGroundHit(Temp)) MouseDragStart = Temp;
}

void AUserDrone::RightMouseReleased() {
	bRightClickHeld = false;
}

void AUserDrone::PanUp(float Value) {
	//if (bRightClickHeld) {
	//	SetActorLocation(GetActorLocation() + GetActorForwardVector() * Value * 10);
	//}
}

void AUserDrone::PanRight(float Value) {
	//if (bRightClickHeld) {
	//	SetActorLocation(GetActorLocation() + GetActorRightVector() * Value * 10);
	//}
}

bool AUserDrone::GetMouseGroundHit(FVector& OutVector) {
	float MouseX, MouseY;
	PC->GetMousePosition(MouseX, MouseY);

	FVector WorldLocation, WorldDirection;
	PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);

	// trace down to ground plane (or terrain)
	FVector End = WorldLocation + WorldDirection * 100000.f;

	FHitResult Hit;
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, End, ECC_WorldStatic);

	if (!bHit) {
		OutVector = FVector(0, 0, 0);
		return false;
	}

	OutVector = Hit.Location;
	return bHit;

	// Fallback: intersect with a plane (e.g., Z=0)
	const float GroundZ = 0.f;
	const FPlane GroundPlane(FVector::UpVector, GroundZ);

	const FVector PlanePoint = FMath::LinePlaneIntersection(WorldLocation, End, GroundPlane);

	// Optional: reject if the ray is almost parallel to the plane
	if (FMath::Abs(WorldDirection.Z) < KINDA_SMALL_NUMBER)
		return false;

	OutVector = PlanePoint;
	return true;
}

