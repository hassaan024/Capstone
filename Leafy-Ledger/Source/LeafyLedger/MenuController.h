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
    void ShowSavedPlants();

    UFUNCTION()
    void HandleLoginFailed(const FString& Error);

    UFUNCTION()
    void HandleLoginSucceededWarmCaches();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> LoginWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> DisplayNameWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> SavedPlantsWidgetClass;

    UPROPERTY(Transient)
    UUserWidget* CurrentWidget = nullptr;


private:
    void SetRootWidget(TSubclassOf<UUserWidget> WidgetClass);

    bool OpenWebsite = true;
};

/** 

create garden

1. select a garden that has plants saved to it (but not planted) OR
2. start from scratch and have all plants available

1. read from backend and add the plants saved in that JSON or whatever to the plant select dropdown
2. read from the backend but add every plant there to the plant select dropdown
*/ 