// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ParkourVariablesDataAsset.generated.h"

struct FGameplayTag;

UCLASS(Blueprintable)
class PARKOURSYSTEM_API UParkourVariablesDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ParkourMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FGameplayTag ParkourInState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FGameplayTag ParkourOutState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Warp 1 X Offset", meta = (AllowPrivateAccess = "true"))
	float Warp1XOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Warp 1 Z Offset", meta = (AllowPrivateAccess = "true"))
	float Warp1ZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Warp 2 X Offset", meta = (AllowPrivateAccess = "true"))
	float Warp2XOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Warp 2 Z Offset", meta = (AllowPrivateAccess = "true"))
	float Warp2ZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Warp 3 X Offset", meta = (AllowPrivateAccess = "true"))
	float Warp3XOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName = "Warp 3 Z Offset", meta = (AllowPrivateAccess = "true"))
	float Warp3ZOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float MontageStartPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float FallingMontageStartPosition;

};
