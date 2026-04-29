// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"
#include "Components/Button.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "BackendApiTypes.h"
#include "ManageSave.h"
#include "PlantCardPopup.generated.h"

class UTextBlock;
class UImage;
class UPlantObject;

UCLASS()
class LEAFYLEDGER_API UPlantCardPopup : public UUserWidget
{
	GENERATED_BODY()
	
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGlobalSaveRemoved, int32, PerenualId);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveStateChanged);

    UPROPERTY(BlueprintAssignable, Category = "Plants")
    FOnGlobalSaveRemoved OnGlobalSaveRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Plants")
    FOnSaveStateChanged OnSaveStateChanged;

    virtual bool Initialize() override;

    virtual void PopulateInfo(UObject* ListItemObject);

    UFUNCTION()
    void OnPressManageSave();

    UFUNCTION()
    void HandleManageSaveApplied(int32 PerenualId, bool bIsGloballySaved);

    void HandlePlantDetailsLoaded(bool bSuccess, const FString& Message, const FBackendPlantDto& Plant);

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UButton* BTN_ManageSave;

    UPROPERTY()
    TSubclassOf<UManageSave> ManageSaveClass;

    UPROPERTY()
    UManageSave* ManageSaveInstance = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout", meta = (AllowPrivateAccess = "true"))
    float DetailValueWrapTextAt = 220.0f;

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_CommonName;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ScientificName;

    UPROPERTY(meta = (BindWidget))
    UImage* IMG_Plant;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_Sunlight;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_Watering;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_Maintenance;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_Type;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_Hardiness;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_BloomDays;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_LifeCycle;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_GrowthRate;

private:
    UPROPERTY()
    UPlantObject* PlantCard = nullptr;

    FString CurrentUrl;

    void PopulateKnownDetails();
    void LoadPlantImage(const FString& ImageUrl);
    void ApplyPlantImageTexture(UTexture2D* Texture);
    void QueueDeferredPlantImageCrop(UTexture2D* Texture);
    void SetTextOrNA(UTextBlock* TextBlock, const FString& Value);
    void ConfigureDetailTextWrapping();
    void ApplyWrapTextAt(UTextBlock* TextBlock);
    void RefreshTextLayout();
    void RefreshTextLayoutNow();
    void QueueDeferredTextLayoutRefresh();
    void ApplyPlantDetails(const FBackendPlantDto& Plant);
    static FString GetPreferredImageUrl(const FBackendPlantDto& Plant);
};
