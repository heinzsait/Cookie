// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/ParkourInterface.h"
#include "GameplayTagContainer.h"
#include "ParkourMovementComponent.generated.h"

class UCharacterMovementComponent;
class UCapsuleComponent;
class UParkourVariablesDataAsset;

namespace EDrawDebugTrace
{
	enum Type;
}

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARKOURSYSTEM_API UParkourMovementComponent : public UActorComponent, public IParkourInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UParkourMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool SetInitializeReference(ACharacter* Character, USpringArmComponent* CameraBoom, UMotionWarpingComponent* MotionWarping, UCameraComponent* Camera) override;

	UFUNCTION(BlueprintCallable)
	void ParkourAction(bool bAutoClimb);

private:

	void CheckWallShape();

	void ShowHitResults();

	void CheckDistance();

	void ParkourType(const bool bAutoClimb);

	void CheckAndDoHighVault();

	void CheckAndDoLowMantle();

	void CheckAndDoMantle();

	void CheckAndDoVault();

	void CheckAndDoThinVault();

	void SetParkourAction(FGameplayTag NewParkourActionTag);

	void SetParkourState(FGameplayTag NewParkourStateTag);

	bool CheckMantleSurface();

	bool CheckVaultSurface();

	void PlayParkourMontage();

	void ResetParkourResult();

	UFUNCTION()
	void OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);

	FVector FindWarpTargetLocation_1(const float WarpXOffset, const float WarpZOffset);

	FVector FindWarpTargetLocation_2(const float WarpXOffset, const float WarpZOffset);

	FVector FindWarpTargetLocation_3(const float WarpXOffset, const float WarpZOffset);

	void ParkourStateSettings(ECollisionEnabled::Type NewType, EMovementMode NewMovementMode, FRotator RotationRate, bool bDoCollisionTest, bool bStopMovementImmediately);

	ACharacter* PlayerCharacter;

	USkeletalMeshComponent* CharacterMesh;

	UCharacterMovementComponent* CharacterMovement;

	UAnimInstance* CharacterAnimInstance;

	UCapsuleComponent* CharacterCapsule;

	USpringArmComponent* CharacterCameraBoom;
	
	UMotionWarpingComponent* CharacterMotionWarping;
	
	UCameraComponent* CharacterCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> ArrowActorClass;

	AActor* ArrowActor;

	float ArrowLocationX = 0;

	float ArrowLocationZ = 195.0f;

	float CharacterHeightDifference;

	float DefaultCameraBoomTargetArmLength;

	FVector DefaultCameraBoomRelativeLocation;

	FGameplayTag ParkourActionTag;

	FGameplayTag ParkourStateTag;

	FGameplayTag MontageBlendOutState;

	bool bCanAutoClimb;

	bool bCanManualClimb = true;

	EDrawDebugTrace::Type DrawDebugType;

	bool bShowHitResults = true;

	FRotator WallRotation;

	FHitResult WallHitResult;

	FHitResult WallTopResult;

	FHitResult WallDepthResult;

	FHitResult WallVaultResult;

	float WallHeight;

	float WallDepth;

	float VaultHeight;

	bool bInGround = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = DataAssets, meta = (AllowPrivateAccess = "true"))
	UParkourVariablesDataAsset* ParkourVariablesDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAssets, meta = (AllowPrivateAccess = "true"))
	UParkourVariablesDataAsset* VaultDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAssets, meta = (AllowPrivateAccess = "true"))
	UParkourVariablesDataAsset* ThinVaultDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAssets, meta = (AllowPrivateAccess = "true"))
	UParkourVariablesDataAsset* HighVaultDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAssets, meta = (AllowPrivateAccess = "true"))
	UParkourVariablesDataAsset* MantleDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = DataAssets, meta = (AllowPrivateAccess = "true"))
	UParkourVariablesDataAsset* LowMantleDataAsset;
};
