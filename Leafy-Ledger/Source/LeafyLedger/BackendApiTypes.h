#pragma once

#include "CoreMinimal.h"

struct FBackendPlantDto
{
	FString CommonName;
	FString ScientificName;
	FString ImgSrcUrl;
	int32 TrefleId = 0;
};

struct FBackendUserDto
{
	int32 Id = 0;
	FString DisplayName;
	FString GoogleDisplayName;
	bool bConfirmedName = false;
};