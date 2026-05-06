#include "GardenTimeManager.h"
#include "BloomDateUtils.h"

AGardenTimeManager::AGardenTimeManager()
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentDayIndex = 0;
}

void AGardenTimeManager::SetCurrentDayIndex(int32 NewDayIndex)
{
	if (NewDayIndex == CurrentDayIndex) return;
	CurrentDayIndex = NewDayIndex;
	OnDayChanged.Broadcast(CurrentDayIndex);
}

void AGardenTimeManager::RefreshCurrentDay()
{
	OnDayChanged.Broadcast(CurrentDayIndex);
}

int32 AGardenTimeManager::GetCurrentDayIndex() const
{
	return CurrentDayIndex;
}

int32 AGardenTimeManager::BloomDateToDayIndex(const FString& BloomDate) const
{
	const FString NormalizedBloomDate = FBloomDateUtils::NormalizeBackendDateString(BloomDate);
	if (NormalizedBloomDate.IsEmpty())
	{
		return -1;
	}

	FDateTime ParsedBloomDate;
	if (!FDateTime::ParseIso8601(*NormalizedBloomDate, ParsedBloomDate))
	{
		return -1;
	}

	const FDateTime BloomDateOnly(ParsedBloomDate.GetYear(), ParsedBloomDate.GetMonth(), ParsedBloomDate.GetDay());
	const FDateTime EpochDate(2000, 1, 1);
	const FTimespan DaysSinceEpoch = BloomDateOnly - EpochDate;
	const int32 DayIndex = static_cast<int32>(DaysSinceEpoch.GetDays());

	UE_LOG(LogTemp, Warning, TEXT("DayIndex: %d"), DayIndex);

	return DayIndex >= 0 ? DayIndex : -1;
}

bool AGardenTimeManager::SetCurrentDayIndexFromBloomDate(const FString& BloomDate)
{
	const int32 DayIndex = BloomDateToDayIndex(BloomDate);
	if (DayIndex < 0)
	{
		return false;
	}

	SetCurrentDayIndex(DayIndex);
	GlobalBloomDate = DayIndex;
	return true;
}
