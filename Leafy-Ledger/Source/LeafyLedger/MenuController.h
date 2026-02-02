// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "MenuController.generated.h"

UCLASS()
class LEAFYLEDGER_API AMenuController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    UFUNCTION()
    void ShowLogin();

    UFUNCTION()
    void ShowMainMenu();

    UFUNCTION()
    void ShowDisplayName();

    UFUNCTION()
    void HandleLoginFailed(const FString& Error);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> LoginWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> DisplayNameWidgetClass;

    UPROPERTY(Transient)
    UUserWidget* CurrentWidget = nullptr;


private:
    void SetRootWidget(TSubclassOf<UUserWidget> WidgetClass);

    bool DisplayNameWidgetShown = false;
};
