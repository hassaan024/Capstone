// Fill out your copyright notice in the Description page of Project Settings.

#include "BloomDateUtils.h"

namespace
{
	constexpr TCHAR DateSeparator = TEXT('/');
}

FBloomDateValidationResult FBloomDateUtils::ValidateUserInput(const FString& InputText, bool bAllowEmpty)
{
	FBloomDateValidationResult Result;
	const FString TrimmedInput = InputText.TrimStartAndEnd();

	if (TrimmedInput.IsEmpty())
	{
		if (bAllowEmpty)
		{
			Result.bIsValid = true;
		}
		else
		{
			Result.ErrorMessage = TEXT("Bloom date is required.");
		}

		return Result;
	}

	FDateTime ParsedDate;
	if (!TryParseDisplayDate(TrimmedInput, ParsedDate))
	{
		Result.ErrorMessage = TEXT("Use MM/DD/YYYY for the bloom date.");
		return Result;
	}

	if (IsDateInPast(ParsedDate))
	{
		Result.ErrorMessage = TEXT("Bloom date cannot be in the past.");
		return Result;
	}

	Result.bIsValid = true;
	Result.Date = ParsedDate;
	Result.DisplayText = FormatForDisplay(ParsedDate);
	Result.BackendText = FormatForBackend(ParsedDate);
	return Result;
}

FString FBloomDateUtils::FormatForDisplay(const FDateTime& Date)
{
	return FString::Printf(TEXT("%02d/%02d/%04d"), Date.GetMonth(), Date.GetDay(), Date.GetYear());
}

FString FBloomDateUtils::FormatForBackend(const FDateTime& Date)
{
	return FString::Printf(
		TEXT("%04d-%02d-%02dT12:00:00.000Z"),
		Date.GetYear(),
		Date.GetMonth(),
		Date.GetDay()
	);
}

FString FBloomDateUtils::NormalizeBackendDateString(const FString& InputText)
{
	const FString TrimmedInput = InputText.TrimStartAndEnd();
	if (TrimmedInput.IsEmpty())
	{
		return TEXT("");
	}

	FDateTime ParsedDate;
	if (TryParseDisplayDate(TrimmedInput, ParsedDate) || TryParseIsoDate(TrimmedInput, ParsedDate))
	{
		return FormatForBackend(ParsedDate);
	}

	return TrimmedInput;
}

FString FBloomDateUtils::DayIndexToDisplayDate(int32 DayIndex)
{
	if (DayIndex < 0)
	{
		return TEXT("");
	}

	const FDateTime EpochDate(2000, 1, 1);
	const FDateTime DisplayDate = EpochDate + FTimespan::FromDays(DayIndex);
	return FormatForDisplay(DisplayDate);
}

bool FBloomDateUtils::TryParseDisplayDate(const FString& InputText, FDateTime& OutDate)
{
	TArray<FString> Parts;
	InputText.ParseIntoArray(Parts, &DateSeparator, true);

	if (Parts.Num() != 3)
	{
		return false;
	}

	for (const FString& Part : Parts)
	{
		if (Part.IsEmpty() || !IsDigitsOnly(Part))
		{
			return false;
		}
	}

	const int32 Month = FCString::Atoi(*Parts[0]);
	const int32 Day = FCString::Atoi(*Parts[1]);
	const int32 Year = FCString::Atoi(*Parts[2]);

	if (Month < 1 || Month > 12 || Year < 1000 || Year > 9999)
	{
		return false;
	}

	const int32 MaxDay = FDateTime::DaysInMonth(Year, Month);
	if (Day < 1 || Day > MaxDay)
	{
		return false;
	}

	OutDate = FDateTime(Year, Month, Day);
	return true;
}

bool FBloomDateUtils::TryParseIsoDate(const FString& InputText, FDateTime& OutDate)
{
	const FString TrimmedInput = InputText.TrimStartAndEnd();

	if (TrimmedInput.Len() >= 10)
	{
		const FString DatePart = TrimmedInput.Left(10);
		if (DatePart.Len() == 10
			&& FChar::IsDigit(DatePart[0])
			&& FChar::IsDigit(DatePart[1])
			&& FChar::IsDigit(DatePart[2])
			&& FChar::IsDigit(DatePart[3])
			&& DatePart[4] == TEXT('-')
			&& FChar::IsDigit(DatePart[5])
			&& FChar::IsDigit(DatePart[6])
			&& DatePart[7] == TEXT('-')
			&& FChar::IsDigit(DatePart[8])
			&& FChar::IsDigit(DatePart[9]))
		{
			const int32 Year = FCString::Atoi(*DatePart.Mid(0, 4));
			const int32 Month = FCString::Atoi(*DatePart.Mid(5, 2));
			const int32 Day = FCString::Atoi(*DatePart.Mid(8, 2));

			if (Month < 1 || Month > 12 || Year < 1000 || Year > 9999)
			{
				return false;
			}

			const int32 MaxDay = FDateTime::DaysInMonth(Year, Month);
			if (Day < 1 || Day > MaxDay)
			{
				return false;
			}

			OutDate = FDateTime(Year, Month, Day);
			return true;
		}
	}

	FDateTime ParsedIsoDate;
	if (FDateTime::ParseIso8601(*TrimmedInput, ParsedIsoDate))
	{
		OutDate = FDateTime(ParsedIsoDate.GetYear(), ParsedIsoDate.GetMonth(), ParsedIsoDate.GetDay());
		return true;
	}

	FDateTime ParsedDate;
	if (FDateTime::Parse(TrimmedInput, ParsedDate))
	{
		OutDate = FDateTime(ParsedDate.GetYear(), ParsedDate.GetMonth(), ParsedDate.GetDay());
		return true;
	}

	return false;
}

bool FBloomDateUtils::IsDateInPast(const FDateTime& Date)
{
	const FDateTime Now = FDateTime::Now();
	const FDateTime Today(Now.GetYear(), Now.GetMonth(), Now.GetDay());
	return Date < Today;
}

bool FBloomDateUtils::IsDigitsOnly(const FString& InputText)
{
	for (const TCHAR Character : InputText)
	{
		if (!FChar::IsDigit(Character))
		{
			return false;
		}
	}

	return true;
}
