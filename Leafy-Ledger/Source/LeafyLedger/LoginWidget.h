// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "LoginWidget.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API ULoginWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual bool Initialize() override;

protected:
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_Login;

    UFUNCTION()
    void OnPress();
};
