#include "SaveDestinationEntry.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

void USaveDestinationEntry::Configure(bool bInIsGlobalDestination, int32 InGardenId, const FString& InTitle, const FString& InDescription, bool bInIsChecked)
{
	bIsGlobalDestination = bInIsGlobalDestination;
	GardenId = InGardenId;
	Title = InTitle;
	Description = InDescription;
	bIsChecked = bInIsChecked;

	RefreshVisuals();
}

void USaveDestinationEntry::SetChecked(bool bInIsChecked)
{
	bIsChecked = bInIsChecked;
	RefreshVisuals();
}

void USaveDestinationEntry::NativeConstruct()
{
	Super::NativeConstruct();
	if (!RootBorder || !TXT_Title || !TXT_Description || !TXT_Check)
	{
		BuildWidgetTreeIfNeeded();
	}
	RefreshVisuals();
}

FReply USaveDestinationEntry::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnDestinationClicked.Broadcast(bIsGlobalDestination, GardenId);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void USaveDestinationEntry::BuildWidgetTreeIfNeeded()
{
	if (!WidgetTree || WidgetTree->RootWidget) return;

	RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RootBorder"));
	WidgetTree->RootWidget = RootBorder;

	RowBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RowBox"));
	RootBorder->SetContent(RowBox);
	RootBorder->SetPadding(FMargin(16.0f, 12.0f));

	TXT_Check = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TXT_Check"));
	if (UHorizontalBoxSlot* CheckSlot = RowBox->AddChildToHorizontalBox(TXT_Check))
	{
		CheckSlot->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));
		CheckSlot->SetVerticalAlignment(VAlign_Center);
	}

	UVerticalBox* TextBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("TextBox"));
	if (UHorizontalBoxSlot* TextSlot = RowBox->AddChildToHorizontalBox(TextBox))
	{
		TextSlot->SetHorizontalAlignment(HAlign_Fill);
		TextSlot->SetVerticalAlignment(VAlign_Center);
	}

	TXT_Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TXT_Title"));
	TextBox->AddChildToVerticalBox(TXT_Title);

	TXT_Description = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TXT_Description"));
	TextBox->AddChildToVerticalBox(TXT_Description);
}

void USaveDestinationEntry::RefreshVisuals()
{
	if (!WidgetTree) return;

	if (!RootBorder || !TXT_Title || !TXT_Description || !TXT_Check)
	{
		BuildWidgetTreeIfNeeded();
	}

	if (!RootBorder || !TXT_Title || !TXT_Description || !TXT_Check) return;

	TXT_Check->SetText(FText::FromString(bIsChecked ? TEXT("[x]") : TEXT("[ ]")));
	TXT_Check->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.95f, 0.90f, 1.0f)));
	FSlateFontInfo CheckFont = TXT_Check->Font;
	CheckFont.Size = 18;
	TXT_Check->SetFont(CheckFont);

	TXT_Title->SetText(FText::FromString(Title));
	TXT_Title->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	FSlateFontInfo TitleFont = TXT_Title->Font;
	TitleFont.Size = 22; //bIsGlobalDestination ? 22 : 22;
	TXT_Title->SetFont(TitleFont);

	TXT_Description->SetText(FText::FromString(Description));
	TXT_Description->SetColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.76f, 0.84f, 1.0f)));
	FSlateFontInfo DescriptionFont = TXT_Description->Font;
	DescriptionFont.Size = 14; //bIsGlobalDestination ? 14 : 14;
	TXT_Description->SetFont(DescriptionFont);

	const FLinearColor BackgroundColor = bIsChecked
		? FLinearColor(0.10f, 0.25f, 0.22f, 0.96f)
		: FLinearColor(0.12f, 0.16f, 0.25f, 0.96f);

	RootBorder->SetBrushColor(BackgroundColor);
}
