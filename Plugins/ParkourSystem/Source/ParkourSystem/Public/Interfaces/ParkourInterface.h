// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ParkourInterface.generated.h"

class UCameraComponent;
class UMotionWarpingComponent;
class USpringArmComponent;

UINTERFACE(BlueprintType)
class UParkourInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PARKOURSYSTEM_API IParkourInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual bool SetInitializeReference(ACharacter* Character, USpringArmComponent* CameraBoom, UMotionWarpingComponent* MotionWarping, UCameraComponent* Camera) = 0;
};
