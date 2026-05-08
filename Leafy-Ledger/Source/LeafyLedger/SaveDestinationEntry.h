#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveDestinationEntry.generated.h"

class UBorder;
class UHorizontalBox;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveDestinationClicked, bool, bIsGlobalDestination, int32, GardenId);

UCLASS()
class LEAFYLEDGER_API USaveDestinationEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnSaveDestinationClicked OnDestinationClicked;

	void Configure(bool bInIsGlobalDestination, int32 InGardenId, const FString& InTitle, const FString& InDescription, bool bInIsChecked);
	void SetChecked(bool bInIsChecked);

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void BuildWidgetTreeIfNeeded();
	void RefreshVisuals();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	UBorder* RootBorder = nullptr;

	UPROPERTY()
	UHorizontalBox* RowBox = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	UTextBlock* TXT_Check = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	UTextBlock* TXT_Title = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional, AllowPrivateAccess = "true"))
	UTextBlock* TXT_Description = nullptr;

	bool bIsGlobalDestination = false;
	int32 GardenId = 0;
	bool bIsChecked = false;
	FString Title;
	FString Description;
};
