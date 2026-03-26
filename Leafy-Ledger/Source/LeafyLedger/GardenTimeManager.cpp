#include "GardenTimeManager.h"

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

int32 AGardenTimeManager::GetCurrentDayIndex() const
{
	return CurrentDayIndex;
}