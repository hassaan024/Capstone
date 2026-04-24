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

	if (BTN_SearchSpecies)
	{
		BTN_SearchSpecies->OnClicked.AddDynamic(this, &UBrowseSpecies::OnPressSearchSpecies);
	}

	if (BTN_Back)
	{
		BTN_Back->OnClicked.AddDynamic(this, &UBrowseSpecies::OnPressBack);
	}

	return true;
}

void UBrowseSpecies::NativeConstruct()
{
	Super::NativeConstruct();
	//ResolveWidgetReferences();

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
			if (UButton* Button = Cast<UButton>(Widget))
			{
				if (Button->GetName().Contains(TEXT("BTN_SubmitSpecies")))
				{
					BTN_SearchSpecies = Button;
				}
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
		UPlantObject* PlantObject = NewObject<UPlantObject>(this);
		if (!PlantObject)
		{
			continue;
		}

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