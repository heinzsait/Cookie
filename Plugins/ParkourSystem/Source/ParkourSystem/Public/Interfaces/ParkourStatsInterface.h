// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ParkourStatsInterface.generated.h"

class UCameraComponent;
class UMotionWarpingComponent;
class USpringArmComponent;

UINTERFACE(BlueprintType)
class UParkourStatsInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PARKOURSYSTEM_API IParkourStatsInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual bool SetParkourState(FText ParkourState) = 0;

	virtual bool SetParkourAction(FText ParkourAction) = 0;

	virtual bool SetClimbStyle(FText ParkourStyle) = 0;
};
