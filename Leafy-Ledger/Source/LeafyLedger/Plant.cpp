#include "Plant.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "GardenTimeManager.h"
#include "PlantObject.h"
#include "BloomDateUtils.h"

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
	Category = PlantData->ModelCategory.TrimStartAndEnd();
	const FString NormalizedCategory = Category.ToLower();

	// Set the meshes to correspond with their categories
	if (NormalizedCategory == TEXT("tree")) {
		SeedMesh = TreeSeedMesh;
		SaplingMesh = TreeSaplingMesh;
		BloomedMesh = TreeBloomedMesh;
		WitherMesh = TreeWitherMesh;
	}
	else if (NormalizedCategory == TEXT("flower")) {
		SeedMesh = FlowerSeedMesh;
		SaplingMesh = FlowerSaplingMesh;
		BloomedMesh = FlowerBloomedMesh;
		WitherMesh = FlowerWitherMesh;
	}
	else if (NormalizedCategory == TEXT("vegetable")) {
		SeedMesh = VegetableSeedMesh;
		SaplingMesh = VegetableSaplingMesh;
		BloomedMesh = VegetableBloomedMesh;
		WitherMesh = VegetableWitherMesh;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Unknown plant model category '%s' for %s; defaulting to flower meshes"), *Category, *PlantName);
		Category = TEXT("flower");
		SeedMesh = FlowerSeedMesh;
		SaplingMesh = FlowerSaplingMesh;
		BloomedMesh = FlowerBloomedMesh;
		WitherMesh = FlowerWitherMesh;
	}

	if (PreviewMesh && BloomedMesh)
	{
		PreviewMesh->SetStaticMesh(BloomedMesh);
	}

	if (Category != "") UE_LOG(LogTemp, Warning, TEXT("%s"), *Category);
}

void APlant::UpdateForDay(int32 DayIndex)
{
	PlantingDate = FBloomDateUtils::DayIndexToDisplayDate(PlantingDayIndex);
	BlueprintDayUpdate(DayIndex);

	if (!PreviewMesh)
	{
		return;
	}

	UStaticMesh* DesiredMesh = nullptr;
	FVector DesiredScale = FVector(1.f);

	if (DayIndex < PlantingDayIndex)
	{
		PreviewMesh->SetVisibility(false, true);
		PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetActorEnableCollision(false);
		SetActorScale3D(DesiredScale);
		return;
	}

	PreviewMesh->SetVisibility(true, true);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetActorEnableCollision(true);

	if (DayIndex < WitherDayIndex)
	{
		const int32 GrowthDuration = FMath::Max(1, BloomDayIndex - PlantingDayIndex);
		const float GrowthAlpha = FMath::Clamp(static_cast<float>(DayIndex - PlantingDayIndex) / static_cast<float>(GrowthDuration), 0.f, 1.f);

		if (DayIndex == BloomDayIndex)
		{
			DesiredMesh = BloomedMesh ? BloomedMesh : (SaplingMesh ? SaplingMesh : SeedMesh);
		}
		else if (GrowthAlpha < 0.33f)
		{
			DesiredMesh = SeedMesh ? SeedMesh : SaplingMesh;
		}
		else
		{
			DesiredMesh = SaplingMesh ? SaplingMesh : SeedMesh;
		}
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
