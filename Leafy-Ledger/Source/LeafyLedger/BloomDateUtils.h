// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FBloomDateValidationResult
{
	bool bIsValid = false;
	FDateTime Date = FDateTime::MinValue();
	FString DisplayText;
	FString BackendText;
	FString ErrorMessage;
};

struct FBloomDateUtils
{
	static FBloomDateValidationResult ValidateUserInput(const FString& InputText, bool bAllowEmpty = false);
	static FString FormatForDisplay(const FDateTime& Date);
	static FString FormatForBackend(const FDateTime& Date);
	static FString NormalizeBackendDateString(const FString& InputText);
	static FString DayIndexToDisplayDate(int32 DayIndex);

private:
	static bool TryParseDisplayDate(const FString& InputText, FDateTime& OutDate);
	static bool TryParseIsoDate(const FString& InputText, FDateTime& OutDate);
	static bool IsDateInPast(const FDateTime& Date);
	static bool IsDigitsOnly(const FString& InputText);
};
