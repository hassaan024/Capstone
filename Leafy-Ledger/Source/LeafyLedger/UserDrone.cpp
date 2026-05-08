#include "UserDrone.h"
#include "BloomDateUtils.h"
#include "GardenHUD.h"
#include "Plant.h"
#include "PlantObject.h"
#include "UserDroneController.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "GardenTimeManager.h"
#include "GardenSessionSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeProxy.h"
#include "LandscapeLayerInfoObject.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

AUserDrone::AUserDrone()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(RootComponent);
	CameraComp->SetRelativeLocation(FVector(0, 0, 290));
	CameraComp->SetRelativeRotation(FRotator(0, -40, 0));

	static ConstructorHelpers::FObjectFinder<ULandscapeLayerInfoObject> DirtLayerFinder(TEXT("/Game/Garden_sharedassets/Dirt_LayerInfo.Dirt_LayerInfo"));
	if (DirtLayerFinder.Succeeded())
	{
		DirtLayerInfo = DirtLayerFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<ULandscapeLayerInfoObject> GrassLayerFinder(TEXT("/Game/Garden_sharedassets/Grass_LayerInfo.Grass_LayerInfo"));
	if (GrassLayerFinder.Succeeded())
	{
		GrassLayerInfo = GrassLayerFinder.Object;
	}

	static ConstructorHelpers::FClassFinder<APlant> DefaultPlantFinder(TEXT("/Game/BP_BasePlant"));
	if (DefaultPlantFinder.Succeeded())
	{
		DefaultPlantClass = DefaultPlantFinder.Class;
	}
}

void AUserDrone::BeginPlay()
{
	Super::BeginPlay();
	PC = Cast<AUserDroneController>(Controller);
	check(PC);
	DroneMovementOrigin = GetActorLocation();
	EnsureDefaultPaintLayersLoaded();
	InitializeRuntimePaint();
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

	if (bLeftPaintHeld && CurrentEditMode == EGardenEditMode::Paint)
	{
		PaintSelectedLandscapeLayer();
	}

	UpdatePaintBrushPreview();
	UpdatePreviewPlant();
}

void AUserDrone::SetSelectedPlantData(UPlantObject* InPlantData)
{
	if (!InPlantData || InPlantData->bIsDropdownToggle)
	{
		return;
	}

	if (SelectedPlantData != InPlantData)
	{
		CancelActivePlantInteraction();
	}

	SelectedPlantData = InPlantData;
	if (CurrentEditMode == EGardenEditMode::Plant || !HasPredictedPlantingDates())
	{
		CurrentEditMode = EGardenEditMode::Plant;
	}
	bLeftPaintHeld = false;
}

bool AUserDrone::IsPlantDataSelected(const UPlantObject* PlantData) const
{
	return PlantData && SelectedPlantData == PlantData;
}

void AUserDrone::UpdatePan()
{
	if (!PC) return;

	FHitResult CurrentHit;
	if (!GetMouseGroundHit(CurrentHit)) return;

	FVector CurrentLoc = CurrentHit.Location;
	FVector Delta = MouseDragStart - CurrentLoc;
	Delta.Z = 0.0f;
	MoveDroneTo(GetActorLocation() + Delta, true);
}

void AUserDrone::UpdatePreviewPlant()
{
	if (IsGardenModificationBlocked())
	{
		CancelActivePlantInteraction();
		return;
	}

	if (CurrentEditMode != EGardenEditMode::Plant)
	{
		return;
	}

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

void AUserDrone::UpdatePaintBrushPreview()
{
	if (CurrentEditMode != EGardenEditMode::Paint || !GetWorld())
	{
		return;
	}

	FHitResult GroundHit;
	if (!GetMousePaintHit(GroundHit))
	{
		return;
	}

	const FVector Center = GroundHit.Location + (GroundHit.ImpactNormal * 6.f);
	const FQuat CircleRotation = FRotationMatrix::MakeFromZ(GroundHit.ImpactNormal).ToQuat();
	const FColor BrushColor = SelectedPaintLayerInfo == GrassLayerInfo ? FColor(60, 220, 80) : FColor(150, 95, 45);

	DrawDebugCircle(
		GetWorld(),
		Center,
		PaintBrushRadius,
		96,
		BrushColor,
		false,
		0.f,
		0,
		4.f,
		FVector(1.f, 0.f, 0.f),
		FVector(0.f, 1.f, 0.f),
		false
	);

	DrawDebugSphere(GetWorld(), Center, 10.f, 12, BrushColor, false, 0.f, 0, 2.f);
}

FVector AUserDrone::ClampDroneLocation(const FVector& DesiredLocation) const
{
	if (!bLimitDroneMovement)
	{
		return DesiredLocation;
	}

	FVector ClampedLocation = DesiredLocation;
	const FVector2D Origin2D(DroneMovementOrigin.X, DroneMovementOrigin.Y);
	const FVector2D Desired2D(DesiredLocation.X, DesiredLocation.Y);
	FVector2D Offset2D = Desired2D - Origin2D;
	const float MaxDistance = FMath::Max(0.f, MaxDronePlanarDistance);

	if (MaxDistance > 0.f && Offset2D.SizeSquared() > FMath::Square(MaxDistance))
	{
		Offset2D = Offset2D.GetSafeNormal() * MaxDistance;
		ClampedLocation.X = DroneMovementOrigin.X + Offset2D.X;
		ClampedLocation.Y = DroneMovementOrigin.Y + Offset2D.Y;
	}

	const float MinHeight = DroneMovementOrigin.Z + FMath::Min(MinDroneRelativeHeight, MaxDroneRelativeHeight);
	const float MaxHeight = DroneMovementOrigin.Z + FMath::Max(MinDroneRelativeHeight, MaxDroneRelativeHeight);
	ClampedLocation.Z = FMath::Clamp(DesiredLocation.Z, MinHeight, MaxHeight);
	return ClampedLocation;
}

void AUserDrone::MoveDroneTo(const FVector& DesiredLocation, bool bSweep)
{
	SetActorLocation(ClampDroneLocation(DesiredLocation), bSweep);
}
void AUserDrone::LeftMousePressed()
{
	if (IsGardenModificationBlocked())
	{
		CancelActivePlantInteraction();
		return;
	}

	if (CurrentEditMode == EGardenEditMode::Delete)
	{
		FHitResult PlantHit;
		if (GetMousePlantHit(PlantHit))
		{
			DeletePlant(Cast<APlant>(PlantHit.Actor));
		}
		return;
	}

	if (CurrentEditMode == EGardenEditMode::Paint)
	{
		bLeftPaintHeld = true;
		PaintSelectedLandscapeLayer();
		return;
	}

	if (CurrentEditMode != EGardenEditMode::Plant)
	{
		return;
	}

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
	bLeftPaintHeld = false;

	if (IsGardenModificationBlocked())
	{
		CancelActivePlantInteraction();
		return;
	}

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
		MoveDroneTo(NewLoc, false);
	}
	else if (Axis < 0)
	{
		FVector NewLoc = GetActorLocation();
		FVector CameraVector = CameraComp->GetForwardVector();
		NewLoc -= CameraVector * 50;
		MoveDroneTo(NewLoc, false);
	}
}

bool AUserDrone::GetMouseGroundHit(FHitResult& OutHit)
{
	if (!PC || !GetWorld())
	{
		return false;
	}

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

	TArray<AActor*> FoundPlants;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlant::StaticClass(), FoundPlants);
	for (AActor* PlantActor : FoundPlants)
	{
		Params.AddIgnoredActor(PlantActor);
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, End, ECC_WorldStatic, Params);
	if (!bHit) return false;

	OutHit = Hit;
	return true;
}

bool AUserDrone::GetMousePaintHit(FHitResult& OutHit)
{
	if (!PC || !GetWorld())
	{
		return false;
	}

	FHitResult CursorHit;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit))
	{
		if (!Cast<APlant>(CursorHit.Actor))
		{
			OutHit = CursorHit;
			return true;
		}
	}

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

	TArray<AActor*> FoundPlants;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlant::StaticClass(), FoundPlants);
	for (AActor* PlantActor : FoundPlants)
	{
		Params.AddIgnoredActor(PlantActor);
	}

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, End, ECC_WorldStatic, Params);
	if (!bHit)
	{
		bHit = GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, End, ECC_Visibility, Params);
	}
	if (!bHit) return false;

	OutHit = Hit;
	return true;
}

bool AUserDrone::GetMousePlantHit(FHitResult& OutHit)
{
	if (!PC || !GetWorld())
	{
		return false;
	}

	float MouseX, MouseY;
	PC->GetMousePosition(MouseX, MouseY);

	FVector WorldLocation, WorldDirection;
	PC->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection);

	const FVector End = WorldLocation + WorldDirection * 100000.f;

	FCollisionQueryParams Params;
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(this);
	if (PreviewPlant)
	{
		Params.AddIgnoredActor(PreviewPlant);
	}

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, End, ECC_Pawn, Params);
	if (!bHit || !Cast<APlant>(Hit.Actor))
	{
		return false;
	}

	OutHit = Hit;
	return true;
}

bool AUserDrone::ValidPlantPlacement()
{
	FHitResult GroundHit;
	if (GetMouseGroundHit(GroundHit))
	{
		if (CurrentPaintMask && RuntimePaintMaskValues.Num() > 0)
		{
			return IsRuntimePaintLocationPlantable(GroundHit);
		}

		if (GroundHit.PhysMaterial.IsValid())
		{
			UPhysicalMaterial* PhysMat = GroundHit.PhysMaterial.Get();
			if (*PhysMat->GetName() == FName("DirtPhysMat")) return true;
		}
	}
	return false;
}

void AUserDrone::PaintSelectedLandscapeLayer()
{
	if (IsGardenModificationBlocked())
	{
		return;
	}

	EnsureDefaultPaintLayersLoaded();

	if (!SelectedPaintLayerInfo)
	{
		SelectedPaintLayerInfo = DirtLayerInfo ? DirtLayerInfo : GrassLayerInfo;
	}

	if (!SelectedPaintLayerInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("PaintSelectedLandscapeLayer: no paint layer selected"));
		return;
	}

	FHitResult GroundHit;
	if (!GetMousePaintHit(GroundHit))
	{
		UE_LOG(LogTemp, Warning, TEXT("PaintSelectedLandscapeLayer: no ground hit"));
		return;
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("PaintSelectedLandscapeLayer: hit actor=%s component=%s layer=%s location=%s"),
		GroundHit.GetActor() ? *GroundHit.GetActor()->GetName() : TEXT("None"),
		GroundHit.GetComponent() ? *GroundHit.GetComponent()->GetName() : TEXT("None"),
		*SelectedPaintLayerInfo->LayerName.ToString(),
		*GroundHit.Location.ToString()
	);

	if (!PaintLandscapeAtHit(GroundHit, SelectedPaintLayerInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("PaintSelectedLandscapeLayer: paint failed for %s"), *SelectedPaintLayerInfo->LayerName.ToString());
	}
}

bool AUserDrone::PaintLandscapeAtHit(const FHitResult& LandscapeHit, ULandscapeLayerInfoObject* LayerInfo)
{
	if (!LayerInfo || !GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("PaintLandscapeAtHit: missing layer or world"));
		return false;
	}

	ALandscapeProxy* Landscape = Cast<ALandscapeProxy>(LandscapeHit.GetActor());
	if (!Landscape)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("PaintLandscapeAtHit: cursor is not over a landscape. HitActor=%s"),
			LandscapeHit.GetActor() ? *LandscapeHit.GetActor()->GetName() : TEXT("None")
		);
		return false;
	}

	if (!PaintMaskA || !PaintMaskB || !PaintBrushMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("PaintLandscapeAtHit: assign PaintMaskA, PaintMaskB, and PaintBrushMaterial on BP_UserDrone"));
		return false;
	}

	if (!PaintBrushMID)
	{
		PaintBrushMID = UMaterialInstanceDynamic::Create(PaintBrushMaterial, this);
	}

	if (!PaintBrushMID)
	{
		UE_LOG(LogTemp, Warning, TEXT("PaintLandscapeAtHit: failed to create paint brush MID"));
		return false;
	}

	if (!CurrentPaintMask || !NextPaintMask)
	{
		InitializeRuntimePaint();
	}

	if (!CurrentPaintMask || !NextPaintMask)
	{
		return false;
	}

	FVector2D PaintUV;
	float NormalizedBrushRadius = 0.f;
	if (!GetLandscapePaintUV(Landscape, LandscapeHit.Location, PaintUV, NormalizedBrushRadius))
	{
		UE_LOG(LogTemp, Warning, TEXT("PaintLandscapeAtHit: failed to convert hit to paint UV"));
		return false;
	}

	const bool bPaintingGrass = LayerInfo == GrassLayerInfo || LayerInfo->LayerName == TEXT("Grass");
	const float PaintValue = bPaintingGrass ? 1.f : 0.f;

	PaintBrushMID->SetTextureParameterValue(BrushPreviousMaskParameterName, CurrentPaintMask);
	PaintBrushMID->SetVectorParameterValue(BrushCenterParameterName, FLinearColor(PaintUV.X, PaintUV.Y, 0.f, 0.f));
	PaintBrushMID->SetScalarParameterValue(BrushRadiusParameterName, NormalizedBrushRadius);
	PaintBrushMID->SetScalarParameterValue(BrushPaintValueParameterName, PaintValue);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, NextPaintMask, PaintBrushMID);
	AdvancePaintRenderTargetsAfterDraw();
	PaintRuntimeMaskValuesAt(PaintUV, NormalizedBrushRadius, bPaintingGrass);
	SaveRuntimePaintMaskToDraft();
	ApplyCurrentPaintMaskToLandscape(Landscape);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("Painted %s runtime mask at UV %.3f,%.3f radius %.4f"),
		bPaintingGrass ? TEXT("Grass") : TEXT("Dirt"),
		PaintUV.X,
		PaintUV.Y,
		NormalizedBrushRadius
	);
	return true;
}

void AUserDrone::InitializeRuntimePaint()
{
	if (!PaintMaskA || !PaintMaskB)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeRuntimePaint: missing paint render target(s). PaintMaskA=%s PaintMaskB=%s"), PaintMaskA ? *PaintMaskA->GetName() : TEXT("None"), PaintMaskB ? *PaintMaskB->GetName() : TEXT("None"));
		return;
	}

	CurrentPaintMask = PaintMaskA;
	NextPaintMask = PaintMaskB;

	UKismetRenderingLibrary::ClearRenderTarget2D(this, PaintMaskA, FLinearColor::Black);
	UKismetRenderingLibrary::ClearRenderTarget2D(this, PaintMaskB, FLinearColor::Black);
	InitializeDefaultRuntimePaintMaskValues();

	if (PaintBrushMaterial && !PaintBrushMID)
	{
		PaintBrushMID = UMaterialInstanceDynamic::Create(PaintBrushMaterial, this);
	}

	bool bHasActiveDraft = false;
	bool bDraftHasSavedPaintMask = false;
	bool bLoadedSavedPaintMask = false;
	if (GetGameInstance())
	{
		if (UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>())
		{
			bHasActiveDraft = GardenSession->HasActiveDraft();
			const FEditableGardenState& Draft = GardenSession->GetDraft();
			bDraftHasSavedPaintMask = !Draft.PaintMaskData.IsEmpty();
			if (bDraftHasSavedPaintMask)
			{
				bLoadedSavedPaintMask = ImportPaintMaskData(Draft.PaintMaskData);
			}
		}
	}

	if (!bLoadedSavedPaintMask)
	{
		RuntimePaintMaskTexture = CreateTextureFromRuntimePaintMask();
		if (RuntimePaintMaskTexture)
		{
			CurrentPaintMask = RuntimePaintMaskTexture;
		}
		NextPaintMask = PaintMaskA;
		SaveRuntimePaintMaskToDraft();
	}

	UE_LOG(
		LogTemp,
		Log,
		TEXT("InitializeRuntimePaint v2 CPUDefault: activeDraft=%d savedMask=%d loadedSaved=%d currentMask=%s nextMask=%s defaultRadius=%.3f transientTexture=%d"),
		bHasActiveDraft ? 1 : 0,
		bDraftHasSavedPaintMask ? 1 : 0,
		bLoadedSavedPaintMask ? 1 : 0,
		CurrentPaintMask ? *CurrentPaintMask->GetName() : TEXT("None"),
		NextPaintMask ? *NextPaintMask->GetName() : TEXT("None"),
		DefaultDirtCircleRadius,
		RuntimePaintMaskTexture ? 1 : 0
	);

	TArray<AActor*> FoundLandscapes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscapeProxy::StaticClass(), FoundLandscapes);
	for (AActor* Actor : FoundLandscapes)
	{
		if (ALandscapeProxy* Landscape = Cast<ALandscapeProxy>(Actor))
		{
			ApplyCurrentPaintMaskToLandscape(Landscape);
		}
	}
}

void AUserDrone::InitializeRuntimePaintMaskValues()
{
	RuntimePaintMaskValues.Reset();
	RuntimePaintMaskWidth = 0;
	RuntimePaintMaskHeight = 0;

	if (!PaintMaskA)
	{
		return;
	}

	RuntimePaintMaskWidth = PaintMaskA->SizeX;
	RuntimePaintMaskHeight = PaintMaskA->SizeY;

	if (RuntimePaintMaskWidth <= 0 || RuntimePaintMaskHeight <= 0)
	{
		return;
	}

	RuntimePaintMaskValues.Init(0, RuntimePaintMaskWidth * RuntimePaintMaskHeight);
}

void AUserDrone::InitializeDefaultRuntimePaintMaskValues()
{
	InitializeRuntimePaintMaskValues();
	if (RuntimePaintMaskValues.Num() == 0 || RuntimePaintMaskWidth <= 0 || RuntimePaintMaskHeight <= 0)
	{
		return;
	}

	RuntimePaintMaskValues.Init(255, RuntimePaintMaskWidth * RuntimePaintMaskHeight);

	const FVector2D CircleCenter(
		FMath::Clamp(DefaultDirtCircleCenter.X, 0.f, 1.f),
		FMath::Clamp(DefaultDirtCircleCenter.Y, 0.f, 1.f)
	);
	const float CircleRadius = FMath::Clamp(DefaultDirtCircleRadius, 0.f, 1.f);
	const float RadiusSquared = FMath::Square(CircleRadius);

	for (int32 Y = 0; Y < RuntimePaintMaskHeight; ++Y)
	{
		const float V = RuntimePaintMaskHeight > 1 ? static_cast<float>(Y) / static_cast<float>(RuntimePaintMaskHeight - 1) : 0.f;
		for (int32 X = 0; X < RuntimePaintMaskWidth; ++X)
		{
			const float U = RuntimePaintMaskWidth > 1 ? static_cast<float>(X) / static_cast<float>(RuntimePaintMaskWidth - 1) : 0.f;
			if (FVector2D::DistSquared(FVector2D(U, V), CircleCenter) <= RadiusSquared)
			{
				RuntimePaintMaskValues[(Y * RuntimePaintMaskWidth) + X] = 0;
			}
		}
	}
}
void AUserDrone::PaintRuntimeMaskValuesAt(const FVector2D& PaintUV, float NormalizedBrushRadius, bool bPaintingGrass)
{
	if (RuntimePaintMaskValues.Num() == 0 || RuntimePaintMaskWidth <= 0 || RuntimePaintMaskHeight <= 0)
	{
		InitializeRuntimePaintMaskValues();
	}

	if (RuntimePaintMaskValues.Num() == 0)
	{
		return;
	}

	const int32 CenterX = FMath::Clamp(FMath::RoundToInt(PaintUV.X * static_cast<float>(RuntimePaintMaskWidth - 1)), 0, RuntimePaintMaskWidth - 1);
	const int32 CenterY = FMath::Clamp(FMath::RoundToInt(PaintUV.Y * static_cast<float>(RuntimePaintMaskHeight - 1)), 0, RuntimePaintMaskHeight - 1);
	const int32 RadiusPixels = FMath::Max(1, FMath::RoundToInt(NormalizedBrushRadius * static_cast<float>(FMath::Max(RuntimePaintMaskWidth, RuntimePaintMaskHeight))));
	const int32 RadiusSquared = RadiusPixels * RadiusPixels;
	const uint8 NewValue = bPaintingGrass ? 255 : 0;

	const int32 MinX = FMath::Max(0, CenterX - RadiusPixels);
	const int32 MaxX = FMath::Min(RuntimePaintMaskWidth - 1, CenterX + RadiusPixels);
	const int32 MinY = FMath::Max(0, CenterY - RadiusPixels);
	const int32 MaxY = FMath::Min(RuntimePaintMaskHeight - 1, CenterY + RadiusPixels);

	for (int32 Y = MinY; Y <= MaxY; ++Y)
	{
		const int32 DeltaY = Y - CenterY;
		for (int32 X = MinX; X <= MaxX; ++X)
		{
			const int32 DeltaX = X - CenterX;
			if ((DeltaX * DeltaX) + (DeltaY * DeltaY) <= RadiusSquared)
			{
				RuntimePaintMaskValues[(Y * RuntimePaintMaskWidth) + X] = NewValue;
			}
		}
	}
}

bool AUserDrone::IsRuntimePaintLocationPlantable(const FHitResult& GroundHit) const
{
	if (RuntimePaintMaskValues.Num() == 0 || RuntimePaintMaskWidth <= 0 || RuntimePaintMaskHeight <= 0)
	{
		return false;
	}

	ALandscapeProxy* Landscape = Cast<ALandscapeProxy>(GroundHit.GetActor());
	if (!Landscape)
	{
		return false;
	}

	FVector2D PaintUV;
	float IgnoredBrushRadius = 0.f;
	if (!GetLandscapePaintUV(Landscape, GroundHit.Location, PaintUV, IgnoredBrushRadius))
	{
		return false;
	}

	const int32 PixelX = FMath::Clamp(FMath::RoundToInt(PaintUV.X * static_cast<float>(RuntimePaintMaskWidth - 1)), 0, RuntimePaintMaskWidth - 1);
	const int32 PixelY = FMath::Clamp(FMath::RoundToInt(PaintUV.Y * static_cast<float>(RuntimePaintMaskHeight - 1)), 0, RuntimePaintMaskHeight - 1);
	const uint8 PaintValue = RuntimePaintMaskValues[(PixelY * RuntimePaintMaskWidth) + PixelX];

	return PaintValue < 128;
}
FString AUserDrone::ExportPaintMaskData() const
{
	if (RuntimePaintMaskValues.Num() == 0 || RuntimePaintMaskWidth <= 0 || RuntimePaintMaskHeight <= 0)
	{
		return TEXT("");
	}

	TArray<FString> Runs;
	Runs.Reserve(256);

	uint8 CurrentValue = RuntimePaintMaskValues[0] >= 128 ? 1 : 0;
	int32 CurrentCount = 0;

	for (uint8 RawValue : RuntimePaintMaskValues)
	{
		const uint8 EncodedValue = RawValue >= 128 ? 1 : 0;
		if (EncodedValue == CurrentValue)
		{
			++CurrentCount;
			continue;
		}

		Runs.Add(FString::Printf(TEXT("%d%c"), CurrentCount, CurrentValue > 0 ? TCHAR('G') : TCHAR('D')));
		CurrentValue = EncodedValue;
		CurrentCount = 1;
	}

	Runs.Add(FString::Printf(TEXT("%d%c"), CurrentCount, CurrentValue > 0 ? TCHAR('G') : TCHAR('D')));
	return FString::Printf(TEXT("LLPM1:%d:%d:%s"), RuntimePaintMaskWidth, RuntimePaintMaskHeight, *FString::Join(Runs, TEXT(",")));
}

bool AUserDrone::ImportPaintMaskData(const FString& PaintMaskData)
{
	TArray<FString> Parts;
	PaintMaskData.ParseIntoArray(Parts, TEXT(":"), false);
	if (Parts.Num() != 4 || Parts[0] != TEXT("LLPM1"))
	{
		return false;
	}

	const int32 SavedWidth = FCString::Atoi(*Parts[1]);
	const int32 SavedHeight = FCString::Atoi(*Parts[2]);
	if (SavedWidth <= 0 || SavedHeight <= 0)
	{
		return false;
	}

	TArray<uint8> SavedValues;
	SavedValues.Reserve(SavedWidth * SavedHeight);

	TArray<FString> Runs;
	Parts[3].ParseIntoArray(Runs, TEXT(","), true);
	for (const FString& Run : Runs)
	{
		if (Run.Len() < 2)
		{
			continue;
		}

		const TCHAR ValueChar = Run[Run.Len() - 1];
		const int32 Count = FCString::Atoi(*Run.LeftChop(1));
		const uint8 Value = ValueChar == TCHAR('G') ? 255 : 0;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			SavedValues.Add(Value);
		}
	}

	if (SavedValues.Num() != SavedWidth * SavedHeight)
	{
		UE_LOG(LogTemp, Warning, TEXT("ImportPaintMaskData: invalid saved mask data size. Expected=%d Actual=%d"), SavedWidth * SavedHeight, SavedValues.Num());
		return false;
	}

	InitializeRuntimePaintMaskValues();
	if (RuntimePaintMaskValues.Num() == 0)
	{
		return false;
	}

	for (int32 Y = 0; Y < RuntimePaintMaskHeight; ++Y)
	{
		const int32 SourceY = FMath::Clamp(FMath::RoundToInt((static_cast<float>(Y) / FMath::Max(1, RuntimePaintMaskHeight - 1)) * static_cast<float>(SavedHeight - 1)), 0, SavedHeight - 1);
		for (int32 X = 0; X < RuntimePaintMaskWidth; ++X)
		{
			const int32 SourceX = FMath::Clamp(FMath::RoundToInt((static_cast<float>(X) / FMath::Max(1, RuntimePaintMaskWidth - 1)) * static_cast<float>(SavedWidth - 1)), 0, SavedWidth - 1);
			RuntimePaintMaskValues[(Y * RuntimePaintMaskWidth) + X] = SavedValues[(SourceY * SavedWidth) + SourceX];
		}
	}

	RuntimePaintMaskTexture = CreateTextureFromRuntimePaintMask();
	if (RuntimePaintMaskTexture)
	{
		CurrentPaintMask = RuntimePaintMaskTexture;
	}

	TArray<AActor*> FoundLandscapes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscapeProxy::StaticClass(), FoundLandscapes);
	for (AActor* Actor : FoundLandscapes)
	{
		if (ALandscapeProxy* Landscape = Cast<ALandscapeProxy>(Actor))
		{
			ApplyCurrentPaintMaskToLandscape(Landscape);
		}
	}

	return true;
}

void AUserDrone::SaveRuntimePaintMaskToDraft() const
{
	if (!GetGameInstance())
	{
		return;
	}

	if (UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>())
	{
		GardenSession->SetPaintMaskData(ExportPaintMaskData());
	}
}

UTexture2D* AUserDrone::CreateTextureFromRuntimePaintMask()
{
	if (RuntimePaintMaskValues.Num() == 0 || RuntimePaintMaskWidth <= 0 || RuntimePaintMaskHeight <= 0)
	{
		return nullptr;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(RuntimePaintMaskWidth, RuntimePaintMaskHeight, PF_B8G8R8A8);
	if (!Texture || !Texture->PlatformData || Texture->PlatformData->Mips.Num() == 0)
	{
		return nullptr;
	}

	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FColor* Pixels = static_cast<FColor*>(Data);
	for (int32 Index = 0; Index < RuntimePaintMaskValues.Num(); ++Index)
	{
		const uint8 Value = RuntimePaintMaskValues[Index];
		Pixels[Index] = FColor(Value, Value, Value, 255);
	}
	Mip.BulkData.Unlock();

	Texture->SRGB = false;
	Texture->UpdateResource();
	return Texture;
}

void AUserDrone::AdvancePaintRenderTargetsAfterDraw()
{
	UTextureRenderTarget2D* PaintedTarget = NextPaintMask;
	CurrentPaintMask = PaintedTarget;
	NextPaintMask = PaintedTarget == PaintMaskA ? PaintMaskB : PaintMaskA;
}
bool AUserDrone::GetLandscapePaintUV(ALandscapeProxy* Landscape, const FVector& WorldLocation, FVector2D& OutUV, float& OutNormalizedBrushRadius) const
{
	if (!Landscape)
	{
		return false;
	}

	FVector Origin;
	FVector Extent;
	Landscape->GetActorBounds(false, Origin, Extent);

	const float Width = FMath::Max(1.f, Extent.X * 2.f);
	const float Height = FMath::Max(1.f, Extent.Y * 2.f);
	const float MinX = Origin.X - Extent.X;
	const float MinY = Origin.Y - Extent.Y;

	OutUV.X = FMath::Clamp((WorldLocation.X - MinX) / Width, 0.f, 1.f);
	OutUV.Y = FMath::Clamp((WorldLocation.Y - MinY) / Height, 0.f, 1.f);
	OutNormalizedBrushRadius = FMath::Clamp(PaintBrushRadius / FMath::Max(Width, Height), 0.001f, 0.5f);
	return true;
}

void AUserDrone::ApplyCurrentPaintMaskToLandscape(ALandscapeProxy* Landscape) const
{
	if (!Landscape || !CurrentPaintMask)
	{
		return;
	}

	FVector Origin;
	FVector Extent;
	Landscape->GetActorBounds(false, Origin, Extent);

	TArray<ULandscapeComponent*> LandscapeComponents;
	Landscape->GetComponents<ULandscapeComponent>(LandscapeComponents);

	Landscape->bUseDynamicMaterialInstance = true;

	Landscape->SetLandscapeMaterialTextureParameterValue(LandscapePaintMaskParameterName, CurrentPaintMask);
	Landscape->SetLandscapeMaterialVectorParameterValue(
		LandscapePaintWorldMinParameterName,
		FLinearColor(Origin.X - Extent.X, Origin.Y - Extent.Y, 0.f, 0.f)
	);
	Landscape->SetLandscapeMaterialVectorParameterValue(
		LandscapePaintWorldSizeParameterName,
		FLinearColor(FMath::Max(1.f, Extent.X * 2.f), FMath::Max(1.f, Extent.Y * 2.f), 0.f, 0.f)
	);

	const FLinearColor PaintWorldMin(Origin.X - Extent.X, Origin.Y - Extent.Y, 0.f, 0.f);
	const FLinearColor PaintWorldSize(FMath::Max(1.f, Extent.X * 2.f), FMath::Max(1.f, Extent.Y * 2.f), 0.f, 0.f);

	int32 DynamicMaterialCount = 0;
	for (ULandscapeComponent* LandscapeComponent : LandscapeComponents)
	{
		if (!LandscapeComponent)
		{
			continue;
		}

		for (UMaterialInstanceDynamic* DynamicMaterial : LandscapeComponent->MaterialInstancesDynamic)
		{
			if (DynamicMaterial)
			{
				++DynamicMaterialCount;
				DynamicMaterial->SetTextureParameterValue(LandscapePaintMaskParameterName, CurrentPaintMask);
				DynamicMaterial->SetVectorParameterValue(LandscapePaintWorldMinParameterName, PaintWorldMin);
				DynamicMaterial->SetVectorParameterValue(LandscapePaintWorldSizeParameterName, PaintWorldSize);
			}
		}

		LandscapeComponent->MarkRenderStateDirty();
	}

	if (DynamicMaterialCount == 0)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("ApplyCurrentPaintMaskToLandscape: %s has no dynamic landscape material instances. Enable 'Use Dynamic Material Instance' on the Landscape actor, then restart PIE."),
			*Landscape->GetName()
		);
	}
}

void AUserDrone::EnsureDefaultPaintLayersLoaded()
{
	if (!DirtLayerInfo)
	{
		DirtLayerInfo = LoadObject<ULandscapeLayerInfoObject>(nullptr, TEXT("/Game/Garden_sharedassets/Dirt_LayerInfo.Dirt_LayerInfo"));
	}

	if (!GrassLayerInfo)
	{
		GrassLayerInfo = LoadObject<ULandscapeLayerInfoObject>(nullptr, TEXT("/Game/Garden_sharedassets/Grass_LayerInfo.Grass_LayerInfo"));
	}

	if (!SelectedPaintLayerInfo)
	{
		SelectedPaintLayerInfo = DirtLayerInfo ? DirtLayerInfo : GrassLayerInfo;
	}

	if (!DirtLayerInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnsureDefaultPaintLayersLoaded: Dirt layer info not found"));
	}
	if (!GrassLayerInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnsureDefaultPaintLayersLoaded: Grass layer info not found"));
	}
}

bool AUserDrone::IsGardenModificationBlocked() const
{
	if (!GetWorld())
	{
		return false;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UGardenHUD::StaticClass(), false);
	for (UUserWidget* Widget : FoundWidgets)
	{
		if (const UGardenHUD* GardenHUD = Cast<UGardenHUD>(Widget))
		{
			return GardenHUD->IsPredictionRunning();
		}
	}

	return false;
}

bool AUserDrone::SpawnPlant(APlant*& Plant)
{
	Plant = nullptr;

	if (IsGardenModificationBlocked())
	{
		return false;
	}

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
	const FString BloomDate = GetCurrentGardenTimelineDate();
	const FString PredictedPlantedDate = FindPredictedPlantedDateForSpecies(Plant->SpeciesId, BloomDate);

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGardenTimeManager::StaticClass(), Found);
	if (Found.Num() > 0)
	{
		AGardenTimeManager* TM = Cast<AGardenTimeManager>(Found[0]);
		if (TM)
		{
			if (!PredictedPlantedDate.IsEmpty())
			{
				ApplyPlacementSchedule(Plant, PredictedPlantedDate, BloomDate);
			}
			else
			{
				if (!BloomDate.IsEmpty())
				{
					int32 BloomDayIndex = -1;
					if (TryDateToDayIndex(BloomDate, BloomDayIndex))
					{
						Plant->BloomDayIndex = BloomDayIndex;
						Plant->DaysToWither = APlant::CalculateDaysToWitherFromBloom(Plant->DaysToBloom);
						Plant->WitherDayIndex = BloomDayIndex + Plant->DaysToWither;
					}
				}
				if (!bPredictedPlacementSchedulesEnabled)
				{
					Plant->SetForceBloomedMesh(true);
				}
				Plant->UpdatePlantingDateText();
				Plant->SetPlantingDateTextVisible(IsGardenDateSliderVisible());
			}
		}
	}

	return true;
}

void AUserDrone::TrackPlacedPlant(APlant* PlantActor)
{
	if (!PlantActor) return;
	if (IsGardenModificationBlocked()) return;

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

	const int32 SoilId = 1;
	const FString BloomDate = GetCurrentGardenTimelineDate();
	const FString PredictedPlantedDate = FindPredictedPlantedDateForSpecies(PlantActor->SpeciesId, BloomDate);

	PlantActor->HeightCm = 0.f;
	PlantActor->AgeDays = 0;
	PlantActor->HealthStatus = TEXT("Healthy");
	PlantActor->LastWateredIso8601 = FDateTime::UtcNow().ToIso8601();

	if (PlantActor->SpeciesId <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackPlacedPlant failed: PlantActor->SpeciesId is invalid (%d)"), PlantActor->SpeciesId);
		return;
	}

	const FGuid LocalId = GardenSession->AddPlantPlacement(
		PlantActor->SpeciesId,
		SoilId,
		PlantActor->GetActorLocation(),
		PlantActor->GetActorRotation(),
		PlantActor->GetActorScale3D(),
		PlantActor->HeightCm,
		PlantActor->AgeDays,
		PlantActor->HealthStatus,
		PlantActor->LastWateredIso8601,
		PlantActor->PerenualId,
		PlantActor->PlantName,
		TEXT(""),
		PlantActor->Category,
		PlantActor->Notes,
		PredictedPlantedDate,
		BloomDate
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

	if (!PredictedPlantedDate.IsEmpty())
	{
		ApplyPlacementSchedule(PlantActor, PredictedPlantedDate, BloomDate);
	}

	UE_LOG(LogTemp, Log, TEXT("Tracked placed plant. LocalId=%s PerenualId=%d"), *LocalId.ToString(), PlantActor->PerenualId);
}

void AUserDrone::DeletePlant(APlant* PlantActor)
{
	if (IsGardenModificationBlocked())
	{
		return;
	}

	if (!PlantActor)
	{
		return;
	}

	if (PlantActor->bIsTrackedInGardenDraft && GetGameInstance())
	{
		if (UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>())
		{
			GardenSession->RemovePlantPlacement(PlantActor->LocalPlantId);
		}
	}

	if (PreviewPlant == PlantActor)
	{
		PreviewPlant = nullptr;
	}

	PlantActor->Destroy();
	bDraggingRealPlant = false;
	bSelectedPlantSpawned = false;
}

void AUserDrone::CancelActivePlantInteraction()
{
	if (!PreviewPlant)
	{
		bDraggingRealPlant = false;
		bSelectedPlantSpawned = false;
		return;
	}

	APlant* PlantActor = PreviewPlant;
	PreviewPlant = nullptr;

	if (bDraggingRealPlant)
	{
		PlantActor->SetActorTransform(DragOriginalTransform);
	}
	else if (bSelectedPlantSpawned)
	{
		PlantActor->Destroy();
	}

	bDraggingRealPlant = false;
	bSelectedPlantSpawned = false;
}

void AUserDrone::SetGardenEditMode(EGardenEditMode NewMode)
{
	if (CurrentEditMode == NewMode)
	{
		return;
	}

	CancelActivePlantInteraction();
	CurrentEditMode = NewMode;
	bLeftPaintHeld = false;
}

void AUserDrone::SetPredictedPlacementSchedulesEnabled(bool bEnabled)
{
	bPredictedPlacementSchedulesEnabled = bEnabled;
}

void AUserDrone::SetPaintLayerByName(FName LayerName)
{
	EnsureDefaultPaintLayersLoaded();

	if (LayerName == TEXT("Grass"))
	{
		SelectedPaintLayerInfo = GrassLayerInfo;
	}
	else
	{
		SelectedPaintLayerInfo = DirtLayerInfo;
	}
}

FName AUserDrone::GetSelectedPaintLayerName() const
{
	if (SelectedPaintLayerInfo == GrassLayerInfo)
	{
		return TEXT("Grass");
	}

	return TEXT("Dirt");
}

void AUserDrone::SetPaintBrushRadius(float NewRadius)
{
	PaintBrushRadius = FMath::Clamp(NewRadius, 25.f, 1200.f);
}

FString AUserDrone::FindPredictedPlantedDateForSpecies(int32 SpeciesId, const FString& BloomDate) const
{
	const FString NormalizedBloomDate = FBloomDateUtils::NormalizeBackendDateString(BloomDate);
	if (!bPredictedPlacementSchedulesEnabled || SpeciesId <= 0 || NormalizedBloomDate.IsEmpty() || !GetGameInstance())
	{
		return TEXT("");
	}

	const UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();
	if (!GardenSession || !GardenSession->HasActiveDraft())
	{
		return TEXT("");
	}

	for (const FEditablePlantPlacement& Plant : GardenSession->GetDraft().Plants)
	{
		if (!Plant.bPendingDelete
			&& Plant.SpeciesId == SpeciesId
			&& FBloomDateUtils::NormalizeBackendDateString(Plant.BloomDate) == NormalizedBloomDate
			&& !Plant.PlantedDate.IsEmpty())
		{
			return Plant.PlantedDate;
		}
	}

	return TEXT("");
}

FString AUserDrone::GetCurrentGardenTimelineDate() const
{
	if (!GetWorld())
	{
		return TEXT("");
	}

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGardenTimeManager::StaticClass(), Found);
	if (Found.Num() <= 0)
	{
		return TEXT("");
	}

	const AGardenTimeManager* TimeManager = Cast<AGardenTimeManager>(Found[0]);
	if (!TimeManager)
	{
		return TEXT("");
	}

	const int32 CurrentDayIndex = TimeManager->GetCurrentDayIndex();
	const int32 FinalBloomDayIndex = TimeManager->GlobalBloomDate;
	if (CurrentDayIndex <= 0)
	{
		if (const UGardenSessionSubsystem* GardenSession = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>() : nullptr)
		{
			if (GardenSession->HasActiveDraft())
			{
				return GardenSession->GetDraft().BloomDate;
			}
		}
	}

	if (FinalBloomDayIndex > 0 && CurrentDayIndex > FinalBloomDayIndex)
	{
		return FBloomDateUtils::DayIndexToBackendDate(FinalBloomDayIndex);
	}
	if (FinalBloomDayIndex > 0 && CurrentDayIndex < FinalBloomDayIndex - 365)
	{
		return FBloomDateUtils::DayIndexToBackendDate(FinalBloomDayIndex - 365);
	}

	return FBloomDateUtils::DayIndexToBackendDate(CurrentDayIndex);
}

bool AUserDrone::HasPredictedPlantingDates() const
{
	if (!GetGameInstance())
	{
		return false;
	}

	const UGardenSessionSubsystem* GardenSession = GetGameInstance()->GetSubsystem<UGardenSessionSubsystem>();
	if (!GardenSession || !GardenSession->HasActiveDraft())
	{
		return false;
	}

	for (const FEditablePlantPlacement& Plant : GardenSession->GetDraft().Plants)
	{
		if (!Plant.bPendingDelete && !Plant.PlantedDate.IsEmpty())
		{
			return true;
		}
	}

	return false;
}

bool AUserDrone::TryDateToDayIndex(const FString& DateText, int32& OutDayIndex) const
{
	const FString NormalizedDate = FBloomDateUtils::NormalizeBackendDateString(DateText);
	if (NormalizedDate.IsEmpty())
	{
		return false;
	}

	FDateTime ParsedDate;
	if (!FDateTime::ParseIso8601(*NormalizedDate, ParsedDate))
	{
		return false;
	}

	const FDateTime DateOnly(ParsedDate.GetYear(), ParsedDate.GetMonth(), ParsedDate.GetDay());
	const FDateTime EpochDate(2000, 1, 1);
	OutDayIndex = static_cast<int32>((DateOnly - EpochDate).GetDays());
	return OutDayIndex >= 0;
}

void AUserDrone::HideGardenDateSlider() const
{
	if (!GetWorld())
	{
		return;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UGardenHUD::StaticClass(), false);
	for (UUserWidget* Widget : FoundWidgets)
	{
		if (UGardenHUD* GardenHUD = Cast<UGardenHUD>(Widget))
		{
			GardenHUD->HideDateSlider();
		}
	}
}

bool AUserDrone::IsGardenDateSliderVisible() const
{
	if (!GetWorld())
	{
		return false;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UGardenHUD::StaticClass(), false);
	for (UUserWidget* Widget : FoundWidgets)
	{
		if (const UGardenHUD* GardenHUD = Cast<UGardenHUD>(Widget))
		{
			return GardenHUD->IsDateSliderVisible();
		}
	}

	return false;
}

void AUserDrone::MoveGardenToBloomDate() const
{
	if (!GetWorld())
	{
		return;
	}

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGardenTimeManager::StaticClass(), Found);
	if (Found.Num() <= 0)
	{
		return;
	}

	if (AGardenTimeManager* TimeManager = Cast<AGardenTimeManager>(Found[0]))
	{
		TimeManager->SetCurrentDayIndex(TimeManager->GlobalBloomDate);
	}
}

void AUserDrone::ApplyPlacementSchedule(APlant* PlantActor, const FString& PlantedDate, const FString& BloomDate) const
{
	if (!PlantActor || !GetWorld())
	{
		return;
	}

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGardenTimeManager::StaticClass(), Found);
	if (Found.Num() <= 0)
	{
		return;
	}

	AGardenTimeManager* TimeManager = Cast<AGardenTimeManager>(Found[0]);
	if (!TimeManager)
	{
		return;
	}

	int32 PlantingDayIndex = -1;
	int32 BloomDayIndex = TimeManager->GlobalBloomDate;
	if (!BloomDate.IsEmpty())
	{
		int32 ParsedBloomDayIndex = -1;
		if (TryDateToDayIndex(BloomDate, ParsedBloomDayIndex))
		{
			BloomDayIndex = ParsedBloomDayIndex;
		}
	}

	if (!TryDateToDayIndex(PlantedDate, PlantingDayIndex) || PlantingDayIndex > BloomDayIndex)
	{
		return;
	}

	PlantActor->PlantingDayIndex = PlantingDayIndex;
	PlantActor->SetForceBloomedMesh(false);
	PlantActor->BloomDayIndex = FMath::Clamp(BloomDayIndex, PlantingDayIndex, TimeManager->GlobalBloomDate);
	PlantActor->DaysToBloom = FMath::Max(1, PlantActor->BloomDayIndex - PlantActor->PlantingDayIndex);
	PlantActor->DaysToWither = APlant::CalculateDaysToWitherFromBloom(PlantActor->DaysToBloom);
	PlantActor->WitherDayIndex = PlantActor->BloomDayIndex + PlantActor->DaysToWither;
	PlantActor->UpdateForDay(TimeManager->GetCurrentDayIndex());
	PlantActor->SetPlantingDateTextVisible(IsGardenDateSliderVisible());
}
