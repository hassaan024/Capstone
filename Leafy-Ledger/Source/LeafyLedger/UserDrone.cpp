#include "UserDrone.h"
#include "Plant.h"
#include "PlantObject.h"
#include "UserDroneController.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "GardenTimeManager.h"
#include "GardenSessionSubsystem.h"

AUserDrone::AUserDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(RootComponent);
	CameraComp->SetRelativeLocation(FVector(0, 0, 290));
	CameraComp->SetRelativeRotation(FRotator(0, -40, 0));
}

void AUserDrone::BeginPlay()
{
	Super::BeginPlay();
	PC = Cast<AUserDroneController>(Controller);
	check(PC);
}

void AUserDrone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MouseWheel", this, &AUserDrone::MouseScroll);

	PlayerInputComponent->BindAction("LeftClick", IE_Pressed, this, &AUserDrone::LeftMousePressed);
	PlayerInputComponent->BindAction("LeftClick", IE_Released, this, &AUserDrone::LeftMouseReleased);
	PlayerInputComponent->BindAction("RightClick", IE_Pressed, this, &AUserDrone::RightMousePressed);
	PlayerInputComponent->BindAction("RightClick", IE_Released, this, &AUserDrone::RightMouseReleased);
}

void AUserDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bRightClickHeld)
	{
		UpdatePan();
	}

	UpdatePreviewPlant();
}

void AUserDrone::SetSelectedPlantData(UPlantObject* InPlantData)
{
	SelectedPlantData = InPlantData;
}

void AUserDrone::UpdatePan()
{
	if (!PC) return;

	FHitResult CurrentHit;
	if (!GetMouseGroundHit(CurrentHit)) return;

	FVector CurrentLoc = CurrentHit.Location;
	const FVector Delta = MouseDragStart - CurrentLoc;
	AddActorWorldOffset(Delta, true);
}

void AUserDrone::UpdatePreviewPlant()
{
	if (!PreviewPlant) return;

	FHitResult Hit;
	if (!GetMouseGroundHit(Hit)) return;

	PreviewPlant->SetActorLocation(Hit.Location);

	if (PreviewPlant->DynamicPreviewMaterial)
	{
		if (ValidPlantPlacement())
		{
			PreviewPlant->DynamicPreviewMaterial->SetVectorParameterValue(TEXT("Color"), FVector(0.288197f, 0.765625f, 0.251979f));
		}
		else
		{
			PreviewPlant->DynamicPreviewMaterial->SetVectorParameterValue(TEXT("Color"), FVector(0.765625f, 0.063621f, 0.078853f));
		}
	}
}

void AUserDrone::LeftMousePressed()
{
	float MouseX, MouseY;
	PC->GetMousePosition(MouseX, MouseY);

	FVector WorldLocation, WorldDirection;
	PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);

	FVector End = WorldLocation + WorldDirection * 100000.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, End, ECC_Pawn, Params);

	if (APlant* Temp = Cast<APlant>(Hit.Actor))
	{
		PreviewPlant = Temp;
		bDraggingRealPlant = true;
		bSelectedPlantSpawned = false;
		DragOriginalTransform = Temp->GetActorTransform();
		return;
	}

	if (PreviewPlant) return;
	if (!SelectedPlantData) return;

	if (SpawnPlant(PreviewPlant))
	{
		bSelectedPlantSpawned = true;
	}
}

void AUserDrone::LeftMouseReleased()
{
	if (!PreviewPlant)
	{
		bDraggingRealPlant = false;
		bSelectedPlantSpawned = false;
		return;
	}

	APlant* FinalizedPlant = PreviewPlant;

	if (ValidPlantPlacement())
	{
		FinalizedPlant->SetPlacedMaterial();
		PreviewPlant = nullptr;

		if (bDraggingRealPlant && FinalizedPlant->bIsTrackedInGardenDraft)
		{
			if (GetGameInstance())
			{
				if (UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>())
				{
					GardenSession->UpdatePlantTransform(
						FinalizedPlant->LocalPlantId,
						FinalizedPlant->GetActorLocation(),
						FinalizedPlant->GetActorRotation(),
						FinalizedPlant->GetActorScale3D()
					);
				}
			}
		}
		else
		{
			TrackPlacedPlant(FinalizedPlant);
		}
	}
	else
	{
		if (bDraggingRealPlant)
		{
			FinalizedPlant->SetActorTransform(DragOriginalTransform);
		}
		else
		{
			FinalizedPlant->Destroy();
		}

		PreviewPlant = nullptr;
	}

	bDraggingRealPlant = false;
	bSelectedPlantSpawned = false;
}

void AUserDrone::RightMousePressed()
{
	bRightClickHeld = true;

	FHitResult Temp;
	if (GetMouseGroundHit(Temp)) MouseDragStart = Temp.Location;
}

void AUserDrone::RightMouseReleased()
{
	bRightClickHeld = false;
}

void AUserDrone::MouseScroll(float Axis)
{
	if (Axis > 0)
	{
		FVector NewLoc = GetActorLocation();
		FVector CameraVector = CameraComp->GetForwardVector();
		NewLoc += CameraVector * 50;
		SetActorLocation(NewLoc);
	}
	else if (Axis < 0)
	{
		FVector NewLoc = GetActorLocation();
		FVector CameraVector = CameraComp->GetForwardVector();
		NewLoc -= CameraVector * 50;
		SetActorLocation(NewLoc);
	}
}

bool AUserDrone::GetMouseGroundHit(FHitResult& OutHit)
{
	float MouseX, MouseY;
	PC->GetMousePosition(MouseX, MouseY);

	FVector WorldLocation, WorldDirection;
	PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);

	FVector End = WorldLocation + WorldDirection * 100000.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(this);
	if (PreviewPlant) Params.AddIgnoredActor(PreviewPlant);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, End, ECC_WorldStatic, Params);
	if (!bHit) return false;

	OutHit = Hit;
	return true;
}

bool AUserDrone::ValidPlantPlacement()
{
	FHitResult GroundHit;
	if (GetMouseGroundHit(GroundHit))
	{
		if (GroundHit.PhysMaterial.IsValid())
		{
			UPhysicalMaterial* PhysMat = GroundHit.PhysMaterial.Get();
			if (*PhysMat->GetName() == FName("DirtPhysMat")) return true;
		}
	}
	return false;
}

bool AUserDrone::SpawnPlant(APlant*& Plant)
{
	if (!SelectedPlantData) return false;
	if (!ValidPlantPlacement()) return false;

	FVector MouseWorldLocation;
	FVector MouseWorldDirection;

	if (!PC->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection)) return false;

	FVector StartLoc = MouseWorldLocation;
	FVector EndLoc = StartLoc + MouseWorldDirection * 10000.f;

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_WorldStatic, CollisionParams);
	if (!bHit) return false;

	TSubclassOf<APlant> ClassToSpawn = DefaultPlantClass;
	if (SelectedPlantData->PlantClass)
	{
		ClassToSpawn = SelectedPlantData->PlantClass;
	}

	if (!ClassToSpawn) return false;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = this;

	Plant = GetWorld()->SpawnActor<APlant>(ClassToSpawn, HitResult.Location, FRotator::ZeroRotator, SpawnParams);
	if (!Plant) return false;

	Plant->InitializeFromPlantData(SelectedPlantData);

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGardenTimeManager::StaticClass(), Found);
	if (Found.Num() > 0)
	{
		AGardenTimeManager* TM = Cast<AGardenTimeManager>(Found[0]);
		if (TM)
		{
			Plant->BloomDayIndex = TM->GetCurrentDayIndex();
			Plant->PlantingDayIndex = Plant->BloomDayIndex - Plant->DaysToBloom;
			Plant->WitherDayIndex = Plant->BloomDayIndex + Plant->DaysToWither;
			Plant->UpdateForDay(TM->GetCurrentDayIndex());
		}
	}

	return true;
}

void AUserDrone::TrackPlacedPlant(APlant* PlantActor)
{
	if (!PlantActor) return;

	if (!GetGameInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackPlacedPlant failed: GameInstance is null"));
		return;
	}

	UGardenSessionSubsystem* GardenSession =
		GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();

	if (!GardenSession)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackPlacedPlant failed: GardenSessionSubsystem missing"));
		return;
	}

	if (!GardenSession->HasActiveDraft())
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackPlacedPlant failed: no active garden draft"));
		return;
	}

	if (PlantActor->PerenualId <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackPlacedPlant failed: PlantActor->PerenualId is invalid (%d)"), PlantActor->PerenualId);
		return;
	}

	const int32 SoilId = 1;

	const FGuid LocalId = GardenSession->AddPlantPlacement(
		PlantActor->PerenualId,
		SoilId,
		PlantActor->GetActorLocation(),
		PlantActor->GetActorRotation(),
		PlantActor->GetActorScale3D()
	);

	const FEditableGardenState& Draft = GardenSession->GetDraft();
	UE_LOG(LogTemp, Error, TEXT("Draft now has %d plant(s)"), Draft.Plants.Num());

	if (!LocalId.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackPlacedPlant failed: AddPlantPlacement returned invalid LocalId"));
		return;
	}

	PlantActor->LocalPlantId = LocalId;
	PlantActor->bIsTrackedInGardenDraft = true;

	UE_LOG(LogTemp, Log, TEXT("Tracked placed plant. LocalId=%s PerenualId=%d"), *LocalId.ToString(), PlantActor->PerenualId);
}