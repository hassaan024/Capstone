// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "MainMenu.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* BTN_UpdateDisplayName;

	UFUNCTION()
	void OnPressUpdateDisplayName();
};
