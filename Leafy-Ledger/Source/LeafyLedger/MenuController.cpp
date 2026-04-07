// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuController.h"
#include "OAuthGISubsystem.h"
#include "SavedPlantCacheSubsystem.h"

void AMenuController::BeginPlay()
{
    Super::BeginPlay();

    UOAuthGISubsystem* Auth = GetGameInstance()->GetSubsystem<UOAuthGISubsystem>();
    if (!Auth)
    {
        ShowLogin();
        return;
    }

    Auth->OnLoginSucceeded.AddDynamic(this, &AMenuController::ShowMainMenu);
    Auth->OnLoginSucceeded.AddDynamic(this, &AMenuController::HandleLoginSucceededWarmCaches);
    Auth->OnLoginFailed.AddDynamic(this, &AMenuController::HandleLoginFailed);

    if (Auth->IsLoggedIn()) {
        UE_LOG(LogTemp, Warning, TEXT("ShowMainMenu Auth Logged In"));
        ShowMainMenu();
    }
    else {
        ShowLogin();
    }
}

void AMenuController::SetRootWidget(TSubclassOf<UUserWidget> WidgetClass)
{
    if (!WidgetClass)
    {
        //UE_LOG(LogTemp, Warning, TEXT("SetRootWidget called with null WidgetClass"));
        return;
    }

    if (CurrentWidget)
    {
        CurrentWidget->RemoveFromParent();
        CurrentWidget = nullptr;
    }

    CurrentWidget = CreateWidget<UUserWidget>(this, WidgetClass);
    if (!CurrentWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateWidget failed for %s"), *WidgetClass->GetName());
        return;
    }

    CurrentWidget->AddToViewport(0);

    bShowMouseCursor = true;

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(CurrentWidget->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
}

void AMenuController::ShowLogin()
{
    SetRootWidget(LoginWidgetClass);
}

void AMenuController::ShowMainMenu()
{
    if (OpenWebsite) {
        FPlatformProcess::LaunchURL(
            TEXT("http://localhost:5173/login"),
            nullptr,
            nullptr
        );
    }

    SetRootWidget(MainMenuWidgetClass);
}

void AMenuController::ShowDisplayName() 
{
    OpenWebsite = false;

    SetRootWidget(DisplayNameWidgetClass);
}

void AMenuController::ShowSavedPlants()
{
    OpenWebsite = false;

    SetRootWidget(SavedPlantsWidgetClass);
}



void AMenuController::HandleLoginFailed(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("Login failed: %s"), *Error);
}

void AMenuController::HandleLoginSucceededWarmCaches()
{
    if (!GetGameInstance())
    {
        return;
    }

    USavedPlantCacheSubsystem* PlantCache = GetGameInstance()->GetSubsystem<USavedPlantCacheSubsystem>();
    if (!PlantCache)
    {
        UE_LOG(LogTemp, Error, TEXT("SavedPlantCacheSubsystem not found"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Login succeeded: warming saved plant cache"));
    PlantCache->WarmAfterLogin();
}