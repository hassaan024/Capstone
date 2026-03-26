// Fill out your copyright notice in the Description page of Project Settings.


#include "Plant.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "GardenTimeManager.h"

// Sets default values
APlant::APlant()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Preview Mesh"));
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// Called when the game starts or when spawned
void APlant::BeginPlay()
{
	Super::BeginPlay();
	PreviewMaterial = PreviewMesh->GetMaterial(0);
	DynamicPreviewMaterial = UMaterialInstanceDynamic::Create(PreviewMaterial, this);
	DynamicPreviewMaterial->SetScalarParameterValue(TEXT("Opacity"), 0.3f);
	PreviewMesh->SetMaterial(0, DynamicPreviewMaterial);

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

// Called every frame
void APlant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlant::HandleDayChanged(int32 NewDayIndex)
{
	UpdateForDay(NewDayIndex);
}

void APlant::UpdateForDay(int32 DayIndex)
{
	const int32 Age = DayIndex - PlantedDayIndex;
	UE_LOG(LogTemp, Warning, TEXT("UpdateForDay Called with index %d"), DayIndex);

	float Growth01 = 0.f;
	if (DaysToBloom > 0)
	{
		Growth01 = FMath::Clamp(static_cast<float>(Age) / static_cast<float>(DaysToBloom), 0.f, 1.f);
	}

	const float Scale = FMath::Lerp(0.2f, 1.0f, Growth01);
	SetActorScale3D(FVector(Scale));
}

