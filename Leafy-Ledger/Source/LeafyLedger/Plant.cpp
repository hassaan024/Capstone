#include "Plant.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "GardenTimeManager.h"
#include "PlantObject.h"

APlant::APlant()
{
	PrimaryActorTick.bCanEverTick = true;

	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Preview Mesh"));
	SetRootComponent(PreviewMesh);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void APlant::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Plant beginplay"));
	PreviewMaterial = PreviewMesh->GetMaterial(0);
	if (PreviewMaterial)
	{
		DynamicPreviewMaterial = UMaterialInstanceDynamic::Create(PreviewMaterial, this);
		if (DynamicPreviewMaterial)
		{
			DynamicPreviewMaterial->SetScalarParameterValue(TEXT("Opacity"), 0.3f);
			PreviewMesh->SetMaterial(0, DynamicPreviewMaterial);
		}
	}

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGardenTimeManager::StaticClass(), Found);
	if (Found.Num() > 0)
	{
		AGardenTimeManager* TM = Cast<AGardenTimeManager>(Found[0]);
		if (TM)
		{
			TM->OnDayChanged.AddDynamic(this, &APlant::HandleDayChanged);
			UpdateForDay(TM->GetCurrentDayIndex());
		}
	}
}

void APlant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlant::HandleDayChanged(int32 NewDayIndex)
{
	UpdateForDay(NewDayIndex);
}

void APlant::InitializeFromPlantData(UPlantObject* PlantData)
{
	if (!PlantData) return;

	SpeciesId = PlantData->SpeciesId;
	PerenualId = PlantData->PerenualId;
	PlantName = PlantData->CommonName;
	DaysToBloom = PlantData->DaysToBloom;
	DaysToWither = PlantData->DaysToWither;
	Category = PlantData->ModelCategory;

	// Set the meshes to correspond with their categories
	if (Category == "Tree") {
		SeedMesh = TreeSeedMesh;
		SaplingMesh = TreeSaplingMesh;
		BloomedMesh = TreeBloomedMesh;
		WitherMesh = TreeWitherMesh;
	}
	else if (Category == "Flower") {
		SeedMesh = FlowerSeedMesh;
		SaplingMesh = FlowerSaplingMesh;
		BloomedMesh = FlowerBloomedMesh;
		WitherMesh = FlowerWitherMesh;
	}
	else {
		SeedMesh = VegetableSeedMesh;
		SaplingMesh = VegetableSaplingMesh;
		BloomedMesh = VegetableBloomedMesh;
		WitherMesh = VegetableWitherMesh;
	}

	if (PreviewMesh && BloomedMesh)
	{
		PreviewMesh->SetStaticMesh(BloomedMesh);
	}

	if (Category != "") UE_LOG(LogTemp, Warning, TEXT("%s"), *Category);
}

void APlant::UpdateForDay(int32 DayIndex)
{
	BlueprintDayUpdate(DayIndex);

	if (!PreviewMesh)
	{
		return;
	}

	UStaticMesh* DesiredMesh = nullptr;
	FVector DesiredScale = FVector(1.f);

	if (DayIndex < PlantingDayIndex)
	{
		DesiredMesh = SeedMesh;
		DesiredScale = FVector(1.f);
	}
	else if (DayIndex < BloomDayIndex)
	{
		if (SeedMesh && SaplingMesh)
		{
			const int32 GrowthDuration = FMath::Max(1, BloomDayIndex - PlantingDayIndex);
			const float GrowthAlpha = FMath::Clamp(static_cast<float>(DayIndex - PlantingDayIndex) / static_cast<float>(GrowthDuration), 0.f, 1.f);

			if (GrowthAlpha < 0.5f)
			{
				DesiredMesh = SeedMesh;
			}
			else
			{
				DesiredMesh = SaplingMesh;
			}
		}
		else if (SaplingMesh)
		{
			DesiredMesh = SaplingMesh;
		}
		else
		{
			DesiredMesh = SeedMesh;
		}
	}
	else if (DayIndex < WitherDayIndex)
	{
		DesiredMesh = BloomedMesh;
	}
	else
	{
		DesiredMesh = WitherMesh ? WitherMesh : BloomedMesh;
	}

	if (DesiredMesh)
	{
		PreviewMesh->SetStaticMesh(DesiredMesh);
	}

	SetActorScale3D(DesiredScale);
}