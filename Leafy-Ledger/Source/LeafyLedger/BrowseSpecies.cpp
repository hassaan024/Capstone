#include "BrowseSpecies.h"
#include "BackendApiSubsystem.h"
#include "PlantObject.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TileView.h"

bool UBrowseSpecies::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	return true;
}

void UBrowseSpecies::NativeConstruct()
{
	Super::NativeConstruct();
	ResolveWidgetReferences();
	BindMenuButtons();

	//if (BTN_SubmitSpecies)
	//{
	//	BTN_SubmitSpecies->OnClicked.RemoveDynamic(this, &UBrowseSpecies::OnPressSearchSpecies);
	//	BTN_SubmitSpecies->OnClicked.AddDynamic(this, &UBrowseSpecies::OnPressSearchSpecies);
	//}
	//else
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("BrowseSpecies: BTN_SubmitSpecies was not resolved"));
	//}
}

void UBrowseSpecies::BindMenuButtons()
{
	if (UButton* SearchSpeciesButton = ResolveMenuButton(BTN_SearchSpecies))
	{
		SearchSpeciesButton->OnClicked.RemoveDynamic(this, &UBrowseSpecies::OnPressSearchSpecies);
		SearchSpeciesButton->OnClicked.AddDynamic(this, &UBrowseSpecies::OnPressSearchSpecies);
		UE_LOG(LogTemp, Log, TEXT("BrowseSpecies: bound BTN_SearchSpecies to inner button %s"), *SearchSpeciesButton->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BrowseSpecies: failed to resolve BTN_SearchSpecies as a clickable menu button"));
	}

	if (UButton* BackButton = ResolveMenuButton(BTN_Back))
	{
		BackButton->OnClicked.RemoveDynamic(this, &UBrowseSpecies::OnPressBack);
		BackButton->OnClicked.AddDynamic(this, &UBrowseSpecies::OnPressBack);
		UE_LOG(LogTemp, Log, TEXT("BrowseSpecies: bound BTN_Back to inner button %s"), *BackButton->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BrowseSpecies: failed to resolve BTN_Back as a clickable menu button"));
	}
}

UButton* UBrowseSpecies::ResolveMenuButton(UWidget* MenuButtonWidget) const
{
	if (!MenuButtonWidget)
	{
		return nullptr;
	}

	if (UButton* DirectButton = Cast<UButton>(MenuButtonWidget))
	{
		return DirectButton;
	}

	UUserWidget* MenuButtonUserWidget = Cast<UUserWidget>(MenuButtonWidget);
	if (!MenuButtonUserWidget || !MenuButtonUserWidget->WidgetTree)
	{
		return nullptr;
	}

	UButton* ResolvedButton = nullptr;
	MenuButtonUserWidget->WidgetTree->ForEachWidget([&ResolvedButton](UWidget* Widget)
	{
		if (!ResolvedButton)
		{
			ResolvedButton = Cast<UButton>(Widget);
		}
	});

	if (!ResolvedButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("BrowseSpecies: no inner UButton found in %s"), *MenuButtonWidget->GetName());
	}

	return ResolvedButton;
}

void UBrowseSpecies::OnPressSearchSpecies()
{
	if (!ET_Browse || !GetGameInstance())
	{
		UE_LOG(LogTemp, Warning, TEXT("BrowseSpecies: missing search box or game instance"));
		return;
	}

	//UBackendApiSubsystem* Api = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	//if (!Api) return;

	UBackendApiSubsystem* Api = GetGameInstance()->GetSubsystem<UBackendApiSubsystem>();
	if (!Api)
	{
		UE_LOG(LogTemp, Error, TEXT("Backend API subsystem not found"));
		return;
	}

	if (!ET_Browse->GetText().IsEmpty())
	{
		Api->SearchPerenualPlants(ET_Browse->GetText().ToString(), FBackendPlantSearchResponse::CreateUObject(this, &UBrowseSpecies::HandleSearchResponse));
	}
}

void UBrowseSpecies::ResolveWidgetReferences()
{
	if (!WidgetTree)
	{
		return;
	}

	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (!ET_Browse)
		{
			if (UEditableTextBox* EditableText = Cast<UEditableTextBox>(Widget))
			{
				if (EditableText->GetName().Contains(TEXT("ET_Browse")))
				{
					ET_Browse = EditableText;
				}
			}
		}

		if (!BTN_SearchSpecies)
		{
			if (Widget->GetName().Contains(TEXT("BTN_SearchSpecies")))
			{
				BTN_SearchSpecies = Widget;
			}
		}

		if (!BTN_Back)
		{
			if (Widget->GetName().Contains(TEXT("BTN_Back")))
			{
				BTN_Back = Widget;
			}
		}

		if (!TV_PlantCards)
		{
			if (UTileView* TileView = Cast<UTileView>(Widget))
			{
				if (TileView->GetName().Contains(TEXT("TV_PlantCards")))
				{
					TV_PlantCards = TileView;
				}
			}
		}
	});
}

void UBrowseSpecies::HandleSearchResponse(bool bSuccess, const FString& Message, const TArray<FBackendPlantSearchResultDto>& Plants)
{
	if (!TV_PlantCards)
	{
		UE_LOG(LogTemp, Warning, TEXT("BrowseSpecies: TV_PlantCards was not resolved"));
		return;
	}

	TV_PlantCards->ClearListItems();

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("BrowseSpecies search failed: %s"), *Message);
		return;
	}

	for (const FBackendPlantSearchResultDto& Plant : Plants)
	{
		if (Plant.CommonName.TrimStartAndEnd().IsEmpty()
			|| Plant.ScientificName.TrimStartAndEnd().IsEmpty()
			|| Plant.ImageUrl.TrimStartAndEnd().IsEmpty())
		{
			continue;
		}

		UPlantObject* PlantObject = NewObject<UPlantObject>(this);
		if (!PlantObject) continue;

		PlantObject->PerenualId = Plant.PerenualId;
		PlantObject->CommonName = Plant.CommonName;
		PlantObject->ScientificName = Plant.ScientificName;
		PlantObject->ImgSrcUrl = Plant.ImageUrl;
		TV_PlantCards->AddItem(PlantObject);
	}
}

void UBrowseSpecies::OnPressBack()
{
	MenuController = Cast<AMenuController>(UGameplayStatics::GetPlayerController(this, 0));
	if (MenuController)
	{
		MenuController->ShowMainMenu();
	}
}
