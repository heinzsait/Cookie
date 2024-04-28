// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify/ReachLedgeIK.h"
#include "Components/ParkourMovementComponent.h"

void UReachLedgeIK::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{

}

void UReachLedgeIK::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (bSecondIK)
	{
		if (UParkourMovementComponent* ParkourMovementComponent = MeshComp->GetOwner()->GetComponentByClass<UParkourMovementComponent>())
		{
			if (WhichHand == FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.Left")))
			{
				ParkourMovementComponent->LimbsClimbIK(!bSecondIK, true);
			}
			else
			{
				ParkourMovementComponent->LimbsClimbIK(!bSecondIK, false);
			}				
		}
	}
}