#include "ActorActions/QuickActorsWidget.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "DebugHeader.h"
#include "Subsystems/EditorActorSubsystem.h"
void UQuickActorsWidget::SelectAllActorWithSimilarName()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 SelectionCounter = 0;

	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No Actor Selected"));
		return;
	}

	if (SelectedActors.Num() > 1)
	{
		DebugHeader::ShowNotifyInfo(TEXT("You can only Select one  Actor "));
		return;
	}

	FString SelectedActorName = SelectedActors[0]->GetActorLabel();
	const FString NameToSearch = SelectedActorName.LeftChop(4);

	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();
	for (AActor* ActorInLevel : AllLevelActors)
	{
		if (!ActorInLevel)
		{
			continue;
		}

		if (ActorInLevel->GetActorLabel().Contains(NameToSearch, SearchCase))
		{
			EditorActorSubsystem->SetActorSelectionState(ActorInLevel, true);
			SelectionCounter++;

		}

	}
	if (SelectionCounter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully selected") +
			FString::FromInt(SelectionCounter) + TEXT(" actors"));
	}
	else
	{
		DebugHeader::ShowNotifyInfo(TEXT("No Actor with similar name found"));

	}




}
#pragma region ActorBatchDuplication
void UQuickActorsWidget::DupicateActors()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 Counter = 0;

	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No Actor Selected"));
		return;
	}

	if (NumberOfDuplicates <= 0 || OffsetDist == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Did not specify a number duplications or an offset distance "));
		return;
	}
	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor)
		{
			continue;
		}
		for (int32 i = 0; i < NumberOfDuplicates; i++)
		{
			AActor* DuplicatedActor = EditorActorSubsystem->DuplicateActor(SelectedActor, SelectedActor->GetWorld());

			if (!DuplicatedActor)
			{
				continue;
			}

			const float DuplicationOffsetDist = (i + 1) * OffsetDist;


			switch (AxisForDuplication)
			{
			case E_Duplication::EDA_XAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(DuplicationOffsetDist, 0.f, 0.f));
				break;
			case E_Duplication::EDA_YAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(0.f, DuplicationOffsetDist, 0.f));
				break;
			case E_Duplication::EDA_ZAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(0.f, 0.f, DuplicationOffsetDist));
				break;
			case E_Duplication::EDA_MAX:
				break;
			default:
				break;
			}

			EditorActorSubsystem->SetActorSelectionState(DuplicatedActor, true);

			Counter++;
		}




	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully duplicated")
			+ FString::FromInt(Counter) + TEXT(" actors"));


	}



}

#pragma endregion ActorBatchDuplication
#pragma region RandomizeActorTransform
void UQuickActorsWidget::RandomizeActorTransform()
{
	const bool bConditionNotSet = !RandomActorRotation.bRandomizeRotYaw &&
		!RandomActorRotation.bRandomizeRotPitch &&
		!RandomActorRotation.bRandomizeRotRoll&&
		!bRandomizeScale&&
		!bRandomizeOffset;

	if (bConditionNotSet)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No variation condition specified"));
		return;
	}


	if (!GetEditorActorSubsystem())
	{
		return;
	}


	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 Counter = 0;

	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("No Actor Selected"));
		return;
	}
	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor)
		{
			continue;
		}
		if (RandomActorRotation.bRandomizeRotYaw)
		{
			const float RandomRotYawValue = FMath::RandRange(RandomActorRotation.RotYawMin, RandomActorRotation.RotYawMax);

			SelectedActor->AddActorWorldRotation(FRotator(0.f, RandomRotYawValue, 0.f));

		}

		if (RandomActorRotation.bRandomizeRotPitch)
		{
			const float RandomRotPitchValue = FMath::RandRange(RandomActorRotation.RotPitchMin, RandomActorRotation.RotPitchMax);
			SelectedActor->AddActorWorldRotation(FRotator(RandomRotPitchValue, 0.f, 0.f));
		}

		if (RandomActorRotation.bRandomizeRotRoll)
		{
			const float RandomRotRollValue = FMath::RandRange(RandomActorRotation.RotRollMin, RandomActorRotation.RotRollMax);
			SelectedActor->AddActorWorldRotation(FRotator(0.f, 0.f, RandomRotRollValue));
		}

		if (bRandomizeScale)
		{
			SelectedActor->SetActorScale3D(FVector(FMath::FRandRange(ScaleMin,ScaleMax)));

		}
		if (bRandomizeOffset)
		{
			const float RandomOffsetValue = FMath::FRandRange(OffsetMin, OffsetMax);

			SelectedActor->AddActorWorldOffset(FVector(RandomOffsetValue, RandomOffsetValue,0.f));
		}

	
		Counter++;
		

	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully set") +
			FString::FromInt(Counter) + TEXT(" actors"));
	}


}

#pragma endregion RandomizeActorTransform
bool UQuickActorsWidget::GetEditorActorSubsystem()
{
	if (!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}


	return EditorActorSubsystem != nullptr;
}
