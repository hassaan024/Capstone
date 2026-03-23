// Fill out your copyright notice in the Description page of Project Settings.


#include "GardenDirector.h"

// Sets default values
AGardenDirector::AGardenDirector()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGardenDirector::BeginPlay()
{
	Super::BeginPlay();
	MakePlantList();
}

// Called every frame
void AGardenDirector::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

