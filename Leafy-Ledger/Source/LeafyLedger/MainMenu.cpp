// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"

bool UMainMenu::Initialize()
{
    if (!Super::Initialize()) return false;

    MenuController = Cast<AMenuController>(UGameplayStatics::GetPlayerController(this, 0));

    if (!MenuController)
    {
        return false;
    }

    if (BTN_UpdateDisplayName)
    {
        BTN_UpdateDisplayName->OnClicked.AddDynamic(this, &UMainMenu::OnPressUpdateDisplayName);
    }

    if (BTN_SavedPlants)
    {
        BTN_SavedPlants->OnClicked.AddDynamic(this, &UMainMenu::OnPressSavedPlants);
    }
    return true;
}

void UMainMenu::OnPressUpdateDisplayName()
{
    MenuController->ShowDisplayName();
}

void UMainMenu::OnPressSavedPlants()
{
    MenuController->ShowSavedPlants();
}