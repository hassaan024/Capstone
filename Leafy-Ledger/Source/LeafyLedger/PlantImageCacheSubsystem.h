// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Texture2D.h"
#include "PlantImageCacheSubsystem.generated.h"

DECLARE_DELEGATE_OneParam(FOnPlantImageReady, UTexture2D* /*Texture*/)

UCLASS()
class LEAFYLEDGER_API UPlantImageCacheSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void GetOrLoadImage(const FString& Url, const FOnPlantImageReady& Callback);

	UFUNCTION(BlueprintCallable, Category = "Plant Images")
	void ClearCache();

private:
	UPROPERTY()
	TMap<FString, UTexture2D*> TextureCache;

	TMap<FString, TArray<FOnPlantImageReady>> PendingCallbacks;

	void FinishWithFailure(const FString& Url);
	void FinishWithSuccess(const FString& Url, UTexture2D* Texture);
};