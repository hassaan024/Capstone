#include "Plant.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
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

void APlant::NotifyActorBeginCursorOver()
{
	Super::NotifyActorBeginCursorOver();
	UpdateHoverPlantingDateText();
}

void APlant::NotifyActorEndCursorOver()
{
	Super::NotifyActorEndCursorOver();
	UpdatePlantingDateText();
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
	DaysToWither = CalculateDaysToWitherFromBloom(DaysToBloom);
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

	UpdatePlantingDateText();
}

int32 APlant::CalculateDaysToWitherFromBloom(int32 InDaysToBloom)
{
	return InDaysToBloom > 0
		? FMath::Max(1, FMath::RoundToInt(static_cast<float>(InDaysToBloom) / 3.0f))
		: 30;
}

void APlant::UpdateForDay(int32 DayIndex)
{
	UpdateDateStrings();

	if (bForceBloomedMesh)
	{
		if (PreviewMesh)
		{
			PreviewMesh->SetVisibility(true, true);
			PreviewMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SetActorEnableCollision(true);

			if (BloomedMesh)
			{
				PreviewMesh->SetStaticMesh(BloomedMesh);
			}

			PreviewMesh->SetRelativeScale3D(FVector(BloomStageScale));
		}

		UpdatePlantingDateText();
		return;
	}

	if (!PreviewMesh)
	{
		BlueprintDayUpdate(DayIndex);
		UpdatePlantingDateText();
		return;
	}

	UStaticMesh* DesiredMesh = nullptr;
	FVector DesiredVisualScale = FVector(BloomStageScale);

	if (BloomDayIndex <= 0 || PlantingDayIndex <= 0)
	{
		PreviewMesh->SetVisibility(true, true);
		PreviewMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SetActorEnableCollision(true);
		RefreshPlantingDateTextVisibility();

		if (BloomedMesh)
		{
			PreviewMesh->SetStaticMesh(BloomedMesh);
		}

		PreviewMesh->SetRelativeScale3D(DesiredVisualScale);
		BlueprintDayUpdate(DayIndex);
		UpdatePlantingDateText();
		return;
	}

	if (DayIndex < PlantingDayIndex)
	{
		PreviewMesh->SetVisibility(false, true);
		PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetActorEnableCollision(false);
		RefreshPlantingDateTextVisibility();
		PreviewMesh->SetRelativeScale3D(DesiredVisualScale);
		BlueprintDayUpdate(DayIndex);
		UpdatePlantingDateText();
		return;
	}

	PreviewMesh->SetVisibility(true, true);
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetActorEnableCollision(true);
	RefreshPlantingDateTextVisibility();

	if (DayIndex < WitherDayIndex)
	{
		const int32 GrowthDuration = FMath::Max(1, BloomDayIndex - PlantingDayIndex);
		const float GrowthAlpha = FMath::Clamp(static_cast<float>(DayIndex - PlantingDayIndex) / static_cast<float>(GrowthDuration), 0.f, 1.f);
		constexpr float SaplingStartAlpha = 0.33f;

		if (DayIndex >= BloomDayIndex)
		{
			DesiredMesh = BloomedMesh ? BloomedMesh : (SaplingMesh ? SaplingMesh : SeedMesh);
			DesiredVisualScale = FVector(BloomStageScale);
		}
		else if (GrowthAlpha < SaplingStartAlpha)
		{
			DesiredMesh = SeedMesh ? SeedMesh : SaplingMesh;
			const float SeedAlpha = FMath::Clamp(GrowthAlpha / SaplingStartAlpha, 0.f, 1.f);
			DesiredVisualScale = FVector(FMath::Lerp(SeedStageStartScale, SeedStageEndScale, SeedAlpha));
		}
		else
		{
			DesiredMesh = SaplingMesh ? SaplingMesh : SeedMesh;
			const float SaplingAlpha = FMath::Clamp((GrowthAlpha - SaplingStartAlpha) / (1.f - SaplingStartAlpha), 0.f, 1.f);
			DesiredVisualScale = FVector(FMath::Lerp(SaplingStageStartScale, BloomStageScale, SaplingAlpha));
		}
	}
	else
	{
		DesiredMesh = WitherMesh ? WitherMesh : BloomedMesh;
		DesiredVisualScale = FVector(BloomStageScale);
	}

	if (DesiredMesh)
	{
		PreviewMesh->SetStaticMesh(DesiredMesh);
	}

	PreviewMesh->SetRelativeScale3D(DesiredVisualScale);
	BlueprintDayUpdate(DayIndex);
	UpdatePlantingDateText();
}

void APlant::UpdatePlantingDateText()
{
	UpdateDateStrings();

	UTextRenderComponent* TextRender = GetPlantingDateTextRender();
	if (!TextRender)
	{
		return;
	}

	TextRender->SetText(FText::FromString(BuildDefaultTextRenderText()));

	if (!PreviewMesh)
	{
		return;
	}

	FVector Origin;
	FVector BoxExtent;
	float SphereRadius = 0.f;
	UKismetSystemLibrary::GetComponentBounds(PreviewMesh, Origin, BoxExtent, SphereRadius);

	TextRender->SetWorldLocation(FVector(Origin.X, Origin.Y, Origin.Z + BoxExtent.Z + PlantingDateTextPadding));
	TextRender->SetWorldScale3D(PlantingDateTextWorldScale);
	RefreshPlantingDateTextVisibility();
}

void APlant::UpdateHoverPlantingDateText()
{
	UpdateDateStrings();

	if (UTextRenderComponent* TextRender = GetPlantingDateTextRender())
	{
		TextRender->SetText(FText::FromString(BuildHoverTextRenderText()));
	}
}

void APlant::SetForceBloomedMesh(bool bForce)
{
	bForceBloomedMesh = bForce;
	if (!bForceBloomedMesh)
	{
		return;
	}

	if (PreviewMesh)
	{
		PreviewMesh->SetVisibility(true, true);
		PreviewMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SetActorEnableCollision(true);

		if (BloomedMesh)
		{
			PreviewMesh->SetStaticMesh(BloomedMesh);
		}

		PreviewMesh->SetRelativeScale3D(FVector(BloomStageScale));
	}

	UpdatePlantingDateText();
}

void APlant::UpdateDateStrings()
{
	PlantingDate = PlantingDayIndex > 0 ? FBloomDateUtils::DayIndexToDisplayDate(PlantingDayIndex) : TEXT("");
	BloomDate = BloomDayIndex > 0 ? FBloomDateUtils::DayIndexToDisplayDate(BloomDayIndex) : TEXT("");
}

FString APlant::BuildDefaultTextRenderText() const
{
	return BloomDate.IsEmpty()
		? PlantName
		: PlantName + TEXT("\n") + BloomDate;
}

FString APlant::BuildHoverTextRenderText() const
{
	return PlantingDate.IsEmpty()
		? BuildDefaultTextRenderText()
		: BuildDefaultTextRenderText() + TEXT("\n") + PlantingDate;
}

UTextRenderComponent* APlant::GetPlantingDateTextRender() const
{
	TArray<UTextRenderComponent*> TextRenderComponents;
	GetComponents<UTextRenderComponent>(TextRenderComponents);

	for (UTextRenderComponent* Component : TextRenderComponents)
	{
		if (Component && Component->GetName() == TEXT("TextRender"))
		{
			return Component;
		}
	}

	return TextRenderComponents.Num() > 0 ? TextRenderComponents[0] : nullptr;
}

void APlant::SetPlantingDateTextVisible(bool bVisible)
{
	bPlantingDateTextEnabled = bVisible;
	RefreshPlantingDateTextVisibility();
}

void APlant::RefreshPlantingDateTextVisibility()
{
	if (UTextRenderComponent* TextRender = GetPlantingDateTextRender())
	{
		const bool bMeshVisible = PreviewMesh && PreviewMesh->IsVisible();
		TextRender->SetVisibility(bMeshVisible);
	}
}
