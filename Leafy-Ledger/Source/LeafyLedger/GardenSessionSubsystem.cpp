// Fill out your copyright notice in the Description page of Project Settings.

#include "GardenSessionSubsystem.h"

void UGardenSessionSubsystem::StartNewGardenDraft(const FString& Name, const FString& Description)
{
	Draft = FEditableGardenState{};
	Draft.Name = Name;
	Draft.Description = Description;
	Draft.bHasUnsavedChanges = true;
	Draft.bIsInitialized = true;
}

void UGardenSessionSubsystem::LoadGardenDraft(const FBackendGardenDetailDto& Garden)
{
	Draft = FEditableGardenState{};
	Draft.BackendGardenId = Garden.Id;
	Draft.Name = Garden.Name;
	Draft.Description = Garden.Description;
	Draft.Latitude = Garden.Latitude;
	Draft.Longitude = Garden.Longitude;
	Draft.Timezone = Garden.Timezone;
	Draft.bIsInitialized = true;
	Draft.bHasUnsavedChanges = false;

	for (int32 Index = 0; Index < Garden.Plants.Num(); ++Index)
	{
		const FBackendGardenPlantInstanceDto& SourcePlant = Garden.Plants[Index];

		FEditablePlantPlacement Plant;
		Plant.LocalId = FGuid::NewGuid();
		Plant.BackendPlantInstanceId = SourcePlant.Id;
		Plant.SpeciesId = SourcePlant.SpeciesId;
		Plant.SoilId = SourcePlant.SoilId;
		Plant.PerenualId = SourcePlant.Species.PerenualId;
		Plant.SpeciesCommonName = SourcePlant.Species.CommonName;
		Plant.SpeciesScientificName = SourcePlant.Species.ScientificName;
		Plant.SpeciesModelCategory = SourcePlant.Species.ModelCategory;
		Plant.Location = SourcePlant.Location;
		Plant.Rotation = SourcePlant.Rotation;
		Plant.Scale = SourcePlant.Scale;
		Plant.HeightCm = SourcePlant.HeightCm;
		Plant.AgeDays = SourcePlant.AgeDays;
		Plant.HealthStatus = SourcePlant.HealthStatus;
		Plant.LastWateredIso8601 = SourcePlant.LastWatered;
		Plant.Notes = SourcePlant.Notes;
		Plant.bPendingCreate = false;
		Plant.bPendingUpdate = false;
		Plant.bPendingDelete = false;

		Draft.Plants.Add(MoveTemp(Plant));
	}
}

void UGardenSessionSubsystem::ClearDraft()
{
	Draft = FEditableGardenState{};
}

bool UGardenSessionSubsystem::HasActiveDraft() const
{
	return Draft.bIsInitialized;
}

const FEditableGardenState& UGardenSessionSubsystem::GetDraft() const
{
	return Draft;
}

void UGardenSessionSubsystem::SetGardenLocation(float Latitude, float Longitude, const FString& Timezone)
{
	if (!Draft.bIsInitialized) return;

	Draft.Latitude = Latitude;
	Draft.Longitude = Longitude;
	Draft.Timezone = Timezone;
	Draft.bHasUnsavedChanges = true;
}

void UGardenSessionSubsystem::MarkDirty()
{
	if (!Draft.bIsInitialized) return;

	Draft.bHasUnsavedChanges = true;
}

FGuid UGardenSessionSubsystem::AddPlantPlacement(
	int32 SpeciesId,
	int32 SoilId,
	const FVector& Location,
	const FRotator& Rotation,
	const FVector& Scale,
	float HeightCm,
	int32 AgeDays,
	const FString& HealthStatus,
	const FString& LastWateredIso8601,
	const FString& Notes
)
{
	if (!Draft.bIsInitialized)
	{
		return FGuid();
	}

	FEditablePlantPlacement Plant;
	Plant.LocalId = FGuid::NewGuid();
	Plant.SpeciesId = SpeciesId;
	Plant.SoilId = SoilId;
	Plant.Location = Location;
	Plant.Rotation = Rotation;
	Plant.Scale = Scale;
	Plant.HeightCm = HeightCm;
	Plant.AgeDays = AgeDays;
	Plant.HealthStatus = HealthStatus;
	Plant.LastWateredIso8601 = LastWateredIso8601;
	Plant.Notes = Notes;
	Plant.bPendingCreate = true;

	Draft.Plants.Add(Plant);
	Draft.bHasUnsavedChanges = true;

	return Plant.LocalId;
}

bool UGardenSessionSubsystem::UpdatePlantTransform(
	const FGuid& LocalId,
	const FVector& Location,
	const FRotator& Rotation,
	const FVector& Scale
)
{
	const int32 Index = FindPlantIndexByLocalId(LocalId);
	if (Index == INDEX_NONE) return false;

	FEditablePlantPlacement& Plant = Draft.Plants[Index];
	Plant.Location = Location;
	Plant.Rotation = Rotation;
	Plant.Scale = Scale;

	if (!Plant.bPendingCreate)
	{
		Plant.bPendingUpdate = true;
	}

	Draft.bHasUnsavedChanges = true;
	return true;
}

bool UGardenSessionSubsystem::RemovePlantPlacement(const FGuid& LocalId)
{
	const int32 Index = FindPlantIndexByLocalId(LocalId);
	if (Index == INDEX_NONE) return false;

	FEditablePlantPlacement& Plant = Draft.Plants[Index];

	if (Plant.BackendPlantInstanceId <= 0 && Plant.bPendingCreate)
	{
		Draft.Plants.RemoveAt(Index);
	}
	else
	{
		Plant.bPendingDelete = true;
		Plant.bPendingUpdate = false;
	}

	Draft.bHasUnsavedChanges = true;
	return true;
}

bool UGardenSessionSubsystem::SetPlantNotes(const FGuid& LocalId, const FString& Notes)
{
	const int32 Index = FindPlantIndexByLocalId(LocalId);
	if (Index == INDEX_NONE) return false;

	FEditablePlantPlacement& Plant = Draft.Plants[Index];
	Plant.Notes = Notes;

	if (!Plant.bPendingCreate)
	{
		Plant.bPendingUpdate = true;
	}

	Draft.bHasUnsavedChanges = true;
	return true;
}

int32 UGardenSessionSubsystem::FindPlantIndexByLocalId(const FGuid& LocalId) const
{
	for (int32 i = 0; i < Draft.Plants.Num(); ++i)
	{
		if (Draft.Plants[i].LocalId == LocalId)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

void UGardenSessionSubsystem::RefreshDirtyState()
{
	if (!Draft.bIsInitialized)
	{
		Draft.bHasUnsavedChanges = false;
		return;
	}

	const bool bGardenSaved = Draft.BackendGardenId > 0;
	bool bAnyPendingPlantChanges = false;

	for (const FEditablePlantPlacement& Plant : Draft.Plants)
	{
		if (Plant.bPendingCreate || Plant.bPendingUpdate || Plant.bPendingDelete)
		{
			bAnyPendingPlantChanges = true;
			break;
		}
	}

	Draft.bHasUnsavedChanges = !bGardenSaved || bAnyPendingPlantChanges;
}

bool UGardenSessionSubsystem::IsDirty() const
{
	return Draft.bIsInitialized && Draft.bHasUnsavedChanges;
}

FEditableGardenState UGardenSessionSubsystem::GetDraftCopy() const
{
	return Draft;
}

void UGardenSessionSubsystem::MarkGardenSaved(int32 InBackendGardenId)
{
	if (!Draft.bIsInitialized) return;

	Draft.BackendGardenId = InBackendGardenId;
	RefreshDirtyState();
}

void UGardenSessionSubsystem::MarkPlantSaved(const FGuid& LocalId, int32 InBackendPlantInstanceId)
{
	const int32 Index = FindPlantIndexByLocalId(LocalId);
	if (Index == INDEX_NONE) return;

	FEditablePlantPlacement& Plant = Draft.Plants[Index];
	Plant.BackendPlantInstanceId = InBackendPlantInstanceId;
	Plant.bPendingCreate = false;
	Plant.bPendingUpdate = false;
	Plant.bPendingDelete = false;
	RefreshDirtyState();
}
