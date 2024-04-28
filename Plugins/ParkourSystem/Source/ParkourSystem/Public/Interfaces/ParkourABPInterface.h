// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "ParkourABPInterface.generated.h"

UINTERFACE(Blueprintable)
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

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetParkourState(FGameplayTag NewParkourState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetParkourAction(FGameplayTag NewParkourAction);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetClimbStyle(FGameplayTag NewClimbStyle);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetClimbMovement(FGameplayTag NewDirection);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetLeftHandLedgeLocation(FVector LeftHandLedgeLocation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetRightHandLedgeLocation(FVector RightHandLedgeLocation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetLeftFootLocation(FVector LeftFootLocation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetRightFootLocation(FVector RightFootLocation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetLeftHandLedgeRotation(FRotator LeftHandLedgeRotation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool SetRightHandLedgeRotation(FRotator RightHandLedgeRotation);
};
