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
	BuildWidgetTreeIfNeeded();
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

	CheckText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CheckText"));
	if (UHorizontalBoxSlot* CheckSlot = RowBox->AddChildToHorizontalBox(CheckText))
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

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TextBox->AddChildToVerticalBox(TitleText);

	DescriptionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("DescriptionText"));
	TextBox->AddChildToVerticalBox(DescriptionText);
}

void USaveDestinationEntry::RefreshVisuals()
{
	if (!WidgetTree) return;

	if (!RootBorder || !TitleText || !DescriptionText || !CheckText)
	{
		BuildWidgetTreeIfNeeded();
	}

	if (!RootBorder || !TitleText || !DescriptionText || !CheckText) return;

	CheckText->SetText(FText::FromString(bIsChecked ? TEXT("[x]") : TEXT("[ ]")));
	CheckText->SetColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.95f, 0.90f, 1.0f)));
	FSlateFontInfo CheckFont = CheckText->Font;
	CheckFont.Size = 18;
	CheckText->SetFont(CheckFont);

	TitleText->SetText(FText::FromString(Title));
	TitleText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	FSlateFontInfo TitleFont = TitleText->Font;
	TitleFont.Size = 22; //bIsGlobalDestination ? 22 : 22;
	TitleText->SetFont(TitleFont);

	DescriptionText->SetText(FText::FromString(Description));
	DescriptionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.76f, 0.84f, 1.0f)));
	FSlateFontInfo DescriptionFont = DescriptionText->Font;
	DescriptionFont.Size = 14; //bIsGlobalDestination ? 14 : 14;
	DescriptionText->SetFont(DescriptionFont);

	const FLinearColor BackgroundColor = bIsChecked
		? FLinearColor(0.10f, 0.25f, 0.22f, 0.96f)
		: FLinearColor(0.12f, 0.16f, 0.25f, 0.96f);

	RootBorder->SetBrushColor(BackgroundColor);
}
