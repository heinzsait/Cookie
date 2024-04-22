// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CookieCharacterMovementComponent.h"
#include "Cookie/CookieCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "KismetTraceUtils.h"


UCookieCharacterMovementComponent::UCookieCharacterMovementComponent()
{
	MaxWalkSpeed = MaxRunSpeed;
}

void UCookieCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	if (CharacterOwner)
	{
		CookieCharacter = Cast<ACookieCharacter>(CharacterOwner);
		AnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

		CapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		
		DefaultGravity = GravityScale;
		DefaultAirControl = AirControl;
	}
}

void UCookieCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);	

	WallSlide(DeltaTime);

	if (bIsWallSliding == false)
	{
		WallRun(DeltaTime);
	}

	LedgeTrace(DeltaTime);

	MoveInLedge(DeltaTime);
}

bool UCookieCharacterMovementComponent::CanMove()
{
	return (bIsWallSliding == false && bIsWallJumping == false && bIsGrabbingLedge == false);
}

void UCookieCharacterMovementComponent::EnableSprint()
{
	bIsSprinting = true;
	MaxWalkSpeed = MaxSprintSpeed;
}

void UCookieCharacterMovementComponent::DisableSprint()
{
	bIsSprinting = false;
	MaxWalkSpeed = MaxRunSpeed;
}

void UCookieCharacterMovementComponent::ProcessJump()
{
	if (bIsWallSliding)
	{
		WallJump();
	}
	else if (IsWallRunning())
	{
		WallRunJump();
	}
	else if (bIsGrabbingLedge)
	{
		LedgeClimbToPlatform();
	}
	else
	{
		Jump();
	}
}

void UCookieCharacterMovementComponent::Jump()
{
	JumpCount++;
	if (JumpCount <= MaxJumpCount)
	{
		if (JumpCount == 1)
		{
			if (bIsSprinting)
			{
				MaxWalkSpeed = MaxRunSpeed;
			}
			else
			{
				MaxWalkSpeed = MaxRunSpeed * 0.75f;
			}
			FVector JumpLaunchVelocity(0, 0, JumpZVelocity);
			CharacterOwner->LaunchCharacter(JumpLaunchVelocity, false, true);
		}
		else
		{
			MaxWalkSpeed = MaxRunSpeed * 0.75f;
			FVector JumpLaunchVelocity(0, 0, JumpZVelocity);
			CharacterOwner->LaunchCharacter(JumpLaunchVelocity, false, true);

			if (AnimInstance && DoubleJumpMontage)
			{
				AnimInstance->Montage_Play(DoubleJumpMontage);
			}
		}
	}
}

void UCookieCharacterMovementComponent::OnLanded()
{
	JumpCount = 0;

	bCanPerformWallRun = false;
	bIsWallRunRightSide = false;
	bIsWallRunLeftSide = false;
	bIsJumpingOfWall = false;
	bIsWallSliding = false;
	bCanDash = true;

	AirControl = DefaultAirControl;
	GravityScale = DefaultGravity;

	if (bIsSprinting)
	{
		MaxWalkSpeed = MaxSprintSpeed;
	}
	else
	{
		MaxWalkSpeed = MaxRunSpeed;
	}
}

void UCookieCharacterMovementComponent::Dash()
{
	if (bCanDash == false || IsWallRunning() || bIsWallSliding || bIsGrabbingLedge)
	{
		if (bIsGrabbingLedge)
		{
			LedgeHangDrop();
		}
		return;
	}

	if (IsFalling() == false)
	{
		if (bIsDashing == false)
		{
			bIsDashing = true;
			Velocity = CharacterOwner->GetActorForwardVector() * DashSpeed;

			FTimerDelegate TimerDelegate;
			TimerDelegate.BindLambda([&]
			{
				bIsDashing = false;
				Velocity = CharacterOwner->GetActorForwardVector() * 100;
			});

			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, DashTime, false);

			if (AnimInstance && DashMontage)
			{
				AnimInstance->Montage_Play(DashMontage);
			}
		}		
	}
	else
	{
		if (bIsAirDashing == false)
		{
			bIsAirDashing = true;
			bIsDashing = true;
			GravityScale = 0.1f;
			Velocity = CharacterOwner->GetActorForwardVector() * DashSpeed;
			bCanDash = false;

			FTimerDelegate TimerDelegate;
			TimerDelegate.BindLambda([&]
			{
				bIsAirDashing = false;
				bIsDashing = false;
				GravityScale = DefaultGravity;
				Velocity = CharacterOwner->GetActorForwardVector() * 100;
			});

			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, DashTime, false);
			
			if (AnimInstance && DashMontage)
			{
				AnimInstance->Montage_Play(DashMontage);
			}
		}
	}
}

void UCookieCharacterMovementComponent::WallSlide(const float DeltaTime)
{
	if (IsFalling() && bIsGrabbingLedge == false)
		{
			if (UWorld* World = GetWorld())
			{
				FHitResult Out;
				FVector TraceStart = CharacterOwner->GetActorLocation() + (CapsuleHalfHeight * FVector::UpVector);
				bool bGotHit = World->LineTraceSingleByChannel
				(
					Out,
					TraceStart,
					(CharacterOwner->GetActorForwardVector() * 50.0f) + TraceStart,
					ECollisionChannel::ECC_Visibility
				);

				//DrawDebugLine(GetWorld(), TraceStart, (GetActorForwardVector() * 50.0f) + TraceStart, FColor::Red);

				float Angle = GetForwardTraceAngle();

				if (bGotHit && (Angle >= 0 && Angle < 20))
				{
					bIsWallSliding = true;
					FRotator TargetRotation(0, UKismetMathLibrary::MakeRotFromX(Out.ImpactNormal).Yaw + 180, 0);
					CharacterOwner->SetActorRotation(TargetRotation);
					return;
				}
			}
		}
	

	bIsWallSliding = false;
}

void UCookieCharacterMovementComponent::WallRun(const float DeltaTime)
{
	if (CanPerformWallRun())
		{
			if (UWorld* World = GetWorld())
			{							
				FVector TraceStart = CharacterOwner->GetActorLocation() - (CapsuleHalfHeight * FVector::UpVector);
				float TraceLength = 45;
				if (bIsWallRunLeftSide == false)
				{
					FHitResult OutRight;
					bool bGotHitRight = World->LineTraceSingleByChannel
					(
						OutRight,
						TraceStart,
						(CharacterOwner->GetActorRightVector() * TraceLength) + TraceStart,
						ECollisionChannel::ECC_Visibility
					);

					//DrawDebugLine(GetWorld(), TraceStart, (GetActorRightVector() * TraceLength) + TraceStart, FColor::Red);

					if (bGotHitRight)
					{
						bIsWallRunRightSide = true;

						bWallRunOnRightSide = true;

						if (bIsJumpingOfWall == false)
						{
							CharacterOwner->SetActorRotation(FRotator(0, (UKismetMathLibrary::MakeRotFromX(OutRight.Normal).Yaw + 90), 0));
							FVector TargetVelocity = CharacterOwner->GetActorForwardVector() * WallRunSpeed;
							TargetVelocity.Z = WallRunZVelocity;
							Velocity = TargetVelocity;
						}
					}
					else
					{
						bIsWallRunRightSide = false;
					}
				}

				if (bIsWallRunRightSide == false)
				{
					FHitResult OutLeft;
					bool bGotHitLeft = World->LineTraceSingleByChannel
					(
						OutLeft,
						TraceStart,
						(CharacterOwner->GetActorRightVector() * -TraceLength) + TraceStart,
						ECollisionChannel::ECC_Visibility
					);

					//DrawDebugLine(GetWorld(), TraceStart, (GetActorRightVector() * -TraceLength) + TraceStart, FColor::Red);

					if (bGotHitLeft)
					{
						bIsWallRunLeftSide = true;

						bWallRunOnRightSide = false;

						if (bIsJumpingOfWall == false)
						{
							CharacterOwner->SetActorRotation(FRotator(0, (UKismetMathLibrary::MakeRotFromX(OutLeft.Normal).Yaw - 90), 0));
							FVector TargetVelocity = CharacterOwner->GetActorForwardVector() * WallRunSpeed;
							TargetVelocity.Z = WallRunZVelocity;
							Velocity = TargetVelocity;
						}
					}
					else
					{
						bIsWallRunLeftSide = false;
					}
				}				
			}
		}
	
}

void UCookieCharacterMovementComponent::WallJump()
{	
	if (bIsWallJumping == false)
	{
		FRotator TargetRotation(0, CharacterOwner->GetActorRotation().Yaw + 180, 0);
		CharacterOwner->SetActorRotation(TargetRotation);

		FVector LaunchVelocity((CharacterOwner->GetActorForwardVector() * WallJumpSpeed) + FVector(0, 0, JumpZVelocity));

		CharacterOwner->LaunchCharacter(LaunchVelocity, true, true);
		bIsWallJumping = true;

		JumpCount++;
		bCanDash = false;
		AirControl = WallJumpAirControl;

		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([&]
		{
			bIsWallJumping = false;
		});

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, JumpResetTime, false);
	}
}

void UCookieCharacterMovementComponent::WallRunJump()
{
	if (bIsJumpingOfWall == false)
	{
		FVector LaunchDirection;
		if (bWallRunOnRightSide)
		{
			LaunchDirection = CharacterOwner->GetActorRightVector() * -WallRunSpeed;
		}
		else
		{
			LaunchDirection = CharacterOwner->GetActorRightVector() * WallRunSpeed;
		}

		LaunchDirection.Z = JumpZVelocity * 0.75f;

		CharacterOwner->LaunchCharacter(LaunchDirection, false, true);
		bIsJumpingOfWall = true;

		JumpCount++;
		bCanDash = false;
		AirControl = 0;

		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([&]
			{
				bIsJumpingOfWall = false;
			});

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, JumpResetTime, false);
	}
}

bool UCookieCharacterMovementComponent::CanPerformWallRun()
{
	FVector VelocityXY = FVector(Velocity.X, Velocity.Y, 0);
	if (VelocityXY.Size() < MinimumWallRunSpeedThreshold)
	{
		bCanPerformWallRun = false;
		return bCanPerformWallRun;
	}

	if (UWorld* World = GetWorld())
	{
		FHitResult OutDown;
		FVector DownTraceStart = CharacterOwner->GetActorLocation() + (FVector::DownVector * (CapsuleHalfHeight - 5));
		FVector DownTraceEnd = DownTraceStart + (FVector::DownVector * (CapsuleHalfHeight * 2));
		bool bGotDownHit = World->LineTraceSingleByChannel
		(
			OutDown,
			DownTraceStart,
			DownTraceEnd,
			ECollisionChannel::ECC_Visibility
		);		

		float Angle = GetForwardTraceAngle();

		float DistanceToGround = FVector::Distance(DownTraceStart, OutDown.ImpactPoint);

		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, FString::Printf(TEXT("Angle = %f, Distance to ground = %f"), Angle, DistanceToGround));

		if (DistanceToGround > CapsuleHalfHeight && (Angle >= 20 && Angle <= 75))
		{
			bCanPerformWallRun = true;
		}
	}

	return bCanPerformWallRun;
}

bool UCookieCharacterMovementComponent::IsWallRunning()
{
	return (bIsWallRunRightSide || bIsWallRunLeftSide);
}

float UCookieCharacterMovementComponent::GetForwardTraceAngle()
{
	float Angle;
	if (UWorld* World = GetWorld())
	{
		FHitResult Out;
		bool bGotHit = World->LineTraceSingleByChannel
		(
			Out,
			CharacterOwner->GetActorLocation(),
			(CharacterOwner->GetActorForwardVector() * 200.0f) + CharacterOwner->GetActorLocation(),
			ECollisionChannel::ECC_Visibility
		);		

		//DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (GetActorForwardVector() * 150), FColor::Red);

		if (bGotHit)
		{
			FVector Vector1 = (CharacterOwner->GetActorForwardVector() * 200.0f);
			FVector Vector2 = Out.ImpactNormal;
			// Calculate the angle between the vectors in degrees
			Angle = 180 - FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Vector1.GetSafeNormal(), Vector2.GetSafeNormal())));
		}
	}
	
	return Angle;
}

void UCookieCharacterMovementComponent::LedgeTrace(const float DeltaSeconds)
{	
	if (UWorld* World = GetWorld())
	{
		FHitResult ForwardTraceOut;
		FVector ForwardTraceStart = CharacterOwner->GetActorLocation();
		FVector ForwardTraceEnd = CharacterOwner->GetActorLocation() + (CharacterOwner->GetActorForwardVector() * 150.0f);
		bool bForwardTraceGotHit = World->SweepSingleByChannel(ForwardTraceOut, ForwardTraceStart, ForwardTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10));

		//DrawDebugSphereTraceSingle(World, ForwardTraceStart, ForwardTraceEnd, 10, EDrawDebugTrace::ForOneFrame, bForwardTraceGotHit, ForwardTraceOut, FColor::Green, FColor::Red, 1.0f);

		if (bForwardTraceGotHit)
		{
			FHitResult HeightTraceHitOut;
			FVector HeightTraceStart = CharacterOwner->GetActorLocation() + FVector(0, 0, 500) + (CharacterOwner->GetActorForwardVector() * 75.0f);
			FVector HeightTraceEnd = HeightTraceStart - FVector(0, 0, 500);
			bool bHeightTraceGotHit = World->SweepSingleByChannel(HeightTraceHitOut, HeightTraceStart, HeightTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10));

			//DrawDebugSphereTraceSingle(World, HeightTraceStart, HeightTraceEnd, 10, EDrawDebugTrace::ForOneFrame, bHeightTraceGotHit, HeightTraceHitOut, FColor::Green, FColor::Red, 1.0f);

			if (bHeightTraceGotHit)
			{						
				CorrectedAnimOffsetLocation = (HeightTraceHitOut.ImpactPoint + (FVector::UpVector * CapsuleHalfHeight));
				DrawDebugSphere(GetWorld(), CorrectedAnimOffsetLocation, 10, 10, FColor::Green, false, 0.5f);

				float PelvisToPlatformHeight = CharacterOwner->GetMesh()->GetSocketLocation(FName("pelvisSocket")).Z - HeightTraceHitOut.Location.Z;
				if (PelvisToPlatformHeight >= -50 && PelvisToPlatformHeight <= 0)
				{
					FHitResult OutDown;
					FVector DownTraceStart = CharacterOwner->GetActorLocation();
					FVector DownTraceEnd = DownTraceStart + (FVector::DownVector * (CapsuleHalfHeight * 2));
					bool bGotDownHit = World->LineTraceSingleByChannel
					(
						OutDown,
						DownTraceStart,
						DownTraceEnd,
						ECollisionChannel::ECC_Visibility
					);		

					//DrawDebugLine(GetWorld(), DownTraceStart, DownTraceEnd, FColor::Red, false, 1.0f);

					float DistanceToGround = FVector::Distance(DownTraceStart, OutDown.ImpactPoint);
					//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, FString::Printf(TEXT("Distance to ground = %f"), DistanceToGround));
					if (Velocity.Z < 0 && bIsGrabbingLedge == false)
					{
						if (DistanceToGround > (CapsuleHalfHeight * 1.75f))
						{
							LedgeHang(ForwardTraceOut, HeightTraceHitOut);
						}
					}
				}
			}
		}
	}	

	LedgeSideTrace();

	CorrectLedgePositionOffset(DeltaSeconds);
}

void UCookieCharacterMovementComponent::LedgeHang(const FHitResult& ForwardTraceHit, const FHitResult& HeightTraceHit)
{
	if (bIsGrabbingLedge == false)
	{
		bIsGrabbingLedge = true;

		if (AnimInstance)
		{
			AnimInstance->StopAllMontages(0.2f);
		}

		SetMovementMode(MOVE_Flying);
		StopMovementImmediately();

		FVector WallNormal = ForwardTraceHit.ImpactNormal;
		FVector WallLocation = ForwardTraceHit.Location;
		FVector HeightLocation = HeightTraceHit.Location;

		FVector TargetWallLocation = (WallNormal * 30) + WallLocation;
		float MeshHeightOffset = CapsuleHalfHeight + 15;
		FVector TargetLocation = FVector(TargetWallLocation.X, TargetWallLocation.Y, HeightLocation.Z - MeshHeightOffset);
		FRotator TargetRotation = UKismetMathLibrary::MakeRotFromX(-WallNormal);
		CharacterOwner->SetActorLocationAndRotation(TargetLocation, TargetRotation);
	}
}

void UCookieCharacterMovementComponent::LedgeHangDrop()
{
	bIsGrabbingLedge = false;
	bCorrectLedgeClimbOffset = false;
	SetMovementMode(MOVE_Walking);
}

void UCookieCharacterMovementComponent::LedgeClimbToPlatform()
{
	if (LedgeClimbToPlatformMontage && AnimInstance && bIsGrabbingLedge && AnimInstance->Montage_IsPlaying(LedgeClimbToPlatformMontage) == false)
	{
		FVector AnimOffsetLocation = CharacterOwner->GetActorLocation() - (FVector::DownVector * 20);
		CharacterOwner->SetActorLocation(AnimOffsetLocation);
		AnimInstance->Montage_Play(LedgeClimbToPlatformMontage);
		FScriptDelegate OnClimbMontageNotifyBegin;
		OnClimbMontageNotifyBegin.BindUFunction(this, FName("LedgeClimbUp_AnimNotify"));
		AnimInstance->OnPlayMontageNotifyBegin.AddUnique(OnClimbMontageNotifyBegin);
	}
}

void UCookieCharacterMovementComponent::LedgeClimbUp_AnimNotify()
{
	bIsGrabbingLedge = false;
	bCorrectLedgeClimbOffset = true;
	SetMovementMode(MOVE_Walking);
}

void UCookieCharacterMovementComponent::CorrectLedgePositionOffset(const float DeltaSeconds)
{
	if (bCorrectLedgeClimbOffset)
	{		
		CharacterOwner->SetActorLocation(UKismetMathLibrary::VInterpTo(CharacterOwner->GetActorLocation(), CorrectedAnimOffsetLocation, DeltaSeconds, 15));
		float Dist = FVector::Distance(CharacterOwner->GetActorLocation(), CorrectedAnimOffsetLocation);
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, FString::Printf(TEXT("Dist = %f"), Dist));
		if (Dist < 5.0f)
		{
			bCorrectLedgeClimbOffset = false;
			CharacterOwner->SetActorLocation(CorrectedAnimOffsetLocation);
		}
	}
}

void UCookieCharacterMovementComponent::LedgeSideTrace()
{	
	if (UWorld* World = GetWorld())
	{
		FHitResult LeftTraceOut;
		FVector LeftTraceStart = (CharacterOwner->GetActorLocation() + (FVector::UpVector * CapsuleHalfHeight)) + (-CharacterOwner->GetActorRightVector() * 50) + (CharacterOwner->GetActorForwardVector() * 50);
		FVector LeftTraceEnd = LeftTraceStart + (FVector::DownVector * 25);
		bool bLeftTraceGotHit = World->SweepSingleByChannel(LeftTraceOut, LeftTraceStart, LeftTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10));

		bCanLedgeMoveLeft = bLeftTraceGotHit;

		DrawDebugSphereTraceSingle(World, LeftTraceStart, LeftTraceEnd, 10, EDrawDebugTrace::ForOneFrame, bLeftTraceGotHit, LeftTraceOut, FColor::Green, FColor::Red, 1.0f);
		
		FHitResult RightTraceOut;
		FVector RightTraceStart = (CharacterOwner->GetActorLocation() + (FVector::UpVector * CapsuleHalfHeight)) + (CharacterOwner->GetActorRightVector() * 50) + (CharacterOwner->GetActorForwardVector() * 50);
		FVector RightTraceEnd = RightTraceStart + (FVector::DownVector * 25);
		bool bRightTraceGotHit = World->SweepSingleByChannel(RightTraceOut, RightTraceStart, RightTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10));

		bCanLedgeMoveRight = bRightTraceGotHit;

		DrawDebugSphereTraceSingle(World, RightTraceStart, RightTraceEnd, 10, EDrawDebugTrace::ForOneFrame, bRightTraceGotHit, RightTraceOut, FColor::Green, FColor::Red, 1.0f);
	}
}

bool UCookieCharacterMovementComponent::CanLedgeInMove()
{
	return (bCanLedgeMoveLeft || bCanLedgeMoveRight);
}

void UCookieCharacterMovementComponent::MoveInLedge(const float DeltaSeconds)
{
	if (CookieCharacter == nullptr)
	{
		return;
	}

	bool bCanMove = (CookieCharacter->GetMoveHorizontalAxis() != 0);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, FString::Printf(TEXT("Move = %f"), CookieCharacter->GetMoveHorizontalAxis()));

	if (bCanLedgeMoveRight && bCanMove)
	{
		if (CookieCharacter->GetMoveHorizontalAxis() > 0)
		{
			 const FVector RightDirection = FRotationMatrix(CookieCharacter->GetActorRotation()).GetUnitAxis(EAxis::Y);
			 FVector TargetLocation = CookieCharacter->GetActorLocation() + (RightDirection * 20);

			 CookieCharacter->SetActorLocation(UKismetMathLibrary::VInterpTo(CookieCharacter->GetActorLocation(), TargetLocation, DeltaSeconds, 5));
		}
	}	

	if (bCanLedgeMoveLeft && bCanMove)
	{
		if (CookieCharacter->GetMoveHorizontalAxis() < 0)
		{
			 const FVector RightDirection = FRotationMatrix(CookieCharacter->GetActorRotation()).GetUnitAxis(EAxis::Y);
			 FVector TargetLocation = CookieCharacter->GetActorLocation() - (RightDirection * 20);

			 CookieCharacter->SetActorLocation(UKismetMathLibrary::VInterpTo(CookieCharacter->GetActorLocation(), TargetLocation, DeltaSeconds, 5));
		}
	}
}
