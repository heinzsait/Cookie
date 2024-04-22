// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CookieCharacterMovementComponent.generated.h"

class ACookieCharacter;

UCLASS()
class COOKIE_API UCookieCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:

	UCookieCharacterMovementComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float MaxRunSpeed = 500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float MaxSprintSpeed = 750;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	int MaxJumpCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float DashSpeed = 3500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float DashTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float WallJumpSpeed = 800;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float WallJumpAirControl = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float WallRunSpeed = 800;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float MinimumWallRunSpeedThreshold = 400;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float WallRunZVelocity = -100;

	UPROPERTY(BlueprintReadOnly)
	bool bIsWallSliding;

	UPROPERTY(BlueprintReadOnly)
	bool bIsWallRunRightSide;

	UPROPERTY(BlueprintReadOnly)
	bool bIsWallRunLeftSide;

	UPROPERTY(BlueprintReadOnly)
	bool bWallRunOnRightSide;

	UPROPERTY(BlueprintReadOnly)
	bool bIsDashing;

	UPROPERTY(BlueprintReadOnly)
	bool bIsAirDashing;

	UPROPERTY(BlueprintReadOnly)
	bool bIsSprinting;	

	UPROPERTY(BlueprintReadOnly)
	bool bIsGrabbingLedge;

	bool CanMove();

	void EnableSprint();

	void DisableSprint();

	void ProcessJump();

	void Jump();

	void OnLanded();

	void Dash();

	void WallSlide(const float DeltaTime);

	void WallRun(const float DeltaTime);

	void WallJump();

	void WallRunJump();

	UFUNCTION(BlueprintCallable)
	bool IsWallRunning();

	bool CanPerformWallRun();

	float GetForwardTraceAngle();

	void LedgeTrace(const float DeltaSeconds);

	void LedgeHang(const FHitResult& ForwardTraceHit, const FHitResult& HeightTraceHit);

	void LedgeHangDrop();

	void LedgeClimbToPlatform();

	UFUNCTION()
	void LedgeClimbUp_AnimNotify();

	void CorrectLedgePositionOffset(const float DeltaSeconds);
	
	void LedgeSideTrace();

	bool CanLedgeInMove();

	void MoveInLedge(const float DeltaSeconds);

private:

	UAnimInstance* AnimInstance;

	ACookieCharacter* CookieCharacter;

	float DefaultGravity;

	float DefaultAirControl;

	bool bCanDash = true;

	bool bIsWallJumping;

	int JumpCount = 0;

	float JumpResetTime = 0.2f;

	bool bCanPerformWallRun = false;

	bool bIsJumpingOfWall;

	bool bCanLedgeMoveRight;

	bool bCanLedgeMoveLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* LedgeClimbToPlatformMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DoubleJumpMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DashMontage;

	FVector LedgePlatformTargetLocation;

	float CapsuleHalfHeight;

	bool bCorrectLedgeClimbOffset;

	FVector CorrectedAnimOffsetLocation;
};
