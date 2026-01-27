// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginWidget.h"
#include "OAuthGISubsystem.h" 
#include "Kismet/GameplayStatics.h"

bool ULoginWidget::Initialize()
{
    if (!Super::Initialize()) return false;

    if (BTN_Login)
    {
        BTN_Login->OnClicked.AddDynamic(this, &ULoginWidget::OnPress);
    }
    return true;
}

void ULoginWidget::OnPress()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UOAuthGISubsystem* Auth = GI->GetSubsystem<UOAuthGISubsystem>())
        {
            Auth->BeginGoogleLogin();
        }
    }
}
