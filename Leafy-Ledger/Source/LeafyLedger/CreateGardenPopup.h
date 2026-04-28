// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BackendApiTypes.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "CreateGardenPopup.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreateGardenPopupGardenCreated);

enum class ECreateGardenLocationSource : uint8
{
	None,
	BackendUserLocation,
	ZipCode
};

enum class ECreateGardenSubmitAction : uint8
{
	None,
	CreateOnly,
	StartPlanting
};

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API UCreateGardenPopup : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintAssignable, Category = "Garden")
	FOnCreateGardenPopupGardenCreated OnGardenCreated;

	UFUNCTION()
	void OnPressCreate();

	UFUNCTION()
	void OnPressStartPlanting();

	UFUNCTION()
	void OnPressUseLocation();

	UFUNCTION()
	void HandleLocationTextChanged(const FText& NewText);

	UFUNCTION()
	void HandleGardenLocationResponse(bool bSuccess, const FString& Message, const FBackendUserLocationDto& Location);

	UFUNCTION()
	void HandleCreateGardenLocationResponse(bool bSuccess, const FString& Message, const FBackendUserLocationDto& Location);

	void HandleZipLocationResponse(bool bSuccess, const FString& Message, float Latitude, float Longitude);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_Create;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_StartPlanting;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_UseLocation;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ET_GardenName;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ET_GardenDesc;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ET_BloomDate;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ET_Location;

private:
	void EnsureBloomDateField();
	bool ReadValidatedGardenInput(FString& OutGardenName, FString& OutGardenDesc, FString& OutBloomDate);
	bool ReadValidatedZipCode(FString& OutZipCode);
	void ResolveSelectedLocation(ECreateGardenSubmitAction SubmitAction);
	void ContinueWithResolvedLocation(float Latitude, float Longitude, const FString& Timezone);
	void CreateGardenFromInput(float Latitude, float Longitude, const FString& Timezone);

	FString PendingCreateGardenName;
	FString PendingCreateGardenDesc;
	FString PendingCreateBloomDate;
	ECreateGardenLocationSource SelectedLocationSource = ECreateGardenLocationSource::None;
	ECreateGardenSubmitAction PendingSubmitAction = ECreateGardenSubmitAction::None;
};
