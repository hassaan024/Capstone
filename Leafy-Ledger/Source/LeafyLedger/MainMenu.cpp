// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenu.h"
#include "MenuController.h"

bool UMainMenu::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_UpdateDisplayName)
    {
        BTN_UpdateDisplayName->OnClicked.AddDynamic(this, &UMainMenu::OnPressUpdateDisplayName);
    }
    return true;
}

void UMainMenu::OnPressUpdateDisplayName()
{
    if (AMenuController* MenuController = Cast<AMenuController>(UGameplayStatics::GetPlayerController(this, 0))) 
    {
        MenuController->ShowDisplayName();
    }
}