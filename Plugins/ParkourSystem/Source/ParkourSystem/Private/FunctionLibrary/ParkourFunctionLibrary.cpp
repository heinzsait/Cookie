// Fill out your copyright notice in the Description page of Project Settings.


#include "FunctionLibrary/ParkourFunctionLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameplayTagContainer.h"

FRotator UParkourFunctionLibrary::NormalReverseRotationZ(const FVector NormalVector)
{
	return FRotator(0, (UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::MakeRotFromX(NormalVector), FRotator(0, 180, 0))).Yaw, 0);
}

FRotator UParkourFunctionLibrary::ReverseRotation(const FRotator Rotation)
{
	return UKismetMathLibrary::NormalizedDeltaRotator(Rotation, FRotator(0, 180, 0));
}

float UParkourFunctionLibrary::SelectClimbStyleFloat(const float Braced, const float FreeHang, const FGameplayTag ClimbStyle)
{
	FGameplayTag BracedTag = FGameplayTag::RequestGameplayTag(FName("Parkour.ClimbStyle.Braced"));
	FGameplayTag FreeHangTag = FGameplayTag::RequestGameplayTag(FName("Parkour.ClimbStyle.FreeHang"));

	if (ClimbStyle == BracedTag)
	{
		return Braced;
	}
	else if (ClimbStyle == FreeHangTag)
	{
		return FreeHang;
	}
	else
	{
		return -1;
	}
}

float UParkourFunctionLibrary::SelectParkourDirectionFloat(const float Forward, const float Backward, const float Left, const float Right, const float ForwardLeft, const float ForwardRight, const float BackwardLeft, const float BackwardRight, const FGameplayTag Direction)
{
	FGameplayTag ForwardTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.Forward"));
	FGameplayTag BackwardTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.Backward"));
	FGameplayTag LeftTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.Left"));
	FGameplayTag RightTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.Right"));
	FGameplayTag ForwardLeftTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.ForwardLeft"));
	FGameplayTag ForwardRightTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.ForwardRight"));
	FGameplayTag BackwardLeftTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.BackwardLeft"));
	FGameplayTag BackwardRightTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.BackwardRight"));

	if (Direction == ForwardTag)
	{
		return Forward;
	}
	else if (Direction == BackwardTag)
	{
		return Backward;
	}
	else if (Direction == LeftTag)
	{
		return Left;
	}
	else if (Direction == RightTag)
	{
		return Right;
	}
	else if (Direction == ForwardLeftTag)
	{
		return ForwardLeft;
	}
	else if (Direction == ForwardRightTag)
	{
		return ForwardRight;
	}
	else if (Direction == BackwardLeftTag)
	{
		return BackwardLeft;
	}
	else if (Direction == BackwardRightTag)
	{
		return BackwardRight;
	}
	else
	{
		return -1;
	}
}

float UParkourFunctionLibrary::SelectParkoutStateFloat(const float NotBusy, const float Vault, const float Mantle, const float Climb, const FGameplayTag StateTag)
{
	FGameplayTag NotBusyTag = FGameplayTag::RequestGameplayTag(FName("Parkour.State.NotBusy"));
	FGameplayTag VaultTag = FGameplayTag::RequestGameplayTag(FName("Parkour.State.Vault"));
	FGameplayTag MantleTag = FGameplayTag::RequestGameplayTag(FName("Parkour.State.Mantle"));
	FGameplayTag ClimbTag = FGameplayTag::RequestGameplayTag(FName("Parkour.State.Climb"));

	if (StateTag == NotBusyTag)
	{
		return NotBusy;
	}
	else if (StateTag == VaultTag)
	{
		return Vault;
	}
	else if (StateTag == MantleTag)
	{
		return Mantle;
	}
	else if (StateTag == ClimbTag)
	{
		return Climb;
	}
	else
	{
		return -1;
	}
}

FGameplayTag UParkourFunctionLibrary::SelectParkourDirectionHopAction(const FGameplayTag Forward, const FGameplayTag Backward, const FGameplayTag Left, const FGameplayTag Right, const FGameplayTag ForwardLeft, const FGameplayTag ForwardRight, const FGameplayTag BackwardLeft, const FGameplayTag BackwardRight, const FGameplayTag Direction)
{
	FGameplayTag ForwardTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.Forward"));
	FGameplayTag BackwardTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.Backward"));
	FGameplayTag LeftTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.Left"));
	FGameplayTag RightTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.Right"));
	FGameplayTag ForwardLeftTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.ForwardLeft"));
	FGameplayTag ForwardRightTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.ForwardRight"));
	FGameplayTag BackwardLeftTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.BackwardLeft"));
	FGameplayTag BackwardRightTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Direction.BackwardRight"));

	if (Direction == ForwardTag)
	{
		return Forward;
	}
	else if (Direction == BackwardTag)
	{
		return Backward;
	}
	else if (Direction == LeftTag)
	{
		return Left;
	}
	else if (Direction == RightTag)
	{
		return Right;
	}
	else if (Direction == ForwardLeftTag)
	{
		return ForwardLeft;
	}
	else if (Direction == ForwardRightTag)
	{
		return ForwardRight;
	}
	else if (Direction == BackwardLeftTag)
	{
		return BackwardLeft;
	}
	else if (Direction == BackwardRightTag)
	{
		return BackwardRight;
	}
	else
	{
		return FGameplayTag();
	}
}