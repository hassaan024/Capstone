// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "GardenSessionSubsystem.h"
#include "GardenExit.generated.h"

class UBackendApiSubsystem;

UCLASS()
class LEAFYLEDGER_API UGardenExit : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual bool Initialize() override;

    UFUNCTION()
    void OnPressSave();

    UFUNCTION()
    void OnPressExit();

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_Save;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_Exit;

private:
    void SavePendingPlants(int32 GardenId, const TArray<FEditablePlantPlacement>& Plants, int32 StartIndex = 0);
};
