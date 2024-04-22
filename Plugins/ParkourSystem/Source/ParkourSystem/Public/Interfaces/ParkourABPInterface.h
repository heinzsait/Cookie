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

	virtual bool SetParkourState(FGameplayTag NewParkourState) = 0;

	virtual bool SetParkourAction(FGameplayTag NewParkourAction) = 0;

	virtual bool SetClimbStyle(FGameplayTag NewClimbStyle) = 0;

	virtual bool SetClimbMovement(FGameplayTag NewDirection) = 0;

	virtual bool SetLeftHandLedgeLocation(FVector LeftHandLedgeLocation) = 0;

	virtual bool SetRightHandLedgeLocation(FVector RightHandLedgeLocation) = 0;

	virtual bool SetLeftFootLocation(FVector LeftFootLocation) = 0;

	virtual bool SetRightFootLocation(FVector RightFootLocation) = 0;

	virtual bool SetLeftHandLedgeRotation(FRotator LeftHandLedgeRotation) = 0;

	virtual bool SetRightHandLedgeRotation(FRotator RightHandLedgeRotation) = 0;
};
