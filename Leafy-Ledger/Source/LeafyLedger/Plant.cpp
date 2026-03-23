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

	PlantName = PlantData->CommonName;
	DaysToBloom = PlantData->DaysToBloom;
	DaysToWither = PlantData->DaysToWither;

	if (PreviewMesh && PlantData->PlantMesh)
	{
		PreviewMesh->SetStaticMesh(PlantData->PlantMesh);
	}
}

void APlant::UpdateForDay(int32 DayIndex)
{
	BlueprintDayUpdate(DayIndex);
	float Scale01 = 0.f;

	if (DayIndex < PlantingDayIndex)
	{
		Scale01 = 0.f;
	}
	else if (DayIndex <= BloomDayIndex)
	{
		if (DaysToBloom <= 0)
		{
			Scale01 = 1.f;
		}
		else
		{
			Scale01 = FMath::Clamp(static_cast<float>(DayIndex - PlantingDayIndex) / static_cast<float>(DaysToBloom), 0.f, 1.f);
		}
	}
	else if (DayIndex <= WitherDayIndex)
	{
		if (DaysToWither <= 0)
		{
			Scale01 = 0.f;
		}
		else
		{
			Scale01 = 1.f - FMath::Clamp(static_cast<float>(DayIndex - BloomDayIndex) / static_cast<float>(DaysToWither), 0.f, 1.f);
		}
	}
	else
	{
		Scale01 = 0.f;
	}

	SetActorScale3D(FVector(Scale01));
}