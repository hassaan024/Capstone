#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "UserDrone.generated.h"

class APlant;
class AUserDroneController;
class UPlantObject;

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
	bool bDraggingRealPlant = false;
	FVector MouseDragStart;
	APlant* PreviewPlant = nullptr;
	FTransform DragOriginalTransform;
	EGardenEditMode CurrentEditMode = EGardenEditMode::Plant;
#pragma endregion

#pragma region Tick Functions
private:
	void UpdatePan();
	void UpdatePreviewPlant();
#pragma endregion

#pragma region Helper Functions
private:
	bool GetMouseGroundHit(FHitResult& OutHit);
	bool GetMousePlantHit(FHitResult& OutHit);
	bool ValidPlantPlacement();
	void TrackPlacedPlant(APlant* PlantActor);
	void DeletePlant(APlant* PlantActor);
	void CancelActivePlantInteraction();
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
	EGardenEditMode GetGardenEditMode() const { return CurrentEditMode; }

	TSubclassOf<APlant> GetDefaultPlantClass() const { return DefaultPlantClass; }
};
