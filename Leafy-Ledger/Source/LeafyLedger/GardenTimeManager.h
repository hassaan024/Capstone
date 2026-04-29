#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GardenTimeManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGardenDayChanged, int32, NewDayIndex);

UCLASS()
class LEAFYLEDGER_API AGardenTimeManager : public AActor
{
	GENERATED_BODY()

public:
	AGardenTimeManager();

	UPROPERTY(BlueprintReadOnly)
		int32 CurrentDayIndex;
	UPROPERTY(BlueprintReadOnly)
		int32 GlobalBloomDate;

	UPROPERTY(BlueprintAssignable)
		FOnGardenDayChanged OnDayChanged;

	UFUNCTION(BlueprintCallable)
		void SetCurrentDayIndex(int32 NewDayIndex);

	UFUNCTION(BlueprintCallable)
		int32 GetCurrentDayIndex() const;

	UFUNCTION(BlueprintCallable)
		int32 BloomDateToDayIndex(const FString& BloomDate) const;

	UFUNCTION(BlueprintCallable)
		bool SetCurrentDayIndexFromBloomDate(const FString& BloomDate);
};
