// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "BackendApiTypes.h"
#include "MenuController.h"
#include "DisplayName.generated.h"

class UEditableTextBox;

UCLASS()
class LEAFYLEDGER_API UDisplayName : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnPressGoogleName();

	UFUNCTION()
	void OnPressSubmit();

	UFUNCTION()
	void OnPressBack();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_GoogleName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_SubmitName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_Back;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ET_DisplayName;

	AMenuController* MenuController;

private:
	void SubmitDisplayName(const FString& NewName);
	void HandleUpdateDisplayNameResponse(bool bSuccess, const FString& Message);
	void HandleGetCurrentUserResponse(bool bSuccess, const FString& Message, const FBackendUserDto& User);

private:
	FBackendUserDto CurrentUser;
	bool bHasCurrentUser = false;
};