// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackendApiTypes.h"
#include "ManageSave.generated.h"

class UButton;
class UScrollBox;
class UTextBlock;
class USaveDestinationEntry;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManageSaveApplied, int32, PerenualId, bool, bIsGloballySaved);

UCLASS()
class LEAFYLEDGER_API UManageSave : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnManageSaveApplied OnSaveApplied;

	void ConfigureForPlant(int32 InPerenualId, const FString& InCommonName);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPressClose();

	UFUNCTION()
	void HandleDestinationClicked(bool bIsGlobalDestination, int32 GardenId);

	UFUNCTION()
	void OnPressApplyChanges();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_CloseCard = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_Changes = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TXT_Changes;

private:
	void ResolveWidgetReferences();
	void UpdatePlantNameText() const;
	void RefreshDestinationList();
	void PopulateDestinationList();
	void AddMessageRow(const FString& Message);
	void HandleGlobalStateResponse(bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants);
	void HandleGardenListResponse(bool bSuccess, const FString& Message, const TArray<FBackendGardenSummaryDto>& Gardens);
	void RequestGardenStateAtIndex(int32 GardenIndex);
	void HandleGardenStateResponse(int32 GardenId, bool bSuccess, const FString& Message, const TArray<FBackendPlantDto>& Plants);
	bool HasPendingChanges() const;
	void UpdateChangesText();
	void UpdateExistingRowStates();

	UPROPERTY()
	UScrollBox* SaveOptionsScrollBox = nullptr;

	UPROPERTY()
	UTextBlock* CommonNameText = nullptr;

	UPROPERTY()
	TArray<FBackendGardenSummaryDto> Gardens;

	UPROPERTY()
	TMap<int32, bool> GardenSaveStates;

	UPROPERTY()
	TMap<int32, bool> PendingGardenSaveStates;

	UPROPERTY()
	TMap<int32, USaveDestinationEntry*> GardenRows;

	UPROPERTY()
	USaveDestinationEntry* GlobalRow = nullptr;

	int32 PerenualId = 0;
	FString CommonName;
	bool bIsGloballySaved = false;
	bool bPendingGlobalSaved = false;
	bool bHasLoadedGardens = false;
	bool bIsApplyingToggle = false;
};
