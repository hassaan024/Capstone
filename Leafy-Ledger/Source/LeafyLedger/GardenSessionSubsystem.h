// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BackendApiTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GardenSessionSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FEditablePlantPlacement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FGuid LocalId;

	UPROPERTY(BlueprintReadWrite)
	int32 BackendPlantInstanceId = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 SpeciesId = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 SoilId = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 PerenualId = 0;

	UPROPERTY(BlueprintReadWrite)
	FString SpeciesCommonName;

	UPROPERTY(BlueprintReadWrite)
	FString SpeciesScientificName;

	UPROPERTY(BlueprintReadWrite)
	FString SpeciesModelCategory;

	UPROPERTY(BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite)
	FVector Scale = FVector::OneVector;

	UPROPERTY(BlueprintReadWrite)
	float HeightCm = 0.f;

	UPROPERTY(BlueprintReadWrite)
	int32 AgeDays = 0;

	UPROPERTY(BlueprintReadWrite)
	FString HealthStatus;

	UPROPERTY(BlueprintReadWrite)
	FString LastWateredIso8601;

	UPROPERTY(BlueprintReadWrite)
	FString Notes;

	UPROPERTY(BlueprintReadWrite)
	bool bPendingCreate = false;

	UPROPERTY(BlueprintReadWrite)
	bool bPendingUpdate = false;

	UPROPERTY(BlueprintReadWrite)
	bool bPendingDelete = false;
};

USTRUCT(BlueprintType)
struct FEditableGardenState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 BackendGardenId = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FString Description;

	UPROPERTY(BlueprintReadWrite)
	float Latitude = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float Longitude = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FString Timezone;

	UPROPERTY(BlueprintReadWrite)
	TArray<FEditablePlantPlacement> Plants;

	UPROPERTY(BlueprintReadWrite)
	bool bHasUnsavedChanges = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsInitialized = false;
};

UCLASS()
class LEAFYLEDGER_API UGardenSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void StartNewGardenDraft(const FString& Name, const FString& Description);

	void LoadGardenDraft(const FBackendGardenDetailDto& Garden);

	UFUNCTION(BlueprintCallable)
	void ClearDraft();

	UFUNCTION(BlueprintCallable)
	bool HasActiveDraft() const;

	UFUNCTION(BlueprintCallable)
	const FEditableGardenState& GetDraft() const;

	UFUNCTION(BlueprintCallable)
	void SetGardenLocation(float Latitude, float Longitude, const FString& Timezone);

	UFUNCTION(BlueprintCallable)
	void MarkDirty();

	UFUNCTION(BlueprintCallable)
	bool IsDirty() const;

	UFUNCTION(BlueprintCallable)
	FEditableGardenState GetDraftCopy() const;

	UFUNCTION(BlueprintCallable)
	void MarkGardenSaved(int32 InBackendGardenId);

	UFUNCTION(BlueprintCallable)
	void MarkPlantSaved(const FGuid& LocalId, int32 InBackendPlantInstanceId);

	FGuid AddPlantPlacement(
		int32 SpeciesId,
		int32 SoilId,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& Scale,
		float HeightCm,
		int32 AgeDays,
		const FString& HealthStatus,
		const FString& LastWateredIso8601,
		const FString& Notes = TEXT("")
	);

	UFUNCTION(BlueprintCallable)
	bool UpdatePlantTransform(
		const FGuid& LocalId,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& Scale
	);

	UFUNCTION(BlueprintCallable)
	bool RemovePlantPlacement(const FGuid& LocalId);

	UFUNCTION(BlueprintCallable)
	bool SetPlantNotes(const FGuid& LocalId, const FString& Notes);

private:
	UPROPERTY()
	FEditableGardenState Draft;

	int32 FindPlantIndexByLocalId(const FGuid& LocalId) const;
	void RefreshDirtyState();
};
