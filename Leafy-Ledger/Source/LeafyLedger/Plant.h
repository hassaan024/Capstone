#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Plant.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UTextRenderComponent;
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

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		FString PlantingDate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Planting Date")
		float PlantingDateTextPadding = 30.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Planting Date")
		bool bPlantingDateTextEnabled = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 WitherDayIndex = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 DaysToBloom = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		int32 DaysToWither = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		FString PlantName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		FString Category = "";

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Garden Save")
		int32 PerenualId = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Garden Save")
		int32 SpeciesId = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Garden Save")
		float HeightCm = 0.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Garden Save")
		int32 AgeDays = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Garden Save")
		FString HealthStatus;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Garden Save")
		FString LastWateredIso8601;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Garden Save")
		FString Notes;

	UPROPERTY(BlueprintReadWrite, Category = "Garden Save")
		FGuid LocalPlantId;

	UPROPERTY(BlueprintReadWrite, Category = "Garden Save")
		bool bIsTrackedInGardenDraft = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Flower")
		UStaticMesh* FlowerSeedMesh = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Flower")
		UStaticMesh* FlowerSaplingMesh = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Flower")
		UStaticMesh* FlowerBloomedMesh = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Flower")
		UStaticMesh* FlowerWitherMesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Tree")
		UStaticMesh* TreeSeedMesh = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Tree")
		UStaticMesh* TreeSaplingMesh = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Tree")
		UStaticMesh* TreeBloomedMesh = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Tree")
		UStaticMesh* TreeWitherMesh = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Vegetable")
		UStaticMesh* VegetableSeedMesh = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Vegetable")
		UStaticMesh* VegetableSaplingMesh = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Vegetable")
		UStaticMesh* VegetableBloomedMesh = nullptr;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PlantMeshes|Vegetable")
		UStaticMesh* VegetableWitherMesh = nullptr;

	UPROPERTY(BlueprintReadOnly)
		UStaticMesh* SeedMesh = nullptr;
	UPROPERTY(BlueprintReadOnly)
		UStaticMesh* SaplingMesh = nullptr;
	UPROPERTY(BlueprintReadOnly)
		UStaticMesh* BloomedMesh = nullptr;
	UPROPERTY(BlueprintReadOnly)
		UStaticMesh* WitherMesh = nullptr;

	UFUNCTION()
		void HandleDayChanged(int32 NewDayIndex);

	UFUNCTION(BlueprintCallable)
		void InitializeFromPlantData(UPlantObject* PlantData);

	void UpdateForDay(int32 DayIndex);

	UFUNCTION(BlueprintImplementableEvent)
		void BlueprintDayUpdate(int32 DayIndex);

	UFUNCTION(BlueprintCallable)
		UStaticMeshComponent* GetPreviewMesh() const { return PreviewMesh; }

	UFUNCTION(BlueprintCallable)
		void UpdatePlantingDateText();

	UFUNCTION(BlueprintCallable)
		void SetPlantingDateTextVisible(bool bVisible);

	void RefreshPlantingDateTextVisibility();

	UTextRenderComponent* GetPlantingDateTextRender() const;
};
