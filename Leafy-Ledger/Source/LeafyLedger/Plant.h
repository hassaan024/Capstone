#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Plant.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPlantObject;

UCLASS()
class LEAFYLEDGER_API APlant : public AActor
{
	GENERATED_BODY()

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* PreviewMesh;

public:
	APlant();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
		UMaterialInterface* PreviewMaterial;

	UPROPERTY(BlueprintReadOnly)
		UMaterialInstanceDynamic* DynamicPreviewMaterial;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
		FVector InvalidPlacementColor;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
		FVector ValidPlacementColor;

	UFUNCTION(BlueprintImplementableEvent)
		void SetPlacedMaterial();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 BloomDayIndex = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 PlantingDayIndex = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 WitherDayIndex = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 DaysToBloom = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 DaysToWither = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		FString PlantName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Garden Save")
	int32 PerenualId = -1;

	UPROPERTY(BlueprintReadWrite, Category = "Garden Save")
	FGuid LocalPlantId;

	UPROPERTY(BlueprintReadWrite, Category = "Garden Save")
	bool bIsTrackedInGardenDraft = false;

	UFUNCTION()
		void HandleDayChanged(int32 NewDayIndex);

	UFUNCTION(BlueprintCallable)
		void InitializeFromPlantData(UPlantObject* PlantData);

	void UpdateForDay(int32 DayIndex);
	UFUNCTION(BlueprintImplementableEvent)
		void BlueprintDayUpdate(int32 DayIndex);

	UFUNCTION(BlueprintCallable)
		UStaticMeshComponent* GetPreviewMesh() const { return PreviewMesh; }
};