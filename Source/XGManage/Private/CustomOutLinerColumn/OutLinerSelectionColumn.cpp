#include "CustomOutLinerColumn/OutLinerSelectionColumn.h"
#include "CustomStyle/XGManageStyle.h"
#include "ActorTreeItem.h"
#include "XGManage.h"

SHeaderRow::FColumn::FArguments FOutLinerSelectionLockColumn::ConstructHeaderRowColumn()
{

	SHeaderRow::FColumn::FArguments ConstructedHeaderRow =

		SHeaderRow::Column(GetColumnID())
		.FixedWidth(24.f)
		.HAlignHeader(HAlign_Center)
		.VAlignHeader(VAlign_Center)
		.HAlignCell(HAlign_Center)
		.VAlignCell(VAlign_Center)
		.DefaultTooltip(FText::FromString(TEXT("Actor Selection Lock - Press icon loack actor Selection")))
		[
			SNew(SImage)
			.ColorAndOpacity(FSlateColor::UseForeground())
		.Image(FXGManagerStyle::GetCreatedSlateStyleSet()->GetBrush(FName("LevelEditor.LockSelection")))
		]

	;



	return ConstructedHeaderRow;
}

const TSharedRef< SWidget > FOutLinerSelectionLockColumn::ConstructRowWidget(
	FSceneOutlinerTreeItemRef TreeItem,
	const STableRow<FSceneOutlinerTreeItemPtr>& Row)
{
	FActorTreeItem* ActorTreeItem = TreeItem->CastTo<FActorTreeItem>();

	if (!ActorTreeItem || !ActorTreeItem->IsValid())
	{
		return SNullWidget::NullWidget;
	}
	FXGManageModule& XGManageModule =
		FModuleManager::LoadModuleChecked<FXGManageModule>(TEXT("XGManage"));

	const bool bIsActorSelectionLocked = XGManageModule.CheckIsActorSelection(ActorTreeItem->Actor.Get());


	const FCheckBoxStyle& ToggleButtonStyle =
	FXGManagerStyle::GetCreatedSlateStyleSet()
	->GetWidgetStyle<FCheckBoxStyle>(FName("SceneOutliner.SelectionLock"));

	TSharedRef<SCheckBox> ConstructedRowWidgetCheckBox =
		SNew(SCheckBox)
		.Visibility(EVisibility::Visible)
		.Type(ESlateCheckBoxType::ToggleButton)
		.Style(&ToggleButtonStyle)
		.HAlign(HAlign_Center)
		.IsChecked(bIsActorSelectionLocked?ECheckBoxState::Checked:ECheckBoxState::Unchecked)
		.OnCheckStateChanged(this,&FOutLinerSelectionLockColumn::OnRowWidgetCheckStateChanged, ActorTreeItem->Actor);


	return ConstructedRowWidgetCheckBox;
}

void FOutLinerSelectionLockColumn::OnRowWidgetCheckStateChanged(ECheckBoxState NewState, TWeakObjectPtr<AActor> CorrespondingActor)
{
	FXGManageModule& XGManageModule =
		FModuleManager::LoadModuleChecked<FXGManageModule>(TEXT("XGManage"));

	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		XGManageModule.ProcessLockingForOutliner(CorrespondingActor.Get(),false);
		break;
	case ECheckBoxState::Checked:
		XGManageModule.ProcessLockingForOutliner(CorrespondingActor.Get(), true);
		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}



}
