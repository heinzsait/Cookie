// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ParkourFunctionLibrary.generated.h"

struct FGameplayTag;

UCLASS()
class PARKOURSYSTEM_API UParkourFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	static FRotator NormalReverseRotationZ(const FVector NormalVector);
	
	static FRotator ReverseRotation(const FRotator Rotation);

	static FVector GetForwardVector(FRotator Rotation);

	static FVector GetRightVector(FRotator Rotation);

	static float SelectClimbStyleFloat(const float Braced, const float FreeHang, const FGameplayTag ClimbStyle);

	static float SelectParkourDirectionFloat(const float Forward, const float Backward, const float Left, const float Right, const float ForwardLeft, const float ForwardRight, const float BackwardLeft, const float BackwardRight, const FGameplayTag Direction);

	static float SelectParkoutStateFloat(const float NotBusy, const float Vault, const float Mantle, const float Climb, const FGameplayTag StateTag);
	
	static FGameplayTag SelectParkourDirectionHopAction(const FGameplayTag Forward, const FGameplayTag Backward, const FGameplayTag Left, const FGameplayTag Right, const FGameplayTag ForwardLeft, const FGameplayTag ForwardRight, const FGameplayTag BackwardLeft, const FGameplayTag BackwardRight, const FGameplayTag Direction);
};
