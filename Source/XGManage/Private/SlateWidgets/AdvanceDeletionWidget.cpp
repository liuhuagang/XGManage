// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "Widgets/Layout/SScrollBox.h"
#include "DebugHeader.h"
#include "Widgets/Input/SButton.h"
#include "XGManage.h"

#define LISTALL TEXT("List All Available Assets")
#define LISTUNUSED TEXT("List Unused Assets")
#define LISTSAMENAME TEXT("List Assets With the Same Name")

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	StoredAssetsData = InArgs._AssetsDataToStore;
	DisplayedAssetsData = StoredAssetsData;

	CheckBoxsArray.Empty();
	AssetsDataToDeleteArray.Empty();
	ComboxBoxSourceItems.Empty();

	ComboxBoxSourceItems.Add(MakeShared<FString>(LISTALL));
	ComboxBoxSourceItems.Add(MakeShared<FString>(LISTUNUSED));
	ComboxBoxSourceItems.Add(MakeShared<FString>(LISTSAMENAME));



	FSlateFontInfo TitleTextFont = GetEmboseddTextFont();
	TitleTextFont.Size = 30;



	ChildSlot
		[
			// Main Vertical box
			SNew(SVerticalBox)

			//First vertical slot for title text
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Advance Deletion")))
				.Font(TitleTextFont)
				.Justification(ETextJustify::Center)
				.ColorAndOpacity(FColor::White)
			]

			//SecondSlot for drop down to specify the listing condition and text
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				//Combox Box Slot 
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					ConstructComboBox()
				]
				//Help text for combo box slot
				+ SHorizontalBox::Slot()
				.FillWidth(0.6f)
				[
					ConsturctComboHelpTexts(
					TEXT("Specify the listing condition in the drop down. Left mouse click to go to where asset is located")
					,ETextJustify::Center)
				]

				//Help text for folder path
				+ SHorizontalBox::Slot()
					.FillWidth(0.1f)
				[
					ConsturctComboHelpTexts(
						TEXT("Current Folder\n")+InArgs._CurrentSelectedFolder
						, ETextJustify::Right)
				]


			]
			//Third slot for the asset list
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					ConstructAssetListView()
				]

			]
			//Fourth slot for 3 buttons 
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				//Button1 slot
				+SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructDeleteAllButton()
				]


				//Button2 slot
				+SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructSelectAllButton()
				]


				//Button3 slot
				+SHorizontalBox::Slot()
				.FillWidth(10.f)
				.Padding(5.f)
				[
					ConstructDeSelectAllButton()
				]





			]

		];

}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView=
		SNew(SListView<TSharedPtr<FAssetData>>)
		.ItemHeight(24.f)
		.ListItemsSource(&DisplayedAssetsData)
		.OnGenerateRow(this, &SAdvanceDeletionTab::OnGenerateRowForList)
		.OnMouseButtonClick(this, &SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked);


	return	ConstructedAssetListView.ToSharedRef();

}

void SAdvanceDeletionTab::RefreshAssetListView()
{
	AssetsDataToDeleteArray.Empty();
	CheckBoxsArray.Empty();

	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

#pragma region ComboBoxForListingCondition
TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvanceDeletionTab::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox
		= SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&ComboxBoxSourceItems)
		.OnGenerateWidget(this, &SAdvanceDeletionTab::OnGenerateComboContent)
		.OnSelectionChanged(this, &SAdvanceDeletionTab::OnComboSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock,STextBlock)
			.Text(FText::FromString(TEXT("List Assets Option")))

		];



	return ConstructedComboBox;
}


TSharedRef<SWidget> SAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> ConstructedComboText =
		SNew(STextBlock).Text(FText::FromString(*SourceItem.Get()));
	return ConstructedComboText;

}


void SAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	DebugHeader::PrintLog(*SelectedOption.Get());

	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));

	FXGManageModule& XGManageModule 
	= FModuleManager::LoadModuleChecked<FXGManageModule>(TEXT("XGManage"));

	//Pass data for our module to filter based on the selected option
	if (*SelectedOption.Get() == LISTALL)
	{
		//List all stored asset data

		DisplayedAssetsData = StoredAssetsData;
		RefreshAssetListView();

	}
	else if (*SelectedOption.Get() == LISTUNUSED)
	{
		//List all unused assets
		XGManageModule.ListUnusedAssetsForAssetList(StoredAssetsData,DisplayedAssetsData);
		RefreshAssetListView();

	}
	else if (*SelectedOption.Get() == LISTSAMENAME)
	{
		//List out all assets with same name

		XGManageModule.ListSameNameAssetsForAssetList(StoredAssetsData, DisplayedAssetsData);
		RefreshAssetListView();

	}

}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConsturctComboHelpTexts
(const FString& TextContent, ETextJustify::Type TextJustify)
{
	TSharedRef<STextBlock> ConstructHelpText =
	SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Justification(TextJustify)
	.AutoWrapText(true);

	return ConstructHelpText;
	
}



#pragma endregion



#pragma  region RowWidgetForAssetListView





TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(
	TSharedPtr<FAssetData> AssetDataToDisplay,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid())
	{
		return  SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);
	}


	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClassPath.GetAssetName().ToString();

	FSlateFontInfo AssetClassNameFont = GetEmboseddTextFont();
	AssetClassNameFont.Size = 10;
	FSlateFontInfo AssetNameFont = GetEmboseddTextFont();
	AssetNameFont.Size =15;


	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget
		= SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable).Padding(FMargin(5.f))
		[
			SNew(SHorizontalBox)
			//First slot for check box
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillWidth(.05f)
			[
				ConstructCheckBox(AssetDataToDisplay)
			]

			//Second slot for displaying asset class name
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			.FillWidth(.5f)
			[
				ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
			]


			//Third slot for displaying asset name
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Fill)
			[
				ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
			]


			//Fourth slot for a button
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
			[
				ConstructButtonForRowWidget(AssetDataToDisplay)
			]
		
		];


	return ListViewRowWidget;
}

void SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData)
{
	FXGManageModule& XGManageModule = FModuleManager::LoadModuleChecked<FXGManageModule>(TEXT("XGManage"));
	XGManageModule.SyncCBToClickedAssetForAssetList(ClickedData->ObjectPath.ToString());

}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox =
	SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this, &SAdvanceDeletionTab::OnCheckBoxStateChanged,AssetDataToDisplay)
	.Visibility(EVisibility::Visible);


	CheckBoxsArray.Add(ConstructedCheckBox);
	return ConstructedCheckBox;


}

void SAdvanceDeletionTab::OnCheckBoxStateChanged
(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:

		if (AssetsDataToDeleteArray.Contains(AssetData))
		{
			AssetsDataToDeleteArray.Remove(AssetData);
		}
		
		
		break;

	case ECheckBoxState::Checked:
		
		AssetsDataToDeleteArray.AddUnique(AssetData);
		
		
		break;


	case ECheckBoxState::Undetermined:
		DebugHeader::PrintLog(AssetData->AssetName.ToString() + TEXT("is undetermined"));
		break;


	default:
		break;
	}



}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{

	TSharedRef<STextBlock> ConstructedTextBlock =  
	SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(FontToUse)
	.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton=
	SNew(SButton)
	.Text(FText::FromString(TEXT("Delete")))
	.OnClicked(this, &SAdvanceDeletionTab::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData>  ClickedAssetData)
{
	DebugHeader::PrintLog(ClickedAssetData->AssetName.ToString() + TEXT("is clicked "));
	FXGManageModule& XGManageModule =	FModuleManager::LoadModuleChecked<FXGManageModule>(TEXT("XGManage"));

	const bool bAssetDelete=  XGManageModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());
	
	if (bAssetDelete)
	{
		//Updating the list source items
		if (StoredAssetsData.Contains(ClickedAssetData))
		{
			StoredAssetsData.Remove(ClickedAssetData);

		}
		if (DisplayedAssetsData.Contains(ClickedAssetData))
		{
			DisplayedAssetsData.Remove(ClickedAssetData);

		}


		//Refresh the list
		RefreshAssetListView();

	}

	
	return FReply::Handled();
}
#pragma  endregion
#pragma region TabButtons
TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this,&SAdvanceDeletionTab::OnDeleteAllButtonClicked);

	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete All")));

	return DeleteAllButton;
}


TSharedRef<SButton> SAdvanceDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnSelectAllButtonClicked);

	SelectButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));

	return SelectButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeSelectAllButton()
{
	TSharedRef<SButton> DeSelectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeSelectAllButtonClicked);

	DeSelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("DeSelect All")));

	return DeSelectAllButton;
}

FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	if (AssetsDataToDeleteArray.Num()==0)
	{
		DebugHeader::ShowMsgDialog(
		EAppMsgType::Ok,TEXT("No asset current selected"));

		return FReply::Handled();

	}

	TArray<FAssetData> AssetDataToDelete;
	for (const TSharedPtr<FAssetData>& Data:AssetsDataToDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());

	}
	FXGManageModule& XGManageModule = FModuleManager::LoadModuleChecked<FXGManageModule>(TEXT("XGManage"));

	//Pass data to our module for deletion

	const bool bAssetDeleted=  XGManageModule.DeleteMultipleAssetsForAssetList(AssetDataToDelete);
	
	if (bAssetDeleted)
	{
		for (const TSharedPtr<FAssetData>& DeletedData:AssetsDataToDeleteArray)
		{
			//Updating the stored assets date
			if (StoredAssetsData.Contains(DeletedData))
			{
				StoredAssetsData.Remove(DeletedData);
			}


			if (DisplayedAssetsData.Contains(DeletedData))
			{
				DisplayedAssetsData.Remove(DeletedData);
				

			}

		}

		RefreshAssetListView();
	}


	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
{
	if (CheckBoxsArray.Num()==0)
	{

		return FReply::Handled();
	}

	for (const TSharedRef<SCheckBox> CheckBox: CheckBoxsArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}




	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeSelectAllButtonClicked()
{
	if (CheckBoxsArray.Num() == 0)
	{

		return FReply::Handled();
	}

	for (const TSharedRef<SCheckBox> CheckBox : CheckBoxsArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}


	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmboseddTextFont();
	ButtonTextFont.Size = 15;

	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(ButtonTextFont)
	.Justification(ETextJustify::Center);
	return ConstructedTextBlock;
}
#pragma  endregion
