// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"
#include "Components/Button.h"
#include "DisplayName.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API UDisplayName : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual bool Initialize() override;

    UFUNCTION()
    void OnPressGoogleName();

    UFUNCTION()
    void OnPressSubmit();

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_GoogleName;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_SubmitName;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString BackendBaseUrl = TEXT("http://localhost:4000/backend/user");

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString id;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FString googleDisplayName;

    //UPROPERTY(BlueprintReadWrite, EditAnywhere)
    //FString BearerToken;

    UPROPERTY(meta = (BindWidgetOptional))
    class UEditableTextBox* ET_DisplayName;

private:
    void OnPatchUserComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
