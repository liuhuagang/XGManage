#pragma once

#include "Styling/SlateStyle.h"

class FXGManagerStyle
{
public:

	static void InitialzeIcons();
	static void ShutDown();

private:

	static	FName StyleSetName;

	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet();

	static TSharedPtr<FSlateStyleSet> CreatedSlateStyleSet;
public:
	static FName GetStyleSetName() {
		return StyleSetName;
	}

	static TSharedRef<FSlateStyleSet> GetCreatedSlateStyleSet()
	{
		return CreatedSlateStyleSet.ToSharedRef();
	};
};