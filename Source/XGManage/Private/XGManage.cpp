// Copyright Epic Games, Inc. All Rights Reserved.

#include "XGManage.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "CustomStyle/XGManageStyle.h"
#include "LevelEditor.h"
#include "Framework\MultiBox\MultiBoxExtender.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Engine/Selection.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "CustomUICommands/XGManageUICommands.h"
#include "SceneOutlinerModule.h"
#include "CustomOutLinerColumn/OutLinerSelectionColumn.h"


#define LOCTEXT_NAMESPACE "FXGManageModule"

void FXGManageModule::StartupModule()
{
	FXGManagerStyle::InitialzeIcons();

	InitCBMenuExtention();

	RegisterAdvanceDeletionTab();

	FXGManageUICommands::Register();

	InitCustomUICommands();


	InitLevelEditorExtention();

	InitCutomSelectionEvent();

	InitSceneOutlinerColumnExtension();
}

void FXGManageModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AdvanceDeletion"));

	FXGManagerStyle::ShutDown();

	FXGManageUICommands::Unregister();

	UnRegisterSceneOutlinerColumnExtension();


}
#pragma region ContentBrowserMenuExtension
void FXGManageModule::InitCBMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	//Get hold of all the menu extenders
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders =
		ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	/*
		FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
		CustomCBMenuDelegate.BindRaw(this, &FXGManageModule::CustomCBMenuExtender);
		ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);*/

		//We did custom delegate to all the existing delegates
	ContentBrowserModuleMenuExtenders.Add(
		FContentBrowserMenuExtender_SelectedPaths::CreateRaw
		(this, &FXGManageModule::CustomCBMenuExtender));







}
//to define the posititon for inserting menu entry
TSharedRef<FExtender> FXGManageModule::CustomCBMenuExtender(const TArray<FString>& InSelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (InSelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"),//Extention hook,
			EExtensionHook::After,//Inserting before or after
			TSharedPtr<FUICommandList>(),//Custom hot keys
			FMenuExtensionDelegate::CreateRaw(this, &FXGManageModule::AddCBMenuEntry));
		//Second binding,will define details for this menu entry

		FolderPathsSelected = InSelectedPaths;

	}


	return MenuExtender;
}
//define details for the scutom menu entry
void FXGManageModule::AddCBMenuEntry(class FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Unused Assets")),//Title text for menu entry 
		FText::FromString(TEXT("Safely delete all unused assets under folder")),//Tooltip text
		FSlateIcon(FXGManagerStyle::GetStyleSetName(), "ContentBrowser.DeleteUnusedAssets"),//Custom icon
		FExecuteAction::CreateRaw(this, &FXGManageModule::OnDeleteUnusedAssetButtonClicked)
		//The actual function to excute
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Empty Folders")),//Title text for menu entry 
		FText::FromString(TEXT("Safely delete all empty folders ")),//Tooltip text
		FSlateIcon(FXGManagerStyle::GetStyleSetName(), "ContentBrowser.DeleteEmptyFolders"),//Custom icon
		FExecuteAction::CreateRaw(this, &FXGManageModule::OnDeleteEmptyFoldersButtonClicked)
		//The actual function to excute
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Advance Delete")),//Title text for menu entry 
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),//Tooltip text
		FSlateIcon(FXGManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"),//Custom icon
		FExecuteAction::CreateRaw(this, &FXGManageModule::OnAdvanceDeletionButtonClicked)
		//The actual function to excute
	);


}

void FXGManageModule::OnDeleteUnusedAssetButtonClicked()
{
	if (ConstructedDockTab.IsValid())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please clode advance deletion tab before this operation"));

		return;
	}


	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder"));
		return;
	}
	DebugHeader::PrintLog(TEXT("Currently Selected folder :") + FolderPathsSelected[0]);

	TArray<FString> FoundAssetPathName =
		UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	//Whether there are assets under the folder
	if (FoundAssetPathName.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"), false);

		return;
	}

	EAppReturnType::Type ConfirmResult =
		DebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT("A total of ")
			+ FString::FromInt(FoundAssetPathName.Num())
			+ TEXT(" assets need to be checked. \nWoudle you like to processd?"), false);


	if (ConfirmResult == EAppReturnType::No)
	{
		return;
	}

	FixUpRedirectors();

	TArray<FAssetData> UnusedAssetDataArray;
	for (const FString& AssetPathName : FoundAssetPathName)
	{
		//Don't touch root folder
		if (AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__"))
			)
		{
			continue;

		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))
		{
			continue;
		}

		TArray<FString> AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		if (AssetReferencers.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);

			UnusedAssetDataArray.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetDataArray.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetDataArray);

	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"), false);
	}


}

void FXGManageModule::OnDeleteEmptyFoldersButtonClicked()
{
	if (ConstructedDockTab.IsValid())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please clode advance deletion tab before this operation"));

		return;
	}



	FixUpRedirectors();

	TArray<FString> FolderPathArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for (const FString& FolderPath : FolderPathArray)
	{
		if (FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("__ExternalActors__")) ||
			FolderPath.Contains(TEXT("__ExternalObjects__"))
			)
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);


		}


	}

	if (EmptyFoldersPathsArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder"), false);
		return;
	}

	EAppReturnType::Type ConfirmResult =
		DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,
			TEXT("Empty folders found in :\n")
			+ EmptyFolderPathsNames
			+ TEXT("\nWould you like to delete all?"), false);

	if (ConfirmResult == EAppReturnType::Cancel)
	{
		return;
	}
	for (const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		UEditorAssetLibrary::DeleteDirectory(EmptyFolderPathsNames) ?
			++Counter : DebugHeader::PrintLog(TEXT("Failed to delete " + EmptyFolderPath));


	}
	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfilly deleted ")
			+ FString::FromInt(Counter)
			+ TEXT("folders"));
	}

}

void FXGManageModule::OnAdvanceDeletionButtonClicked()
{
	FixUpRedirectors();


	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));


}

void FXGManageModule::FixUpRedirectors()
{

	TArray<UObjectRedirector*> RedirectorsToFixArray;
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassPaths.Add(UObjectRedirector::StaticClass()->GetClassPathName());

	TArray<FAssetData> OutRedirectors;


	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	if (OutRedirectors.Num() > 0)
	{

		for (const FAssetData& RedirectorData : OutRedirectors)
		{
			if (UObjectRedirector* RedirectorsToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
			{

				RedirectorsToFixArray.Add(RedirectorsToFix);


			}

		}

		FAssetToolsModule& AssetToolsModule =
			FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

		AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
	}




}



#pragma endregion
#pragma region LevelEditorMenuExtension
void FXGManageModule::InitLevelEditorExtention()
{
	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	TSharedRef<FUICommandList> ExistingLevelCommands = LevelEditorModule.GetGlobalLevelEditorActions();
	ExistingLevelCommands->Append(CustomUICommands.ToSharedRef());




	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelEditorMenuExtenders =
		LevelEditorModule.GetAllLevelViewportContextMenuExtenders();

	LevelEditorMenuExtenders.Add(
		FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateRaw(this, &FXGManageModule::CustomLevelEditorMenuExtender));


}

TSharedRef<FExtender> FXGManageModule::CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectActors)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());

	if (SelectActors.Num() > 0)
	{
		MenuExtender->AddMenuExtension
		(FName("ActorOptions"),
			EExtensionHook::Before,
			UICommandList,
			FMenuExtensionDelegate::CreateRaw(this, &FXGManageModule::AddLevelEditorMenuEntry));

	}

	return  MenuExtender;


}

void FXGManageModule::AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Lock Acotr Selection")),
		FText::FromString(TEXT("Prevent actor from being selected")),
		FSlateIcon(FXGManagerStyle::GetStyleSetName(), "LevelEditor.LockSelection"),
		FExecuteAction::CreateRaw(this, &FXGManageModule::OnLockActorSelectionButtonClicked));

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Unloack All Acotr Selection")),
		FText::FromString(TEXT("Remove the selection constraint on all actor")),
		FSlateIcon(FXGManagerStyle::GetStyleSetName(), "LevelEditor.UnlockDeletion"),
		FExecuteAction::CreateRaw(this, &FXGManageModule::OnUnlockActorSelectionButtonClicked));




}


void FXGManageModule::OnLockActorSelectionButtonClicked()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectActors = WeakEditorActorSubsystem->GetSelectedLevelActors();

	if (SelectActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No actor selected"));
		return;

	}
	FString CurrentLockedActorsNames = TEXT("Locked selection for :");
	for (AActor* SelectActor : SelectActors)
	{
		if (!SelectActor)
		{
			continue;
		}

		LockActorSelection(SelectActor);
		WeakEditorActorSubsystem->SetActorSelectionState(SelectActor, false);

		CurrentLockedActorsNames.Append(TEXT("\n"));
		CurrentLockedActorsNames.Append(SelectActor->GetActorLabel());

	}
	RefreshSceneOutliner();
	DebugHeader::ShowNotifyInfo(CurrentLockedActorsNames);


}

void FXGManageModule::OnUnlockActorSelectionButtonClicked()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}
	TArray<AActor*> AllActorsInLevel = WeakEditorActorSubsystem->GetAllLevelActors();

	TArray<AActor*> AllLockedActors;

	for (AActor* ActorInLevel : AllActorsInLevel)
	{
		if (!ActorInLevel)
		{
			continue;
		}

		if (CheckIsActorSelection(ActorInLevel))
		{
			AllLockedActors.Add(ActorInLevel);


		}

	}

	if (AllLockedActors.Num() == 0)
	{

		DebugHeader::ShowNotifyInfo(TEXT("No Selection locked actor current"));
		return;

	}
	FString UnLockedAvtorNames = TEXT("Lifted Select constraint for :");
	for (AActor* LoackedActor : AllLockedActors)
	{
		UnlockActorSelection(LoackedActor);
		UnLockedAvtorNames.Append(TEXT("\n"));
		UnLockedAvtorNames.Append(LoackedActor->GetActorLabel());


	}
	RefreshSceneOutliner();
	DebugHeader::ShowNotifyInfo(UnLockedAvtorNames);




}


#pragma endregion LevelEditorMenuExtension

#pragma  region SelectionLock


void FXGManageModule::InitCutomSelectionEvent()
{
	USelection* UserSelection = GEditor->GetSelectedActors();

	UserSelection->SelectObjectEvent.AddRaw(this, &FXGManageModule::OnActorSelected);

}





void FXGManageModule::OnActorSelected(UObject* SelectedObject)
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	if (AActor* SelectedActor = Cast<AActor>(SelectedObject))
	{
		if (CheckIsActorSelection(SelectedActor))
		{
			//Deselect actor right away

			WeakEditorActorSubsystem->SetActorSelectionState(SelectedActor, false);


		}

	}
}

void FXGManageModule::LockActorSelection(AActor* ActorToProcess)
{
	if (!ActorToProcess)
	{
		return;
	}
	if (!ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Add(FName("Locked"));
	}



}

void FXGManageModule::UnlockActorSelection(AActor* ActorToProcess)
{
	if (!ActorToProcess)
	{
		return;
	}
	if (ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Remove(FName("Locked"));
	}


}

void FXGManageModule::RefreshSceneOutliner()
{
	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	TSharedPtr<ISceneOutliner> SceneOutliner = LevelEditorModule.GetFirstLevelEditor()->GetSceneOutliner();

	if (SceneOutliner.IsValid())
	{
		SceneOutliner->FullRefresh();


	}

}

bool FXGManageModule::CheckIsActorSelection(AActor* ActorToProcess)
{
	if (!ActorToProcess)
	{
		return false;
	}
	return(ActorToProcess->ActorHasTag(FName("Locked")));

}

void FXGManageModule::ProcessLockingForOutliner(AActor* ActorToProcess, bool bShouldLock)
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}


	if (bShouldLock)
	{
		LockActorSelection(ActorToProcess);
		WeakEditorActorSubsystem->SetActorSelectionState(ActorToProcess, false);
		DebugHeader::ShowNotifyInfo(TEXT("Locked selection for:\n") + ActorToProcess->GetActorLabel());

	}
	else
	{
		UnlockActorSelection(ActorToProcess);

		DebugHeader::ShowNotifyInfo(TEXT("Removed selection lock for:\n") + ActorToProcess->GetActorLabel());
	}
}

#pragma endregion SelectionLock

#pragma region CustomEditorUICommands
void FXGManageModule::InitCustomUICommands()
{
	CustomUICommands = MakeShareable(new FUICommandList());

	CustomUICommands->MapAction(
		FXGManageUICommands::Get().LockActorSelection,
		FExecuteAction::CreateRaw(this, &FXGManageModule::OnSelectionLockHotKeyPressed));

	CustomUICommands->MapAction(
		FXGManageUICommands::Get().UnlockActorSelection,
		FExecuteAction::CreateRaw(this, &FXGManageModule::OnUnselectionLockHotKeyPressed));

}

void FXGManageModule::OnSelectionLockHotKeyPressed()
{
	DebugHeader::PrintLog(TEXT("Hot key working"));
	OnLockActorSelectionButtonClicked();


}



void FXGManageModule::OnUnselectionLockHotKeyPressed()
{
	OnUnlockActorSelectionButtonClicked();
}

#pragma endregion CustomEditorUICommands


#pragma endregion SceneOutlinerExtension

void FXGManageModule::InitSceneOutlinerColumnExtension()
{
	FSceneOutlinerModule& SceneOutlinerModule
		= FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));

	FSceneOutlinerColumnInfo SelectionLockColumnInfo(
		ESceneOutlinerColumnVisibility::Visible,
		1,
		FCreateSceneOutlinerColumn::CreateRaw(this, &FXGManageModule::OncreateSelectionLockColumn)
	);



	SceneOutlinerModule.RegisterDefaultColumnType<FOutLinerSelectionLockColumn>(SelectionLockColumnInfo);



}

TSharedRef<class ISceneOutlinerColumn> FXGManageModule::OncreateSelectionLockColumn(class ISceneOutliner& SceneOutliner)
{
	return    MakeShareable(new FOutLinerSelectionLockColumn(SceneOutliner));
}

void FXGManageModule::UnRegisterSceneOutlinerColumnExtension()
{
	FSceneOutlinerModule& SceneOutlinerModule
		= FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));

	SceneOutlinerModule.UnRegisterColumnType<FOutLinerSelectionLockColumn>();
}

#pragma endregion SceneOutlinerExtension



bool FXGManageModule::GetEditorActorSubsystem()
{
	if (!WeakEditorActorSubsystem.IsValid())
	{
		WeakEditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();

	}
	return WeakEditorActorSubsystem.IsValid();
}
#pragma  region CustomEditorTab



void FXGManageModule::RegisterAdvanceDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner
	(FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FXGManageModule::OnSpawnAdvancedDeletionTab))
		.SetDisplayName(FText::FromString(TEXT("Advance Deletion")))
		.SetIcon(FSlateIcon(FXGManagerStyle::GetStyleSetName(), "ContentBrowser.AdvanceDeletion"));


}

TSharedRef<SDockTab> FXGManageModule::OnSpawnAdvancedDeletionTab(const FSpawnTabArgs& InSpawnTabArgs)
{
	if (FolderPathsSelected.Num()==0)
	{
		return SNew(SDockTab).TabRole(ETabRole::NomadTab);
	}
	 ConstructedDockTab =

	 SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SAdvanceDeletionTab)
			.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
			.CurrentSelectedFolder(FolderPathsSelected[0])
		];
	 ConstructedDockTab->SetOnTabClosed(

		 SDockTab::FOnTabClosedCallback::CreateRaw(this, &FXGManageModule::OnAdvanceDeletionTabClosed)

	 );


	return ConstructedDockTab.ToSharedRef();
}

TArray<TSharedPtr<FAssetData>> FXGManageModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvaibaleAssetsData;

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	for (const FString& AssetPathName : AssetsPathNames)
	{
		//Don't touch root folder
		if (AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__"))
			)
		{
			continue;

		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))
		{
			continue;
		}

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);


		AvaibaleAssetsData.Add(MakeShared<FAssetData>(Data));


	}


	return AvaibaleAssetsData;
}

void FXGManageModule::OnAdvanceDeletionTabClosed(TSharedRef<SDockTab> TabToClose)
{
	if (ConstructedDockTab.IsValid())
	{
		ConstructedDockTab.Reset();
		FolderPathsSelected.Empty();
	}
}

#pragma  endregion
#pragma  region ProcessDataForAdvanceDeletionTab
bool FXGManageModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);



	return	ObjectTools::DeleteAssets(AssetDataForDeletion) > 0;
}

bool FXGManageModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsToDelete) > 0)
	{
		return true;

	}
	return false;
}

void FXGManageModule::ListUnusedAssetsForAssetList
(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter,
	TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetDataToFilter)
	{

		TArray<FString> AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(DataSharedPtr->ObjectPath.ToString());

		if (AssetReferencers.Num() == 0)
		{
			OutUnusedAssetsData.Add(DataSharedPtr);

		}

	}


}

void FXGManageModule::ListSameNameAssetsForAssetList
(const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter,
	TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData)
{
	OutSameNameAssetsData.Empty();

	//Multimap for supporting finding assets with same name

	TMultiMap<FString, TSharedPtr<FAssetData>> AssetsInfoMultiMap;

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetDataToFilter)
	{

		AssetsInfoMultiMap.Emplace(DataSharedPtr->AssetName.ToString(), DataSharedPtr);


	}

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetDataToFilter)
	{
		TArray<TSharedPtr<FAssetData>> OutAssetsData;
		AssetsInfoMultiMap.MultiFind(DataSharedPtr->AssetName.ToString(), OutAssetsData);

		if (OutAssetsData.Num() <= 1)
		{
			continue;
		}
		for (const TSharedPtr<FAssetData>& SameNameData : OutAssetsData)
		{

			if (SameNameData.IsValid())
			{
				OutSameNameAssetsData.AddUnique(SameNameData);
			}

		}
	}



}




void FXGManageModule::SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync)
{
	TArray<FString> AssetsPathToSync;
	AssetsPathToSync.Add(AssetPathToSync);
	UEditorAssetLibrary::SyncBrowserToObjects(AssetsPathToSync);




}

#pragma endregion
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FXGManageModule, XGManage)