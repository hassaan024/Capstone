// Fill out your copyright notice in the Description page of Project Settings.


#include "Plant.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

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
}

// Called every frame
void APlant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

