// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ParkourABPInterface.generated.h"

struct FGameplayTag;

UINTERFACE(BlueprintType)
class UParkourABPInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PARKOURSYSTEM_API IParkourABPInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintImplementableEvent)
	bool SetParkourState(FGameplayTag NewParkourState);

	UFUNCTION(BlueprintImplementableEvent)
	bool SetParkourAction(FGameplayTag NewParkourAction);

	UFUNCTION(BlueprintImplementableEvent)
	bool SetClimbStyle(FGameplayTag NewClimbStyle);

	UFUNCTION(BlueprintImplementableEvent)
	bool SetClimbMovement(FGameplayTag NewDirection);

	UFUNCTION(BlueprintImplementableEvent)
	bool SetLeftHandLedgeLocation(FVector LeftHandLedgeLocation);

	UFUNCTION(BlueprintImplementableEvent)
	bool SetRightHandLedgeLocation(FVector RightHandLedgeLocation);

	UFUNCTION(BlueprintImplementableEvent)
	bool SetLeftFootLocation(FVector LeftFootLocation);

	UFUNCTION(BlueprintImplementableEvent)
	bool SetRightFootLocation(FVector RightFootLocation);

	UFUNCTION(BlueprintImplementableEvent)
	bool SetLeftHandLedgeRotation(FRotator LeftHandLedgeRotation);

	UFUNCTION(BlueprintImplementableEvent)
	bool SetRightHandLedgeRotation(FRotator RightHandLedgeRotation);
};
