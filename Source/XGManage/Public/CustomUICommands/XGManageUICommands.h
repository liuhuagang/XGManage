#pragma  once


#include "Framework/Commands/Commands.h"
#include "CoreMinimal.h"


class FXGManageUICommands : public TCommands<FXGManageUICommands>
{


public:
	FXGManageUICommands() :TCommands<FXGManageUICommands>(
		TEXT("XGManage"),
		FText::FromString(TEXT("XG Manage UI Commands")),
		NAME_None,
		TEXT("XGManage"))
	{


	}



	virtual void RegisterCommands() override ;
	TSharedPtr<FUICommandInfo> LockActorSelection;
	TSharedPtr<FUICommandInfo> UnlockActorSelection;

};