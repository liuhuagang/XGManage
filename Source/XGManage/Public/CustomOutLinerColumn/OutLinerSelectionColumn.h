#pragma once

#include "ISceneOutlinerColumn.h"


class FOutLinerSelectionLockColumn : public ISceneOutlinerColumn
{
public:
	
	FOutLinerSelectionLockColumn(ISceneOutliner& SceneOutliner) {}


	virtual FName GetColumnID() override{return FName("SelectionLock"); }

	static FName GetID(){ return FName("SelectionLock"); };

	virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;

	virtual const TSharedRef< SWidget > ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) ;

private:

	void OnRowWidgetCheckStateChanged(ECheckBoxState NewState,TWeakObjectPtr<AActor> CorrespondingActor);






};