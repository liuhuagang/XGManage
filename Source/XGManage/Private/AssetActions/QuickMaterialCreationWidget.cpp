// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickMaterialCreationWidget.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"

#pragma region QuickMaterialCreation
void UQuickMaterialCreationWidget::CreateMaterialFromSelectedTextures()
{
	if (bCustomMaterialName)
	{
		if (MaterialName.IsEmpty() || MaterialName.Equals(TEXT("M_")))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please enter a valid name"));

			return;
		}
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> SelectedTexturesArray;
	FString SelectedTextureFolderPath;
	uint32 PinsConnectedCounter = 0;

	if (!ProcessSelectedData(SelectedAssetsData, SelectedTexturesArray, SelectedTextureFolderPath))
	{
		MaterialName = TEXT("M_");
		return;
	}
	if (CheckIsNameUsed(SelectedTextureFolderPath, MaterialName))
	{
		MaterialName = TEXT("M_");
		return;
	}

	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName, SelectedTextureFolderPath);

	if (!CreatedMaterial)
	{
		DebugHeader::PrintLog(TEXT("Failed to create material"));
		return;
	}
	else
	{
		DebugHeader::PrintLog(TEXT("Succeed to create material"));
	}

	for (UTexture2D* SelectedTexture : SelectedTexturesArray)
	{
		if (!SelectedTexture)
		{
			continue;
		}
		switch (ChannelPackingType)
		{
		case E_ChannelPackingType::ECPT_NoChannelPacking:
			Default_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
			break;
		case E_ChannelPackingType::ECPT_ORM:
			ORM_CreateMaterialNodes(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
			break;
		case E_ChannelPackingType::ECPT_MAX:
			break;
		default:
			break;
		}




	}
	if (PinsConnectedCounter > 0)
	{

		DebugHeader::ShowNotifyInfo(TEXT("Successfully connected ")
			+ FString::FromInt(PinsConnectedCounter)
			+ TEXT(" pins"));

	}



	if (bCreateMaterialInstance)
	{
		CreateMaterialInstanceAsset(CreatedMaterial, MaterialName, SelectedTextureFolderPath);

	}
	MaterialName = TEXT("M_");
}
#pragma endregion

#pragma region QuickMaterialCreationCore

//Process the selected data ,will filter out textures , and return false if non-texture selected
bool UQuickMaterialCreationWidget::ProcessSelectedData
(const TArray<FAssetData>& SelectedDataToProcess,
	TArray<UTexture2D*>& OutSelectedTexturesArray,
	FString& OutSelectedTexturePackagePath)
{
	if (SelectedDataToProcess.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No texture selected"));
		return false;
	}

	bool bMaterialName = false;
	for (const FAssetData& SelectedData : SelectedDataToProcess)
	{
		UObject* SelectedAsset = SelectedData.GetAsset();
		if (!SelectedAsset)
		{
			continue;
		}

		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedAsset);
		if (!SelectedTexture)
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("Please select only textures\n")
				+ SelectedAsset->GetName() + TEXT("is not a texture"));

			return false;

		}

		OutSelectedTexturesArray.Add(SelectedTexture);

		if (OutSelectedTexturePackagePath.IsEmpty())
		{
			OutSelectedTexturePackagePath = SelectedData.PackagePath.ToString();


		}


		if (!bCustomMaterialName && !bMaterialName)
		{
			bMaterialName = true;
			MaterialName = SelectedAsset->GetName();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0, TEXT("M_"));


		}


	}



	return true;

}

//Will return ture if the material name is used by asset under specified folder
bool UQuickMaterialCreationWidget::CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck)
{
	TArray<FString> ExistingAssetsPaths = UEditorAssetLibrary::ListAssets(FolderPathToCheck, false);

	for (const FString& ExistingAssetPath : ExistingAssetsPaths)
	{
		const FString ExistionAssetName = FPaths::GetBaseFilename(ExistingAssetPath);

		if (ExistionAssetName.Equals(MaterialNameToCheck))
		{
			DebugHeader::ShowMsgDialog(EAppMsgType::Ok, MaterialNameToCheck +
				TEXT("is already used by asset"));

			return true;
		}



	}


	return false;
}

UMaterial* UQuickMaterialCreationWidget::CreateMaterialAsset
(const FString& NameOftheMaterial, const FString& PathToPutMaterial)
{

	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();


	UObject* CreatedObject = AssetToolsModule.Get().CreateAsset
	(NameOftheMaterial,
		PathToPutMaterial,
		UMaterial::StaticClass(),
		MaterialFactory);

	return Cast<UMaterial>(CreatedObject);
}

void UQuickMaterialCreationWidget::Default_CreateMaterialNodes(UMaterial* CreatedMaterial, UTexture2D* SelectedTexture, uint32& PinsConnectedCounter)
{
	UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);

	if (!TextureSampleNode)
	{
		return;
	}

	if (!CreatedMaterial->HasBaseColorConnected())
	{
		if (TryConnectBaseColor(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	if (!CreatedMaterial->HasMetallicConnected())
	{
		if (TryConnectMetalic(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!CreatedMaterial->HasRoughnessConnected())
	{
		if (TryConnectRoughness(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!CreatedMaterial->HasNormalConnected())
	{
		if (TryConnectNormal(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!CreatedMaterial->HasAmbientOcclusionConnected())
	{
		if (TryConnectAO(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}

	DebugHeader::PrintLog(TEXT("Failed to connect the texture:" + SelectedTexture->GetName()));
}

void UQuickMaterialCreationWidget::ORM_CreateMaterialNodes(
	UMaterial* CreatedMaterial,
	UTexture2D* SelectedTexture,
	uint32& PinsConnectedCounter)
{

	UMaterialExpressionTextureSample* TextureSampleNode
		= NewObject<UMaterialExpressionTextureSample>(CreatedMaterial);

	if (!TextureSampleNode)
	{
		return;
	}
	if (!CreatedMaterial->HasBaseColorConnected())
	{
		if (TryConnectBaseColor(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}


	if (!CreatedMaterial->HasNormalConnected())
	{
		if (TryConnectNormal(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter++;
			return;
		}
	}


	if (!CreatedMaterial->HasRoughnessConnected())
	{
		if (TryConnectORM(TextureSampleNode, SelectedTexture, CreatedMaterial))
		{
			PinsConnectedCounter += 3;
			return;
		}

	}





}

#pragma endregion


#pragma region CreateMaterialNodesConnectPins
bool UQuickMaterialCreationWidget::TryConnectBaseColor(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& BaseColorName : BaseColorArray)
	{
		if (SelectedTexture->GetName().Contains(BaseColorName))
		{
			TextureSampleNode->Texture = SelectedTexture;
			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			UMaterialEditorOnlyData* MaterialEditorOnly = const_cast<UMaterialEditorOnlyData*>(CreatedMaterial->GetEditorOnlyData());
			MaterialEditorOnly->BaseColor.Expression = TextureSampleNode;
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600.f;


			return true;
		}
	}

	return false;
}





bool UQuickMaterialCreationWidget::TryConnectMetalic(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& MetalicName : MetallicArray)
	{
		if (SelectedTexture->GetName().Contains(MetalicName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			UMaterialEditorOnlyData* MaterialEditorOnly = const_cast<UMaterialEditorOnlyData*>(CreatedMaterial->GetEditorOnlyData());
			MaterialEditorOnly->Metallic.Expression = TextureSampleNode;
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600.f;
			TextureSampleNode->MaterialExpressionEditorY += 240.f;
			return true;
		}
	}

	return false;
}

bool UQuickMaterialCreationWidget::TryConnectRoughness(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& RoughnessName : RoughnessArray)
	{
		if (SelectedTexture->GetName().Contains(RoughnessName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			UMaterialEditorOnlyData* MaterialEditorOnly = const_cast<UMaterialEditorOnlyData*>(CreatedMaterial->GetEditorOnlyData());
			MaterialEditorOnly->Roughness.Expression = TextureSampleNode;
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600.f;
			TextureSampleNode->MaterialExpressionEditorY += 480.f;

			return true;
		}


	}

	return false;
}

bool UQuickMaterialCreationWidget::TryConnectNormal(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& NormalName : NormalArray)
	{
		if (SelectedTexture->GetName().Contains(NormalName))
		{

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Normal;

			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			UMaterialEditorOnlyData* MaterialEditorOnly = const_cast<UMaterialEditorOnlyData*>(CreatedMaterial->GetEditorOnlyData());
			MaterialEditorOnly->Normal.Expression = TextureSampleNode;
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600.f;
			TextureSampleNode->MaterialExpressionEditorY += 720.f;

			return true;
		}


	}

	return false;
}

bool UQuickMaterialCreationWidget::TryConnectAO(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& AOName : AmbientOcclusionArray)
	{
		if (SelectedTexture->GetName().Contains(AOName))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;


			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);
			UMaterialEditorOnlyData* MaterialEditorOnly = const_cast<UMaterialEditorOnlyData*>(CreatedMaterial->GetEditorOnlyData());
			MaterialEditorOnly->AmbientOcclusion.Expression = TextureSampleNode;
			CreatedMaterial->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600.f;
			TextureSampleNode->MaterialExpressionEditorY += 960.f;
			return true;
		}
	}

	return false;
}

bool UQuickMaterialCreationWidget::TryConnectORM(UMaterialExpressionTextureSample* TextureSampleNode, UTexture2D* SelectedTexture, UMaterial* CreatedMaterial)
{
	for (const FString& ORM_Name : ORMArray)
	{
		if (SelectedTexture->GetName().Contains(ORM_Name))
		{
			SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Masks;
			SelectedTexture->SRGB = false;
			SelectedTexture->PostEditChange();

			TextureSampleNode->Texture = SelectedTexture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Masks;


			CreatedMaterial->GetExpressionCollection().AddExpression(TextureSampleNode);

			UMaterialEditorOnlyData* MaterialEditorOnly = const_cast<UMaterialEditorOnlyData*>(CreatedMaterial->GetEditorOnlyData());
			MaterialEditorOnly->AmbientOcclusion.Connect(1, TextureSampleNode);
			MaterialEditorOnly->Roughness.Connect(2, TextureSampleNode);
			MaterialEditorOnly->Metallic.Connect(3, TextureSampleNode);
			CreatedMaterial->PostEditChange();


			TextureSampleNode->MaterialExpressionEditorX -= 600.f;
			TextureSampleNode->MaterialExpressionEditorY += 960.f;

			return true;

		}

	}




	return false;
}



#pragma endregion CreateMaterialNodesConnectPins


class UMaterialInstanceConstant* UQuickMaterialCreationWidget::CreateMaterialInstanceAsset(UMaterial* CreatedMaterial, FString NameOfMaterialInstance, const FString& PathToPutMI)
{
	NameOfMaterialInstance.RemoveFromStart(TEXT("M_"));
	NameOfMaterialInstance.InsertAt(0, TEXT("MI_"));

	UMaterialInstanceConstantFactoryNew* MIFactoryNeW = NewObject<UMaterialInstanceConstantFactoryNew>();


	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UObject* CreatedObject
		= AssetToolsModule.Get().CreateAsset(NameOfMaterialInstance, PathToPutMI, UMaterialInstanceConstant::StaticClass(), MIFactoryNeW);

	if (UMaterialInstanceConstant* CreatedMI= Cast<UMaterialInstanceConstant>(CreatedObject))
	{
		CreatedMI->SetParentEditorOnly(CreatedMaterial);
		CreatedMI->PostEditChange();
		CreatedMaterial->PostEditChange();

		return CreatedMI;
	}

	return nullptr;
}
