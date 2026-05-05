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
	FVector Delta = MouseDragStart - CurrentLoc;
	Delta.Z = 0.0f;
	AddActorWorldOffset(Delta, true);
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
		if (GroundHit.PhysMaterial.IsValid())
		{
			UPhysicalMaterial* PhysMat = GroundHit.PhysMaterial.Get();
			if (*PhysMat->GetName() == FName("DirtPhysMat")) return true;
		}
	}
	return false;
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
	const FString PredictedPlantedDate = FindPredictedPlantedDateForSpecies(Plant->SpeciesId);

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGardenTimeManager::StaticClass(), Found);
	if (Found.Num() > 0)
	{
		AGardenTimeManager* TM = Cast<AGardenTimeManager>(Found[0]);
		if (TM)
		{
			if (!PredictedPlantedDate.IsEmpty())
			{
				ApplyPlacementSchedule(Plant, PredictedPlantedDate);
			}
			else
			{
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
	const FString PredictedPlantedDate = FindPredictedPlantedDateForSpecies(PlantActor->SpeciesId);
	const bool bPredictionWasRun = HasPredictedPlantingDates();

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
		PredictedPlantedDate
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
		ApplyPlacementSchedule(PlantActor, PredictedPlantedDate);
	}
	else if (bPredictionWasRun)
	{
		MoveGardenToBloomDate();
		HideGardenDateSlider();
		UE_LOG(LogTemp, Log, TEXT("Placed species %d without an existing prediction. Date slider hidden until predictions are rerun."), PlantActor->SpeciesId);
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
}

FString AUserDrone::FindPredictedPlantedDateForSpecies(int32 SpeciesId) const
{
	if (SpeciesId <= 0 || !GetGameInstance())
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
			&& !Plant.PlantedDate.IsEmpty())
		{
			return Plant.PlantedDate;
		}
	}

	return TEXT("");
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

void AUserDrone::ApplyPlacementSchedule(APlant* PlantActor, const FString& PlantedDate) const
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
	if (!TryDateToDayIndex(PlantedDate, PlantingDayIndex) || PlantingDayIndex > TimeManager->GlobalBloomDate)
	{
		return;
	}

	PlantActor->PlantingDayIndex = PlantingDayIndex;
	PlantActor->BloomDayIndex = TimeManager->GlobalBloomDate;
	PlantActor->DaysToBloom = FMath::Max(1, PlantActor->BloomDayIndex - PlantActor->PlantingDayIndex);
	PlantActor->WitherDayIndex = PlantActor->DaysToWither > 0
		? PlantActor->BloomDayIndex + PlantActor->DaysToWither
		: MAX_int32;
	PlantActor->UpdateForDay(TimeManager->GetCurrentDayIndex());
	PlantActor->SetPlantingDateTextVisible(IsGardenDateSliderVisible());
}
