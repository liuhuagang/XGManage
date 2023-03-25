#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickActorsWidget.generated.h"

UENUM(BlueprintType)
enum class E_Duplication :uint8
{
	EDA_XAxis  UMETA(DisplayName = "X Axis"),
	EDA_YAxis  UMETA(DisplayName = "Y Axis"),
	EDA_ZAxis  UMETA(DisplayName = "Z Axis"),
	EDA_MAX  UMETA(DisplayName = "Default Max"),
};


USTRUCT(BlueprintType)
struct FRandomActorRotation
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="XGManage")
		bool bRandomizeRotYaw = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotYaw"), Category = "XGManage")
		float  RotYawMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotYaw"), Category = "XGManage")
		float  RotYawMax = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XGManage")
		bool bRandomizeRotPitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotPitch"), Category = "XGManage")
		float  RotPitchMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotPitch"), Category = "XGManage")
		float  RotPitchMax = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "XGManage")
		bool bRandomizeRotRoll = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotRoll"), Category = "XGManage")
		float  RotRollMin = -45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bRandomizeRotRoll"), Category = "XGManage")
		float  RotRollMax = 45.f;




};





UCLASS()
class XGMANAGE_API UQuickActorsWidget :public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
#pragma region ActorBatchSelection
	UFUNCTION(BlueprintCallable, Category = "XGManage")
		void SelectAllActorWithSimilarName();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActorBatchSelection")
		TEnumAsByte<ESearchCase::Type> SearchCase = ESearchCase::IgnoreCase;
#pragma region ActorBatchSelection
#pragma region ActorBatchDuplication

	UFUNCTION(BlueprintCallable, Category = "XGManage")
		void DupicateActors();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
		E_Duplication AxisForDuplication = E_Duplication::EDA_XAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
		int32 NumberOfDuplicates = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActorBatchDuplication")
		float OffsetDist = 300.f;



#pragma endregion ActorBatchDuplication
#pragma region RandomizeActorTransform

	UFUNCTION(BlueprintCallable, Category = "XGManage")
		void RandomizeActorTransform();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform")
		FRandomActorRotation RandomActorRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform")
		bool bRandomizeScale = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeScale"))
		float ScaleMin = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeScale"))
		float ScaleMax = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform")
		bool bRandomizeOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeOffset"))
		float OffsetMin = -50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomizeActorTransform", meta = (EditCondition = "bRandomizeOffset"))
		float OffsetMax = 50.f;





#pragma endregion RandomizeActorTransform
private:
	UPROPERTY()
		class UEditorActorSubsystem* EditorActorSubsystem;

	bool GetEditorActorSubsystem();



};
