#include "CustomUICommands/XGManageUICommands.h"


#define LOCTEXT_NAMESPACE "FXGManageModule"

void FXGManageUICommands::RegisterCommands()
{
	UI_COMMAND(
		LockActorSelection,
		"Lock Actor Selection",
		"Lock actor selection iv level, once triggered,actor can no longer be selected",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::W, EModifierKey::Alt));

	UI_COMMAND(
		UnlockActorSelection,
		"Unlock Actor Selection",
		"Remove selection lock on all actors",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::X, EModifierKey::Alt|EModifierKey::Shift));





}

#undef LOCTEXT_NAMESPACE