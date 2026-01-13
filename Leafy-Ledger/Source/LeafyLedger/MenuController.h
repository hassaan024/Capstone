// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "OAuthGISubsystem.h"
#include "MenuController.generated.h"

/**
 * 
 */
UCLASS()
class LEAFYLEDGER_API AMenuController : public APlayerController
{
	GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    void ShowLogin();
    void ShowMainMenu();
    void HandleLoginFailed(const FString& Error);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> LoginWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(Transient)
    UUserWidget* CurrentWidget = nullptr;

private:
    void SetRootWidget(TSubclassOf<UUserWidget> WidgetClass);
};
