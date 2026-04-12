// Fill out your copyright notice in the Description page of Project Settings.

#include "GardenExit.h"

bool UGardenExit::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_Save)
    {
        BTN_Save->OnClicked.AddDynamic(this, &UGardenExit::OnPressSave);
    }

    if (BTN_Exit)
    {
        BTN_Exit->OnClicked.AddDynamic(this, &UGardenExit::OnPressExit);
    }

    return true;
}

void UGardenExit::OnPressSave()
{

}

void UGardenExit::OnPressExit()
{

}