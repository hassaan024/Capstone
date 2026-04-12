// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GardenExit.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API UGardenExit : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual bool Initialize() override;

    UFUNCTION()
    void OnPressSave();

    UFUNCTION()
    void OnPressExit();

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_Save;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_Exit;
};
