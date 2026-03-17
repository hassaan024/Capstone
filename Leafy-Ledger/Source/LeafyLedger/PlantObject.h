// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlantObject.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class LEAFYLEDGER_API UPlantObject : public UObject
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Plant")
	FString CommonName;

	UPROPERTY(BlueprintReadOnly, Category = "Plant")
	FString ScientificName;

	//UPROPERTY(BlueprintReadOnly, Category = "Plant")
	//FString Family;

	UPROPERTY(BlueprintReadOnly, Category = "Plant")
	FString ImgSrcUrl;

	UPROPERTY(BlueprintReadOnly)
	int32 TrefleId;
};
