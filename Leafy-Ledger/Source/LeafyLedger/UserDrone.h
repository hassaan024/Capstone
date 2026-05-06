#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "UserDrone.generated.h"

class APlant;
class AUserDroneController;
class UPlantObject;
class ULandscapeLayerInfoObject;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;
class UTexture;
class UTexture2D;
class ALandscapeProxy;

UENUM(BlueprintType)
enum class EGardenEditMode : uint8
{
	Plant UMETA(DisplayName = "Plant"),
	Paint UMETA(DisplayName = "Paint"),
	Delete UMETA(DisplayName = "Delete")
};

UCLASS()
class LEAFYLEDGER_API AUserDrone : public ACharacter
{
	GENERATED_BODY()

public:
	AUserDrone();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UCameraComponent* CameraComp;

protected:
	virtual void BeginPlay() override;

#pragma region Base Input
private:
	void LeftMousePressed();
	void LeftMouseReleased();
	void RightMousePressed();
	void RightMouseReleased();
	void MouseScroll(float Axis);
#pragma endregion

#pragma region Real-Time Variables
protected:
	UPROPERTY(BlueprintReadWrite, Category = "Plants")
		UPlantObject* SelectedPlantData = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Plants")
		TSubclassOf<APlant> DefaultPlantClass;

	bool bSelectedPlantSpawned = false;

private:
	AUserDroneController* PC;
	bool bRightClickHeld = false;
	bool bLeftPaintHeld = false;
	bool bDraggingRealPlant = false;
	FVector MouseDragStart;
	APlant* PreviewPlant = nullptr;
	FTransform DragOriginalTransform;
	EGardenEditMode CurrentEditMode = EGardenEditMode::Plant;

	UPROPERTY(EditDefaultsOnly, Category = "Paint")
	ULandscapeLayerInfoObject* DirtLayerInfo = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Paint")
	ULandscapeLayerInfoObject* GrassLayerInfo = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Paint")
	float PaintBrushRadius = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Paint")
	float PaintStrength = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Paint|Runtime Mask")
	UTextureRenderTarget2D* PaintMaskA = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Paint|Runtime Mask")
	UTextureRenderTarget2D* PaintMaskB = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Paint|Runtime Mask")
	UMaterialInterface* PaintBrushMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Paint|Runtime Mask")
	FName LandscapePaintMaskParameterName = TEXT("PaintMask");

	UPROPERTY(EditDefaultsOnly, Category = "Paint|Runtime Mask")
	FName LandscapePaintWorldMinParameterName = TEXT("PaintWorldMin");

	UPROPERTY(EditDefaultsOnly, Category = "Paint|Runtime Mask")
	FName LandscapePaintWorldSizeParameterName = TEXT("PaintWorldSize");

	UPROPERTY(EditDefaultsOnly, Category = "Paint|Runtime Mask")
	FName BrushPreviousMaskParameterName = TEXT("PreviousMask");

	UPROPERTY(EditDefaultsOnly, Category = "Paint|Runtime Mask")
	FName BrushCenterParameterName = TEXT("BrushCenter");

	UPROPERTY(EditDefaultsOnly, Category = "Paint|Runtime Mask")
	FName BrushRadiusParameterName = TEXT("BrushRadius");

	UPROPERTY(EditDefaultsOnly, Category = "Paint|Runtime Mask")
	FName BrushPaintValueParameterName = TEXT("PaintValue");

	UPROPERTY()
	ULandscapeLayerInfoObject* SelectedPaintLayerInfo = nullptr;

	UPROPERTY()
	UMaterialInstanceDynamic* PaintBrushMID = nullptr;

	UPROPERTY()
	UTexture* CurrentPaintMask = nullptr;

	UPROPERTY()
	UTextureRenderTarget2D* NextPaintMask = nullptr;

	
	UPROPERTY()
	UTexture2D* RuntimePaintMaskTexture = nullptr;
TArray<uint8> RuntimePaintMaskValues;
	int32 RuntimePaintMaskWidth = 0;
	int32 RuntimePaintMaskHeight = 0;
#pragma endregion

#pragma region Tick Functions
private:
	void UpdatePan();
	void UpdatePreviewPlant();
	void UpdatePaintBrushPreview();
#pragma endregion

#pragma region Helper Functions
private:
	bool GetMouseGroundHit(FHitResult& OutHit);
	bool GetMousePaintHit(FHitResult& OutHit);
	bool GetMousePlantHit(FHitResult& OutHit);
	bool ValidPlantPlacement();
	void PaintSelectedLandscapeLayer();
	bool PaintLandscapeAtHit(const FHitResult& LandscapeHit, ULandscapeLayerInfoObject* LayerInfo);
	void InitializeRuntimePaint();
	void InitializeRuntimePaintMaskValues();
	void PaintRuntimeMaskValuesAt(const FVector2D& PaintUV, float NormalizedBrushRadius, bool bPaintingGrass);
	bool IsRuntimePaintLocationPlantable(const FHitResult& GroundHit) const;
		void SaveRuntimePaintMaskToDraft() const;
	UTexture2D* CreateTextureFromRuntimePaintMask();
	void AdvancePaintRenderTargetsAfterDraw();
bool GetLandscapePaintUV(ALandscapeProxy* Landscape, const FVector& WorldLocation, FVector2D& OutUV, float& OutNormalizedBrushRadius) const;
	void ApplyCurrentPaintMaskToLandscape(ALandscapeProxy* Landscape) const;
	void EnsureDefaultPaintLayersLoaded();
	bool IsGardenModificationBlocked() const;
	void TrackPlacedPlant(APlant* PlantActor);
	void DeletePlant(APlant* PlantActor);
	//void CancelActivePlantInteraction();
	FString FindPredictedPlantedDateForSpecies(int32 SpeciesId) const;
	bool HasPredictedPlantingDates() const;
	bool TryDateToDayIndex(const FString& DateText, int32& OutDayIndex) const;
	void HideGardenDateSlider() const;
	bool IsGardenDateSliderVisible() const;
	void MoveGardenToBloomDate() const;
	void ApplyPlacementSchedule(APlant* PlantActor, const FString& PlantedDate) const;
public:
	UFUNCTION(BlueprintCallable)
	bool SpawnPlant(APlant*& Plant);
#pragma endregion

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void SetSelectedPlantData(UPlantObject* InPlantData);

	UFUNCTION(BlueprintCallable)
	void SetGardenEditMode(EGardenEditMode NewMode);

	UFUNCTION(BlueprintCallable)
	void SetPaintLayerByName(FName LayerName);

	UFUNCTION(BlueprintCallable)
	FName GetSelectedPaintLayerName() const;

	UFUNCTION(BlueprintCallable)
	void SetPaintBrushRadius(float NewRadius);

	UFUNCTION(BlueprintCallable)
	float GetPaintBrushRadius() const { return PaintBrushRadius; }

	
	UFUNCTION(BlueprintCallable)
	FString ExportPaintMaskData() const;

	UFUNCTION(BlueprintCallable)
	bool ImportPaintMaskData(const FString& PaintMaskData);
void CancelActivePlantInteraction();

	UFUNCTION(BlueprintCallable)
	EGardenEditMode GetGardenEditMode() const { return CurrentEditMode; }

	TSubclassOf<APlant> GetDefaultPlantClass() const { return DefaultPlantClass; }
};
