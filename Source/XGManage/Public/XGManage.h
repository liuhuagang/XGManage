// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

class FXGManageModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

#pragma region ContentBrowserMenuExtension

	void InitCBMenuExtention();

	TArray<FString> FolderPathsSelected;
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& InSelectedPaths);

	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);
	void OnDeleteUnusedAssetButtonClicked();
	void OnDeleteEmptyFoldersButtonClicked();
	void OnAdvanceDeletionButtonClicked();


	void FixUpRedirectors();

#pragma endregion ContentBrowserMenuExtension


#pragma region LevelEditorMenuExtension

		void InitLevelEditorExtention();

		TSharedRef<FExtender> CustomLevelEditorMenuExtender
		(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectActors);

		void AddLevelEditorMenuEntry(class FMenuBuilder& MenuBuilder);
		void OnLockActorSelectionButtonClicked();
		void OnUnlockActorSelectionButtonClicked();
#pragma endregion LevelEditorMenuExtension

#pragma  region SelectionLock
	
		void InitCutomSelectionEvent();
		void OnActorSelected(UObject* SelectedObject);

		void LockActorSelection(AActor* ActorToProcess);
		void UnlockActorSelection(AActor* ActorToProcess);
		void RefreshSceneOutliner();
public:
		bool CheckIsActorSelection(AActor* ActorToProcess);
		void ProcessLockingForOutliner(AActor* ActorToProcess, bool bShouldLock);
private:


#pragma endregion SelectionLock


#pragma region CustomEditorUICommands

	TSharedPtr<class FUICommandList> CustomUICommands;

	void InitCustomUICommands();

	void OnSelectionLockHotKeyPressed();

	void OnUnselectionLockHotKeyPressed();

#pragma endregion CustomEditorUICommands

#pragma region SceneOutlinerExtension
		void InitSceneOutlinerColumnExtension();

		TSharedRef<class ISceneOutlinerColumn> OncreateSelectionLockColumn(class ISceneOutliner& SceneOutliner);

		void UnRegisterSceneOutlinerColumnExtension();

#pragma endregion SceneOutlinerExtension


		TWeakObjectPtr<class UEditorActorSubsystem> WeakEditorActorSubsystem;
		bool GetEditorActorSubsystem();
	

#pragma  region CustomEditorTab

	void RegisterAdvanceDeletionTab();

	TSharedRef<SDockTab> OnSpawnAdvancedDeletionTab(const FSpawnTabArgs&  InSpawnTabArgs);

	TSharedPtr<SDockTab> ConstructedDockTab;

	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();


	void OnAdvanceDeletionTabClosed(TSharedRef<SDockTab> TabToClose);

#pragma  endregion CustomEditorTab

public:
#pragma  region ProcessDataForAdvanceDeletionTab
	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);
	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete);

	void ListUnusedAssetsForAssetList(
	const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter,
	TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData);

	void ListSameNameAssetsForAssetList(
	const TArray<TSharedPtr<FAssetData>>& AssetDataToFilter,
		TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData);

	void SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync);



#pragma  endregion ProcessDataForAdvanceDeletionTab

};
