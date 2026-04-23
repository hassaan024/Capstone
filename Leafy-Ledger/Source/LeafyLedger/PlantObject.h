#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlantObject.generated.h"

class APlant;
class UStaticMesh;

UCLASS(BlueprintType)
class LEAFYLEDGER_API UPlantObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
	FString CommonName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
	FString ScientificName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
	FString ImgSrcUrl;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
	int32 PerenualId = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
	int32 SpeciesId = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
	int32 DaysToBloom = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
	int32 DaysToWither = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
	TSubclassOf<APlant> PlantClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
	UStaticMesh* PlantMesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString ModelCategory;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString SelectedMeshId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SliderValue = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
	bool bIsDropdownToggle = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
	bool bIsGlobalPlant = false;
};
