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
		int32 TrefleId = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
		int32 DaysToBloom = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
		int32 DaysToWither = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
		TSubclassOf<APlant> PlantClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Plant")
		UStaticMesh* PlantMesh = nullptr;
};