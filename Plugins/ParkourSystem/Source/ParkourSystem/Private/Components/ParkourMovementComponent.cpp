// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ParkourMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "KismetTraceUtils.h"
#include "FunctionLibrary/ParkourFunctionLibrary.h"
#include "Interfaces/ParkourABPInterface.h"
#include "DataAssets/ParkourVariablesDataAsset.h"
#include "MotionWarpingComponent.h"

// Sets default values for this component's properties
UParkourMovementComponent::UParkourMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ParkourActionTag = FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction"));
	ParkourStateTag = FGameplayTag::RequestGameplayTag(FName("Parkour.State.NotBusy"));
	ClimbStyleTag = FGameplayTag::RequestGameplayTag(FName("Parkour.ClimbStyle"));

	DrawDebugType = EDrawDebugTrace::ForDuration;
	DrawWallShapeTraceDebugType = EDrawDebugTrace::None;
	CharacterHeightDifference = 1;
}


// Called when the game starts
void UParkourMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UParkourMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Cyan, FString::Printf(TEXT("Climb Style: %s"), *ClimbStyleTag.ToString()));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Cyan, FString::Printf(TEXT("Parkour Action: %s"), *ParkourActionTag.ToString()));
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Cyan, FString::Printf(TEXT("Parkour State: %s"), *ParkourStateTag.ToString()));
	}
}

bool UParkourMovementComponent::SetInitializeReference(ACharacter* Character, USpringArmComponent* CameraBoom, UMotionWarpingComponent* MotionWarping, UCameraComponent* Camera)
{
	if (Character)
	{
		PlayerCharacter = Character;
		CharacterMesh = Character->GetMesh();
		CharacterMovement = Character->GetCharacterMovement();
		if (CharacterMesh)
		{
			CharacterAnimInstance = CharacterMesh->GetAnimInstance();
		}
		else
		{
			return false;
		}
		CharacterCapsule = Character->GetCapsuleComponent();
		CharacterCameraBoom = CameraBoom;
		CharacterMotionWarping = MotionWarping;
		CharacterCamera = Camera;
		if (CharacterCameraBoom)
		{
			DefaultCameraBoomTargetArmLength = CharacterCameraBoom->TargetArmLength;
			DefaultCameraBoomRelativeLocation = CharacterCameraBoom->GetRelativeLocation();
		}
		else
		{
			return false;
		}

		if (UWorld* World = GetWorld())
		{
			if (ArrowActorClass)
			{
				ArrowActor = World->SpawnActor(ArrowActorClass);
				if (ArrowActor)
				{
					ArrowActor->SetActorTransform(Character->GetActorTransform());
					ArrowActor->SetOwner(Character);
					FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::KeepRelative, true);
					ArrowActor->AttachToComponent(CharacterMesh, AttachmentTransformRules);
					ArrowActor->SetActorRelativeLocation(FVector(ArrowLocationX, 0, (ArrowLocationZ - CharacterHeightDifference)));
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

void UParkourMovementComponent::AddMovementInput(float ScaleValue, bool bFront)
{
	if (ParkourStateTag != FGameplayTag::RequestGameplayTag(FName("Parkour.State.ReachLedge")))
	{
		bFirstClimbMove = false;
	}

	if (bFront)
	{
		ForwardValue = ScaleValue;
		if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.NotBusy")))
		{
			const FRotator YawRotation(0, PlayerCharacter->GetControlRotation().Yaw, 0);
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			PlayerCharacter->AddMovementInput(ForwardDirection, ScaleValue);
		}
		else if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.Climb")))
		{

		}
	}
	else
	{
		RightValue = ScaleValue;
		if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.NotBusy")))
		{
			const FRotator YawRotation(0, PlayerCharacter->GetControlRotation().Yaw, 0);
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			PlayerCharacter->AddMovementInput(RightDirection, ScaleValue);
		}
		else if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.Climb")))
		{

		}
	}
}

void UParkourMovementComponent::ParkourAction(bool bAutoClimb)
{
	if (ParkourActionTag == FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")))
	{
		if (bAutoClimb)
		{
			if (bCanAutoClimb)
			{
			}
		}
		else
		{
			if (bCanManualClimb)
			{
				CheckWallShape();
				CheckDistance();
				ParkourType(bAutoClimb);
			}
		}
	}
}

void UParkourMovementComponent::CheckWallShape()
{
	if (PlayerCharacter == nullptr || CharacterMovement == nullptr)
	{
		return;
	}	

	TArray<FHitResult> 	WallHitTraces = TArray<FHitResult>();

	TArray<FHitResult> HopHitTraces = TArray<FHitResult>();

	FHitResult TopHits;

	if (UWorld* World = GetWorld())
	{
		//int LastIndex = CharacterMovement->IsFalling() ? 15 : 8;
		for (int Index = 0; Index <= 15; Index++)
		{
			bool bShouldBreak = false;
			for (int Index2 = 0; Index2 <= 11; Index2++)
			{
				FHitResult TraceHitOut;
				FVector TraceStart = (FVector(0, 0, Index * 16) + FVector(0, 0, -60) + PlayerCharacter->GetActorLocation()) + (PlayerCharacter->GetActorForwardVector() * -20);
				FVector TraceEnd = (FVector(0, 0, Index * 16) + FVector(0, 0, -60) + PlayerCharacter->GetActorLocation()) + (PlayerCharacter->GetActorForwardVector() * ((10 * Index2) + 10));
				
				FCollisionQueryParams Params = FCollisionQueryParams();
				Params.bTraceComplex = false;
				Params.AddIgnoredActor(PlayerCharacter);
				bool bTraceGotHit = World->SweepSingleByChannel(TraceHitOut, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10), Params);

				DrawDebugSphereTraceSingle(World, TraceStart, TraceEnd, 10, DrawWallShapeTraceDebugType, bTraceGotHit, TraceHitOut, FColor::Red, FColor::Green, 1.0f);

				if (TraceHitOut.bBlockingHit && TraceHitOut.bStartPenetrating == false)
				{
					bShouldBreak = true;

					WallHitTraces.Empty();
					int LastIndex3 = UParkourFunctionLibrary::SelectParkoutStateFloat(4, 0, 0, 2, ParkourStateTag);
					for (int Index3 = 0; Index3 <= LastIndex3; Index3++)
					{						
						float TargetZ = (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.Climb"))) ? 0.0f : -60.0f;
						FVector Vector1 = FVector(0, 0, TargetZ);

						TargetZ = (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.Climb"))) ? TraceHitOut.ImpactPoint.Z : PlayerCharacter->GetActorLocation().Z;
						FVector Vector2 = FVector(TraceHitOut.ImpactPoint.X, TraceHitOut.ImpactPoint.Y, TargetZ);

						float VectorMultiplier = (Index3 * 20) + UParkourFunctionLibrary::SelectParkoutStateFloat(-40, 0, 0, -20, ParkourStateTag);
						FRotator ReveresedImpactNormal = UParkourFunctionLibrary::NormalReverseRotationZ(TraceHitOut.ImpactNormal);
						FVector ReveresedImpactNormalForwardVector = FRotationMatrix(ReveresedImpactNormal).GetUnitAxis(EAxis::X);
						FVector ReveresedImpactNormalRightVector = FRotationMatrix(ReveresedImpactNormal).GetUnitAxis(EAxis::Y);

						FVector Vector3 = ReveresedImpactNormalRightVector * VectorMultiplier;
						FVector Vector4 = ReveresedImpactNormalForwardVector * -40;
						FVector Vector5 = ReveresedImpactNormalForwardVector * 30;

						FVector LineTraceStart = Vector1 + Vector2 + Vector3 + Vector4;
						FVector LineTraceEnd = Vector1 + Vector2 + Vector3 + Vector5;

						FHitResult LineTraceHitOut;
						FCollisionQueryParams LineTraceParams = FCollisionQueryParams();
						LineTraceParams.bTraceComplex = false;
						LineTraceParams.AddIgnoredActor(PlayerCharacter);
						bool bLineTraceGotHit = World->LineTraceSingleByChannel(LineTraceHitOut, LineTraceStart, LineTraceEnd, ECC_Visibility, LineTraceParams);

						DrawDebugLineTraceSingle(World, LineTraceStart, LineTraceEnd, DrawWallShapeTraceDebugType, bLineTraceGotHit, LineTraceHitOut, FColor::Red, FColor::Green, 1.0f);

						HopHitTraces.Empty();
						int LastIndex4 = UParkourFunctionLibrary::SelectParkoutStateFloat(30, 0, 0, 7, ParkourStateTag);
						for (int Index4 = 0; Index4 <= LastIndex4; Index4++)
						{
							FVector LineTrace2Start = LineTraceHitOut.TraceStart + FVector(0, 0, (Index4 * 8));
							FVector LineTrace2End = LineTraceHitOut.TraceEnd + FVector(0, 0, (Index4 * 8));

							FHitResult LineTrace2HitOut;
							FCollisionQueryParams LineTrace2Params = FCollisionQueryParams();
							LineTrace2Params.bTraceComplex = false;
							LineTrace2Params.AddIgnoredActor(PlayerCharacter);
							bool bLineTrace2GotHit = World->LineTraceSingleByChannel(LineTrace2HitOut, LineTrace2Start, LineTrace2End, ECC_Visibility, LineTrace2Params);

							DrawDebugLineTraceSingle(World, LineTrace2Start, LineTrace2End, DrawWallShapeTraceDebugType, bLineTrace2GotHit, LineTrace2HitOut, FColor::Red, FColor::Green, 1.0f);

							HopHitTraces.Add(LineTrace2HitOut);
						}

						int LastIndex5 = HopHitTraces.Num();
						for (int Index5 = 1; Index5 < LastIndex5; Index5++)
						{
							const FHitResult& HopHitResult = HopHitTraces[Index5];
							const FHitResult& PrevHopHitResult = HopHitTraces[Index5 - 1];
							float Distance = (HopHitResult.bBlockingHit) ? HopHitResult.Distance : FVector::Distance(HopHitResult.TraceStart, HopHitResult.TraceEnd);
							float PrevDistance = (PrevHopHitResult.bBlockingHit) ? PrevHopHitResult.Distance : FVector::Distance(PrevHopHitResult.TraceStart, PrevHopHitResult.TraceEnd);

							if ((Distance - PrevDistance) > 5.0f)
							{
								WallHitTraces.Add(PrevHopHitResult);
								break;
							}						
						}
					}					
					
					int LastIndex4 = WallHitTraces.Num();
					for (int Index4 = 0; Index4 < LastIndex4; Index4++)
					{
						if (Index4 == 0)
						{
							WallHitResult = WallHitTraces[0];
						}
						else
						{
							float DistanceToWallHit = FVector::Distance(WallHitResult.ImpactPoint, PlayerCharacter->GetActorLocation());
							float Distance = FVector::Distance(WallHitTraces[Index4].ImpactPoint, PlayerCharacter->GetActorLocation());

							//Find shortest wall hit result
							if (Distance <= DistanceToWallHit)
							{
								WallHitResult = WallHitTraces[Index4];
							}
						}
					}

					if (WallHitResult.bBlockingHit && WallHitResult.bStartPenetrating == false)
					{
						if (ParkourStateTag != FGameplayTag::RequestGameplayTag(FName("Parkour.State.Climb")))
						{
							WallRotation = UParkourFunctionLibrary::NormalReverseRotationZ(WallHitResult.ImpactNormal);
						}

						for (int Index5 = 0; Index5 <= 8; Index5++)
						{
							FVector WallRotationForward = FRotationMatrix(WallRotation).GetUnitAxis(EAxis::X);
							FVector SphereTraceStart = WallHitResult.ImpactPoint + (WallRotationForward * (Index5 * 30)) + (WallRotationForward * 2.0f) + FVector(0, 0, 7);
							FVector SphereTraceEnd = SphereTraceStart - FVector(0, 0, 7);
							
							FHitResult SphereTraceHitOut;
							FCollisionQueryParams SphereParams = FCollisionQueryParams();
							SphereParams.bTraceComplex = false;
							SphereParams.AddIgnoredActor(PlayerCharacter);
							bool bSphereTraceGotHit = World->SweepSingleByChannel(SphereTraceHitOut, SphereTraceStart, SphereTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(2.5f), SphereParams);

							DrawDebugSphereTraceSingle(World, SphereTraceStart, SphereTraceEnd, 2.5f, DrawWallShapeTraceDebugType, bSphereTraceGotHit, SphereTraceHitOut, FColor::Red, FColor::Green, 1.0f);

							if (Index5 == 0)
							{
								if (bSphereTraceGotHit)
								{
									WallTopResult = SphereTraceHitOut;
								}
							}

							if (bSphereTraceGotHit)
							{
								TopHits = SphereTraceHitOut;
							}
							else
							{
								if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.NotBusy")))
								{
									FVector SphereTrace2Start = TopHits.ImpactPoint + (WallRotationForward * 30);
									FVector SphereTrace2End = TopHits.ImpactPoint;
							
									FHitResult SphereTrace2HitOut;
									FCollisionQueryParams SphereParams2 = FCollisionQueryParams();
									SphereParams2.bTraceComplex = false;
									SphereParams2.AddIgnoredActor(PlayerCharacter);
									bool bSphereTrace2GotHit = World->SweepSingleByChannel(SphereTrace2HitOut, SphereTrace2Start, SphereTrace2End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(2.5f), SphereParams2);

									DrawDebugSphereTraceSingle(World, SphereTrace2Start, SphereTrace2End, 2.5f, DrawWallShapeTraceDebugType, bSphereTrace2GotHit, SphereTrace2HitOut, FColor::Red, FColor::Green, 1.0f);

									if (bSphereTrace2GotHit)
									{
										WallDepthResult = SphereTrace2HitOut;

										FVector SphereTrace3Start = WallDepthResult.ImpactPoint + (WallRotationForward * 70);
										FVector SphereTrace3End = SphereTrace3Start - FVector(0, 0, 200);
							
										FHitResult SphereTrace3HitOut;
										FCollisionQueryParams SphereParams3 = FCollisionQueryParams();
										SphereParams3.bTraceComplex = false;
										SphereParams3.AddIgnoredActor(PlayerCharacter);
										bool bSphereTrace3GotHit = World->SweepSingleByChannel(SphereTrace3HitOut, SphereTrace3Start, SphereTrace3End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10.0f), SphereParams3);

										DrawDebugSphereTraceSingle(World, SphereTrace3Start, SphereTrace3End, 10.0f, DrawWallShapeTraceDebugType, bSphereTrace3GotHit, SphereTrace3HitOut, FColor::Red, FColor::Green, 1.0f);

										if (bSphereTrace3GotHit)
										{
											WallVaultResult = SphereTrace3HitOut;
										}
									}
								}
								break;
							}
						}
					}
					break;
				}
			}
			if (bShouldBreak)
			{
				break;
			}
		}
	}

	ShowHitResults();
}

void UParkourMovementComponent::ShowHitResults()
{
	if (bShowHitResults)
	{
		if (UWorld* World = GetWorld())
		{
			DrawDebugSphere(World, WallTopResult.ImpactPoint, 5, 12, FColor::Blue, false, 1.0f);
			DrawDebugSphere(World, WallDepthResult.ImpactPoint, 5, 12, FColor::Red, false, 1.0f);
			DrawDebugSphere(World, WallVaultResult.ImpactPoint, 5, 12, FColor::Green, false, 1.0f);
		}
	}
}

void UParkourMovementComponent::CheckDistance()
{
	if (WallHitResult.bBlockingHit)
	{
		if (WallTopResult.bBlockingHit)
		{
			WallHeight = WallTopResult.ImpactPoint.Z - CharacterMesh->GetSocketLocation(FName("root")).Z;
		}
		else
		{
			WallHeight = 0;
		}

		if (WallTopResult.bBlockingHit && WallDepthResult.bBlockingHit)
		{
			WallDepth = FVector::Distance(WallTopResult.ImpactPoint, WallDepthResult.ImpactPoint);
		}
		else
		{
			WallDepth = 0;
		}

		if (WallVaultResult.bBlockingHit && WallDepthResult.bBlockingHit)
		{
			VaultHeight = WallDepthResult.ImpactPoint.Z - WallVaultResult.ImpactPoint.Z;
		}
		else
		{
			VaultHeight = 0;
		}
	}
	else
	{
		WallHeight = 0;
		WallDepth = 0;
		VaultHeight = 0;
	}
}

void UParkourMovementComponent::ParkourType(const bool bAutoClimb)
{
	if (WallTopResult.bBlockingHit)
	{
		if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.NotBusy")))
		{
			if (bInGround)
			{
				if (WallHeight > 90 && WallHeight <= 120)
				{
					if (WallDepth >= 0 && WallDepth <= 120)
					{
						if (VaultHeight >= 60 && VaultHeight <= 160)
						{
							if (WallDepth >= 0 && WallDepth <= 30)
							{
								CheckSurfaceAndSetActionAsThinVault();
							}
							else
							{
								if (CharacterMovement->Velocity.Length() > 20)
								{
									CheckSurfaceAndSetActionAsVault();
								}
								else
								{
									CheckSurfaceAndSetActionAsMantle();
								}
							}
						}
						else
						{
							CheckSurfaceAndSetActionAsMantle();
						}
					}
					else
					{
						CheckSurfaceAndSetActionAsMantle();
					}
				}
				else
				{
					if (WallHeight > 44 && WallHeight <= 90)
					{
						CheckSurfaceAndSetActionAsLowMantle();
					}
					else
					{
						if (WallHeight > 120 && WallHeight <= 160)
						{
							if (WallDepth > 0 && WallDepth <= 120)
							{
								if (VaultHeight >= 60 && VaultHeight <= 165)
								{
									if (CharacterMovement->Velocity.Length() > 20)
									{
										CheckSurfaceAndSetActionAsHighVault();
									}
									else
									{
										CheckSurfaceAndSetActionAsMantle();
									}
								}
								else
								{
									CheckSurfaceAndSetActionAsMantle();
								}
							}
							else
							{
								CheckSurfaceAndSetActionAsMantle();
							}
						}
						else
						{
							if (WallHeight > 0 && WallHeight <= 44)
							{
								SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")));
							}
							else
							{
								if (WallHeight > 250)
								{
									SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")));
								}
								else
								{
									if (CheckClimbSurface())
									{
										CheckClimbStyle();
										GetClimbedLedgeHitResult();
										if (ClimbStyleTag == FGameplayTag::RequestGameplayTag(FName("Parkour.ClimbStyle.Braced")))
										{
											SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.Climb")));
										}
										else
										{
											SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.FreeHangClimb")));
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")));
			}
		}
		else if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.Climb")))
		{

		}
	}
	else
	{
		SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")));
		if (bAutoClimb == false)
		{
			PlayerCharacter->Jump();
		}
	}
}

void UParkourMovementComponent::CheckSurfaceAndSetActionAsHighVault()
{
	if (CheckVaultSurface())
	{
		SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.HighVault")));
	}
	else
	{
		SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")));
	}
}

void UParkourMovementComponent::CheckSurfaceAndSetActionAsLowMantle()
{
	if (CheckMantleSurface())
	{
		SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.LowMantle")));
	}
	else
	{
		SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")));
	}
}

void UParkourMovementComponent::CheckSurfaceAndSetActionAsMantle()
{
	if (CheckMantleSurface())
	{
		SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.Mantle")));
	}
	else
	{
		SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")));
	}
}

void UParkourMovementComponent::CheckSurfaceAndSetActionAsVault()
{
	if (CheckVaultSurface())
	{
		SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.Vault")));
	}
	else
	{
		SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")));
	}
}

void UParkourMovementComponent::CheckSurfaceAndSetActionAsThinVault()
{
	if (CheckVaultSurface())
	{
		SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.ThinVault")));
	}
	else
	{
		SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")));
	}
}

void UParkourMovementComponent::SetParkourAction(FGameplayTag NewParkourActionTag)
{
	if (ParkourActionTag != NewParkourActionTag)
	{
		ParkourActionTag = NewParkourActionTag;
		if (CharacterAnimInstance)
		{
			if (UClass* AnimClass = CharacterAnimInstance->GetClass())
			{
				if (AnimClass->ImplementsInterface(UParkourABPInterface::StaticClass()))
				{
					IParkourABPInterface::Execute_SetParkourAction(Cast<UObject>(CharacterAnimInstance), ParkourActionTag);
				}
			}

			if (ParkourActionTag == FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")))
			{
				ParkourVariablesDataAsset = nullptr;
				ResetParkourResult();
			}
			else if (ParkourActionTag == FGameplayTag::RequestGameplayTag(FName("Parkour.Action.ThinVault")))
			{
				ParkourVariablesDataAsset = ThinVaultDataAsset;
			}
			else if (ParkourActionTag == FGameplayTag::RequestGameplayTag(FName("Parkour.Action.HighVault")))
			{
				ParkourVariablesDataAsset = HighVaultDataAsset;
			}
			else if (ParkourActionTag == FGameplayTag::RequestGameplayTag(FName("Parkour.Action.Vault")))
			{
				ParkourVariablesDataAsset = VaultDataAsset;
			}
			else if (ParkourActionTag == FGameplayTag::RequestGameplayTag(FName("Parkour.Action.Mantle")))
			{
				ParkourVariablesDataAsset = MantleDataAsset;
			}
			else if (ParkourActionTag == FGameplayTag::RequestGameplayTag(FName("Parkour.Action.LowMantle")))
			{
				ParkourVariablesDataAsset = LowMantleDataAsset;
			}
			else if (ParkourActionTag == FGameplayTag::RequestGameplayTag(FName("Parkour.Action.Climb")))
			{
				ParkourVariablesDataAsset = BracedClimbDataAsset;
			}
			else if (ParkourActionTag == FGameplayTag::RequestGameplayTag(FName("Parkour.Action.FreeHangClimb")))
			{
				ParkourVariablesDataAsset = FreeHangClimbDataAsset;
			}

			PlayParkourMontage();
		}
	}
}

void UParkourMovementComponent::SetParkourState(FGameplayTag NewParkourStateTag)
{
	if (ParkourStateTag != NewParkourStateTag)
	{
		ParkourStateTag = NewParkourStateTag;
		if (CharacterAnimInstance)
		{
			if (UClass* AnimClass = CharacterAnimInstance->GetClass())
			{
				if (AnimClass->ImplementsInterface(UParkourABPInterface::StaticClass()))
				{
					IParkourABPInterface::Execute_SetParkourState(Cast<UObject>(CharacterAnimInstance), ParkourStateTag);
				}
			}

			if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.NotBusy")))
			{
				ParkourStateSettings(ECollisionEnabled::QueryAndPhysics, MOVE_Walking, FRotator(0, 500, 0), true, false);
			}
			else if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.Mantle")))
			{
				ParkourStateSettings(ECollisionEnabled::NoCollision, MOVE_Flying, FRotator(0, 500, 0), true, false);
			}
			else if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.Vault")))
			{
				ParkourStateSettings(ECollisionEnabled::NoCollision, MOVE_Flying, FRotator(0, 500, 0), true, false);
			}
			else if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.Climb")))
			{
				ParkourStateSettings(ECollisionEnabled::NoCollision, MOVE_Flying, FRotator::ZeroRotator, true, true);
			}
			else if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.ReachLedge")))
			{
				ParkourStateSettings(ECollisionEnabled::NoCollision, MOVE_Flying, FRotator(0, 500, 0), true, false);
			}
		}
	}
}

void UParkourMovementComponent::SetClimbStyle(FGameplayTag NewClimbStyle)
{
	if (ClimbStyleTag != NewClimbStyle)
	{
		ClimbStyleTag = NewClimbStyle;
		if (CharacterAnimInstance)
		{
			if (UClass* AnimClass = CharacterAnimInstance->GetClass())
			{
				if (AnimClass->ImplementsInterface(UParkourABPInterface::StaticClass()))
				{
					IParkourABPInterface::Execute_SetClimbStyle(Cast<UObject>(CharacterAnimInstance), ClimbStyleTag);
				}
			}
		}
	}
}

bool UParkourMovementComponent::CheckMantleSurface()
{
	bool bCapsuleTraceGotHit = false;
	if (UWorld* World = GetWorld())
	{
		FVector CapsuleTraceStart = WallTopResult.ImpactPoint + FVector(0, 0, CharacterCapsule->GetScaledCapsuleHalfHeight() + 8);
		FVector CapsuleTraceEnd = CapsuleTraceStart;

		FHitResult CapsuleTraceHitOut;
		FCollisionQueryParams CapsuleParams = FCollisionQueryParams();
		CapsuleParams.bTraceComplex = false;
		CapsuleParams.AddIgnoredActor(PlayerCharacter);
		bCapsuleTraceGotHit = World->SweepSingleByChannel(CapsuleTraceHitOut, CapsuleTraceStart, CapsuleTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeCapsule(25, CharacterCapsule->GetScaledCapsuleHalfHeight() - 8), CapsuleParams);
		
		DrawDebugCapsuleTraceSingle(World, CapsuleTraceStart, CapsuleTraceEnd, 25, CharacterCapsule->GetScaledCapsuleHalfHeight(), DrawDebugType, bCapsuleTraceGotHit, CapsuleTraceHitOut, FColor::Red, FColor::Green, 1.0f);
	}

	return (!bCapsuleTraceGotHit);
}

bool UParkourMovementComponent::CheckVaultSurface()
{
	bool bCapsuleTraceGotHit = false;
	if (UWorld* World = GetWorld())
	{
		FVector CapsuleTraceStart = WallTopResult.ImpactPoint + FVector(0, 0, (CharacterCapsule->GetScaledCapsuleHalfHeight()/2) + 18);
		FVector CapsuleTraceEnd = CapsuleTraceStart;

		FHitResult CapsuleTraceHitOut;
		FCollisionQueryParams CapsuleParams = FCollisionQueryParams();
		CapsuleParams.bTraceComplex = false;
		CapsuleParams.AddIgnoredActor(PlayerCharacter);
		bCapsuleTraceGotHit = World->SweepSingleByChannel(CapsuleTraceHitOut, CapsuleTraceStart, CapsuleTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeCapsule(25, (CharacterCapsule->GetScaledCapsuleHalfHeight()/2) + 5), CapsuleParams);
		
		DrawDebugCapsuleTraceSingle(World, CapsuleTraceStart, CapsuleTraceEnd, 25, CharacterCapsule->GetScaledCapsuleHalfHeight(), DrawDebugType, bCapsuleTraceGotHit, CapsuleTraceHitOut, FColor::Red, FColor::Green, 1.0f);
	}

	return (!bCapsuleTraceGotHit);
}

bool UParkourMovementComponent::CheckClimbSurface()
{
	bool bCapsuleTraceGotHit = false;
	if (UWorld* World = GetWorld())
	{
		FVector CapsuleTraceStart = WallTopResult.ImpactPoint + FVector(0, 0, -90) + FRotationMatrix(WallRotation).GetUnitAxis(EAxis::X) * -55;
		FVector CapsuleTraceEnd = CapsuleTraceStart;

		FHitResult CapsuleTraceHitOut;
		FCollisionQueryParams CapsuleParams = FCollisionQueryParams();
		CapsuleParams.bTraceComplex = false;
		CapsuleParams.AddIgnoredActor(PlayerCharacter);
		bCapsuleTraceGotHit = World->SweepSingleByChannel(CapsuleTraceHitOut, CapsuleTraceStart, CapsuleTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeCapsule(25, 82), CapsuleParams);
		
		DrawDebugCapsuleTraceSingle(World, CapsuleTraceStart, CapsuleTraceEnd, 25, CharacterCapsule->GetScaledCapsuleHalfHeight(), DrawDebugType, bCapsuleTraceGotHit, CapsuleTraceHitOut, FColor::Red, FColor::Green, 1.0f);
	}

	return (!bCapsuleTraceGotHit);
}

void UParkourMovementComponent::CheckClimbStyle()
{	
	if (UWorld* World = GetWorld())
	{
		FVector SphereTraceStart = WallTopResult.ImpactPoint + (FRotationMatrix(WallRotation).GetUnitAxis(EAxis::X) * -10) + FVector(0, 0, -125);
		FVector SphereTraceEnd = WallTopResult.ImpactPoint + (FRotationMatrix(WallRotation).GetUnitAxis(EAxis::X) * 30) + FVector(0, 0, -125);

		FHitResult SphereTraceHitOut;
		FCollisionQueryParams SphereParams = FCollisionQueryParams();
		SphereParams.bTraceComplex = false;
		SphereParams.AddIgnoredActor(PlayerCharacter);
		bool bSphereTraceGotHit = World->SweepSingleByChannel(SphereTraceHitOut, SphereTraceStart, SphereTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10), SphereParams);

		DrawDebugSphereTraceSingle(World, SphereTraceStart, SphereTraceEnd, 10, DrawDebugType, bSphereTraceGotHit, SphereTraceHitOut, FColor::Red, FColor::Green, 1.0f);

		if (bSphereTraceGotHit)
		{
			SetClimbStyle(FGameplayTag::RequestGameplayTag(FName("Parkour.ClimbStyle.Braced")));
		}
		else
		{
			SetClimbStyle(FGameplayTag::RequestGameplayTag(FName("Parkour.ClimbStyle.FreeHang")));
		}
	}
}

void UParkourMovementComponent::GetClimbedLedgeHitResult()
{
	if (UWorld* World = GetWorld())
	{
		FVector SphereTraceStart = WallHitResult.ImpactPoint + (FRotationMatrix(UParkourFunctionLibrary::NormalReverseRotationZ(WallHitResult.ImpactNormal)).GetUnitAxis(EAxis::X) * -30);
		FVector SphereTraceEnd = WallHitResult.ImpactPoint + (FRotationMatrix(UParkourFunctionLibrary::NormalReverseRotationZ(WallHitResult.ImpactNormal)).GetUnitAxis(EAxis::X) * 30);

		FHitResult SphereTraceHitOut;
		FCollisionQueryParams SphereParams = FCollisionQueryParams();
		SphereParams.bTraceComplex = false;
		SphereParams.AddIgnoredActor(PlayerCharacter);
		bool bSphereTraceGotHit = World->SweepSingleByChannel(SphereTraceHitOut, SphereTraceStart, SphereTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(10), SphereParams);

		DrawDebugSphereTraceSingle(World, SphereTraceStart, SphereTraceEnd, 10, DrawDebugType, bSphereTraceGotHit, SphereTraceHitOut, FColor::Red, FColor::Green, 1.0f);

		if (bSphereTraceGotHit)
		{
			WallRotation = UParkourFunctionLibrary::NormalReverseRotationZ(SphereTraceHitOut.ImpactNormal);

			FVector SphereTrace2Start = SphereTraceHitOut.ImpactPoint + (FRotationMatrix(UParkourFunctionLibrary::NormalReverseRotationZ(SphereTraceHitOut.ImpactNormal)).GetUnitAxis(EAxis::X) * 2) + FVector(0, 0, 5);
			FVector SphereTrace2End = SphereTrace2Start - FVector(0, 0, 50);

			FHitResult SphereTrace2HitOut;
			FCollisionQueryParams SphereParams2 = FCollisionQueryParams();
			SphereParams2.bTraceComplex = false;
			SphereParams2.AddIgnoredActor(PlayerCharacter);
			bool bSphereTrace2GotHit = World->SweepSingleByChannel(SphereTrace2HitOut, SphereTrace2Start, SphereTrace2End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(5), SphereParams2);

			DrawDebugSphereTraceSingle(World, SphereTrace2Start, SphereTrace2End, 5, DrawDebugType, bSphereTrace2GotHit, SphereTrace2HitOut, FColor::Red, FColor::Green, 1.0f);

			if (bSphereTrace2GotHit)
			{
				ClimbedLedgeHitResult = SphereTraceHitOut;
				ClimbedLedgeHitResult.Location = SphereTrace2HitOut.Location;
				ClimbedLedgeHitResult.ImpactPoint = FVector(SphereTraceHitOut.ImpactPoint.X, SphereTraceHitOut.ImpactPoint.Y, SphereTrace2HitOut.ImpactPoint.Z);
			}
		}
	}
}

void UParkourMovementComponent::PlayParkourMontage()
{	
	if (CharacterMotionWarping && ParkourVariablesDataAsset)
	{	
		SetParkourState(ParkourVariablesDataAsset->ParkourInState);
		CharacterMotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FName("Warp 1"), FindWarpTargetLocation_1(ParkourVariablesDataAsset->Warp1XOffset, ParkourVariablesDataAsset->Warp1ZOffset), WallRotation);
		CharacterMotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FName("Warp 2"), FindWarpTargetLocation_2(ParkourVariablesDataAsset->Warp2XOffset, ParkourVariablesDataAsset->Warp2ZOffset), WallRotation);
		CharacterMotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FName("Warp 3"), FindWarpTargetLocation_3(ParkourVariablesDataAsset->Warp3XOffset, ParkourVariablesDataAsset->Warp3ZOffset), WallRotation);
		CharacterMotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FName("Warp 4"), FindWarpTargetLocation_4(ParkourVariablesDataAsset->Warp2XOffset, ParkourVariablesDataAsset->Warp2ZOffset), WallRotation);
		CharacterMotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(FName("Warp 5"), FindWarpTargetLocation_1(ParkourVariablesDataAsset->Warp2XOffset, ParkourVariablesDataAsset->Warp2ZOffset), WallRotation);

		if (CharacterAnimInstance && ParkourVariablesDataAsset->ParkourMontage)
		{
			CharacterAnimInstance->Montage_Play(ParkourVariablesDataAsset->ParkourMontage, 1.0f, EMontagePlayReturnType::MontageLength, ParkourVariablesDataAsset->MontageStartPosition);
			CharacterAnimInstance->Montage_GetBlendingOutDelegate(ParkourVariablesDataAsset->ParkourMontage)->BindUFunction(this, FName("OnMontageBlendOut"));
			MontageBlendOutState = ParkourVariablesDataAsset->ParkourOutState;
		}
	}
}

void UParkourMovementComponent::OnMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	SetParkourState(MontageBlendOutState);
	SetParkourAction(FGameplayTag::RequestGameplayTag(FName("Parkour.Action.NoAction")));
}

FVector UParkourMovementComponent::FindWarpTargetLocation_1(const float WarpXOffset, const float WarpZOffset)
{
	return (WallTopResult.ImpactPoint + (FRotationMatrix(WallRotation).GetUnitAxis(EAxis::X) * WarpXOffset) + FVector(0, 0, WarpZOffset));
}

FVector UParkourMovementComponent::FindWarpTargetLocation_2(const float WarpXOffset, const float WarpZOffset)
{
	return (WallDepthResult.ImpactPoint + (FRotationMatrix(WallRotation).GetUnitAxis(EAxis::X) * WarpXOffset) + FVector(0, 0, WarpZOffset));
}

FVector UParkourMovementComponent::FindWarpTargetLocation_3(const float WarpXOffset, const float WarpZOffset)
{
	return (WallVaultResult.ImpactPoint + (FRotationMatrix(WallRotation).GetUnitAxis(EAxis::X) * WarpXOffset) + FVector(0, 0, WarpZOffset));
}

FVector UParkourMovementComponent::FindWarpTargetLocation_4(const float WarpXOffset, const float WarpZOffset)
{
	if (UWorld* World = GetWorld())
	{
		FVector SphereTraceStart = WallTopResult.ImpactPoint + (FRotationMatrix(WallRotation).GetUnitAxis(EAxis::X) * WarpXOffset) + FVector(0, 0, 40);
		FVector SphereTraceEnd = SphereTraceStart - FVector(0, 0, 60);

		FHitResult SphereTraceHitOut;
		FCollisionQueryParams SphereParams = FCollisionQueryParams();
		SphereParams.bTraceComplex = false;
		SphereParams.AddIgnoredActor(PlayerCharacter);
		bool bSphereTraceGotHit = World->SweepSingleByChannel(SphereTraceHitOut, SphereTraceStart, SphereTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(2.5f), SphereParams);

		DrawDebugSphereTraceSingle(World, SphereTraceStart, SphereTraceEnd, 2.5f, DrawDebugType, bSphereTraceGotHit, SphereTraceHitOut, FColor::Red, FColor::Green, 1.0f);

		if (bSphereTraceGotHit)
		{
			return SphereTraceHitOut.ImpactPoint + FVector(0, 0, WarpZOffset);
		}
	}

	return WallTopResult.ImpactPoint + FVector(0, 0, WarpZOffset);
}

void UParkourMovementComponent::ParkourStateSettings(ECollisionEnabled::Type NewType, EMovementMode NewMovementMode, FRotator RotationRate, bool bDoCollisionTest, bool bStopMovementImmediately)
{
	if (CharacterCapsule && CharacterMovement && CharacterCameraBoom)
	{
		CharacterCapsule->SetCollisionEnabled(NewType);
		CharacterMovement->SetMovementMode(NewMovementMode);
		CharacterMovement->RotationRate = RotationRate;
		if (bStopMovementImmediately)
		{
			CharacterMovement->StopMovementImmediately();
		}
		CharacterCameraBoom->bDoCollisionTest = bDoCollisionTest;
	}
}

void UParkourMovementComponent::ResetParkourResult()
{
	WallHitResult = FHitResult();
	WallTopResult = FHitResult();
	WallDepthResult = FHitResult();
	WallVaultResult = FHitResult();
	ClimbedLedgeHitResult = FHitResult();
}

void UParkourMovementComponent::LimbsClimbIK(bool bFirst, bool bIsLeft)
{
	int LimbDir = bIsLeft ? -1 : 1;
	if (bFirst == false)
	{
		FHitResult LedgeHitResult = ClimbedLedgeHitResult;
		if (ParkourStateTag == FGameplayTag::RequestGameplayTag(FName("Parkour.State.ReachLedge")))
		{
			if (LedgeHitResult.bBlockingHit)
			{
				for (int Index = 0; Index <= 4; Index++)
				{
					bool bBreakThisLoop = false;
					if (UWorld* World = GetWorld())
					{	
						FVector WallForwardRotation = FRotationMatrix(WallRotation).GetUnitAxis(EAxis::X);
						FVector WallRightRotation = FRotationMatrix(WallRotation).GetUnitAxis(EAxis::Y);

						FVector SphereTraceStart = (WallForwardRotation * -20) + (WallRightRotation * ((8 * LimbDir) - (Index * (2 * LimbDir)))) + LedgeHitResult.ImpactPoint;
						FVector SphereTraceEnd = (WallForwardRotation * 20) + (WallRightRotation * ((8 * LimbDir) - (Index * (2 * LimbDir)))) + LedgeHitResult.ImpactPoint;

						FHitResult SphereTraceHitOut;
						FCollisionQueryParams SphereParams = FCollisionQueryParams();
						SphereParams.bTraceComplex = false;
						SphereParams.AddIgnoredActor(PlayerCharacter);
						bool bSphereTraceGotHit = World->SweepSingleByChannel(SphereTraceHitOut, SphereTraceStart, SphereTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(5.0f), SphereParams);

						DrawDebugSphereTraceSingle(World, SphereTraceStart, SphereTraceEnd, 5.0f, DrawDebugType, bSphereTraceGotHit, SphereTraceHitOut, FColor::Red, FColor::Green, 3.0f);

						if (bSphereTraceGotHit)
						{
							for (int Index2 = 0; Index2 <= 5; Index2++)
							{
								FRotator ReversedImpactNormal = UParkourFunctionLibrary::NormalReverseRotationZ(SphereTraceHitOut.ImpactNormal);
								FVector ReversedImpactNormalForward = FRotationMatrix(ReversedImpactNormal).GetUnitAxis(EAxis::X);

								FVector SphereTrace2Start = SphereTraceHitOut.ImpactPoint + (ReversedImpactNormalForward * 2) + FVector(0, 0, Index2 * 5);
								FVector SphereTrace2End = SphereTrace2Start - FVector(0, 0, (Index2 * 5) + 50);

								FHitResult SphereTrace2HitOut;
								FCollisionQueryParams SphereParams2 = FCollisionQueryParams();
								SphereParams2.bTraceComplex = false;
								SphereParams2.AddIgnoredActor(PlayerCharacter);
								bool bSphereTrace2GotHit = World->SweepSingleByChannel(SphereTrace2HitOut, SphereTrace2Start, SphereTrace2End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(5.0f), SphereParams2);

								DrawDebugSphereTraceSingle(World, SphereTrace2Start, SphereTrace2End, 5.0f, DrawDebugType, bSphereTrace2GotHit, SphereTrace2HitOut, FColor::Red, FColor::Green, 3.0f);

								if (SphereTrace2HitOut.bStartPenetrating == false)
								{
									if (SphereTrace2HitOut.bBlockingHit)
									{
										bBreakThisLoop = true;
																		
										if (CharacterAnimInstance)
										{
											if (UClass* AnimClass = CharacterAnimInstance->GetClass())
											{
												if (AnimClass->ImplementsInterface(UParkourABPInterface::StaticClass()))
												{
													float Offset = (ClimbStyleTag == FGameplayTag::RequestGameplayTag(FName("Parkour.ClimbStyle.Braced"))) ? CharacterHandFrontDifference : 0;
													FVector Vector = SphereTraceHitOut.ImpactPoint + (ReversedImpactNormalForward * (-3 + Offset));
													float TargetHandZ = SphereTrace2HitOut.ImpactPoint.Z + CharacterHeightDifference + CharacterHandUpDifference - 9;

													if (bIsLeft)
													{
														IParkourABPInterface::Execute_SetLeftHandLedgeLocation(Cast<UObject>(CharacterAnimInstance), FVector(Vector.X, Vector.Y, TargetHandZ));

														IParkourABPInterface::Execute_SetLeftHandLedgeRotation(Cast<UObject>(CharacterAnimInstance), FRotator(ReversedImpactNormal.Pitch + 90, ReversedImpactNormal.Yaw, ReversedImpactNormal.Roll + 270));
													}
													else
													{
														IParkourABPInterface::Execute_SetRightHandLedgeLocation(Cast<UObject>(CharacterAnimInstance), FVector(Vector.X, Vector.Y, TargetHandZ));

														IParkourABPInterface::Execute_SetRightHandLedgeRotation(Cast<UObject>(CharacterAnimInstance), FRotator(ReversedImpactNormal.Pitch + 270, ReversedImpactNormal.Yaw, ReversedImpactNormal.Roll + 270));
													}
												}
											}
										}
									}

									break;
								}
								else
								{
									if (Index2 == 5)
									{
										if (CharacterAnimInstance)
										{
											if (UClass* AnimClass = CharacterAnimInstance->GetClass())
											{
												if (AnimClass->ImplementsInterface(UParkourABPInterface::StaticClass()))
												{
													if (bIsLeft)
													{
														IParkourABPInterface::Execute_SetLeftHandLedgeLocation(Cast<UObject>(CharacterAnimInstance), FVector(SphereTraceHitOut.ImpactPoint.X, SphereTraceHitOut.ImpactPoint.Y, SphereTraceHitOut.ImpactPoint.Z - 9));

														IParkourABPInterface::Execute_SetLeftHandLedgeRotation(Cast<UObject>(CharacterAnimInstance), FRotator(ReversedImpactNormal.Pitch + 90, ReversedImpactNormal.Yaw, ReversedImpactNormal.Roll + 270));
													}
													else
													{
														IParkourABPInterface::Execute_SetRightHandLedgeLocation(Cast<UObject>(CharacterAnimInstance), FVector(SphereTraceHitOut.ImpactPoint.X, SphereTraceHitOut.ImpactPoint.Y, SphereTraceHitOut.ImpactPoint.Z - 9));

														IParkourABPInterface::Execute_SetRightHandLedgeRotation(Cast<UObject>(CharacterAnimInstance), FRotator(ReversedImpactNormal.Pitch + 270, ReversedImpactNormal.Yaw, ReversedImpactNormal.Roll + 270));
													}
												}
											}
										}
									}
								}
							}
						}
					}

					if (bBreakThisLoop)
					{
						break;
					}
				}
			}
		}


		//Legs Location...
		if (ClimbStyleTag == FGameplayTag::RequestGameplayTag(FName("Parkour.ClimbStyle.Braced")))
		{
			for (int Index = 0; Index <= 2; Index++)
			{
				bool bBreakThisLoop = false;
				if (UWorld* World = GetWorld())
				{
					FVector WallForwardRotation = FRotationMatrix(WallRotation).GetUnitAxis(EAxis::X);
					FVector WallRightRotation = FRotationMatrix(WallRotation).GetUnitAxis(EAxis::Y);
					
					float SideOffset = bIsLeft ? 0 : 2;
					float SideCharHeightDiffOffset = bIsLeft ? 0 : -10;
					FVector SphereTraceStart = FVector(0, 0, (Index * 5)) + LedgeHitResult.ImpactPoint + (WallRightRotation * ((7 + SideOffset) * LimbDir)) + FVector(0, 0, (CharacterHeightDifference * -(150 + SideCharHeightDiffOffset))) + (WallForwardRotation * -30);
					FVector SphereTraceEnd = FVector(0, 0, (Index * 5)) + LedgeHitResult.ImpactPoint + (WallRightRotation * ((7 + SideOffset) * LimbDir)) + FVector(0, 0, (CharacterHeightDifference * -(150 + SideCharHeightDiffOffset))) + (WallForwardRotation * 30);

					FHitResult SphereTraceHitOut;
					FCollisionQueryParams SphereParams = FCollisionQueryParams();
					SphereParams.bTraceComplex = false;
					SphereParams.AddIgnoredActor(PlayerCharacter);
					bool bSphereTraceGotHit = World->SweepSingleByChannel(SphereTraceHitOut, SphereTraceStart, SphereTraceEnd, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(6.0f), SphereParams);

					DrawDebugSphereTraceSingle(World, SphereTraceStart, SphereTraceEnd, 6.0f, DrawDebugType, bSphereTraceGotHit, SphereTraceHitOut, FColor::Blue, FColor::Yellow, 3.0f);
					
					if (bSphereTraceGotHit)
					{
						if (CharacterAnimInstance)
						{
							if (UClass* AnimClass = CharacterAnimInstance->GetClass())
							{
								if (AnimClass->ImplementsInterface(UParkourABPInterface::StaticClass()))
								{
									if (bIsLeft)
									{
										FVector FootLocation = SphereTraceHitOut.ImpactPoint + (FRotationMatrix(UParkourFunctionLibrary::NormalReverseRotationZ(SphereTraceHitOut.ImpactNormal)).GetUnitAxis(EAxis::X) * -17);
										IParkourABPInterface::Execute_SetLeftFootLocation(Cast<UObject>(CharacterAnimInstance), FootLocation);
									}
									else
									{
										FVector FootLocation = SphereTraceHitOut.ImpactPoint + (FRotationMatrix(UParkourFunctionLibrary::NormalReverseRotationZ(SphereTraceHitOut.ImpactNormal)).GetUnitAxis(EAxis::X) * -17);
										IParkourABPInterface::Execute_SetRightFootLocation(Cast<UObject>(CharacterAnimInstance), FootLocation);
									}
								}
							}
						}

						break;
					}
					else
					{
						if (Index == 2)
						{
							for (int Index2 = 0; Index2 <= 4; Index2++)
							{
								FVector SphereTrace2Start = FVector(0, 0, (Index2 * 5)) + LedgeHitResult.ImpactPoint + FVector(0, 0, (CharacterHeightDifference * -(150 + SideCharHeightDiffOffset))) + (WallForwardRotation * -30);
								FVector SphereTrace2End = FVector(0, 0, (Index2 * 5)) + LedgeHitResult.ImpactPoint + FVector(0, 0, (CharacterHeightDifference * -(150 + SideCharHeightDiffOffset))) + (WallForwardRotation * 30);

								FHitResult SphereTrace2HitOut;
								FCollisionQueryParams SphereParams2 = FCollisionQueryParams();
								SphereParams2.bTraceComplex = false;
								SphereParams2.AddIgnoredActor(PlayerCharacter);
								bool bSphereTrace2GotHit = World->SweepSingleByChannel(SphereTrace2HitOut, SphereTrace2Start, SphereTrace2End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(15.0f), SphereParams2);

								DrawDebugSphereTraceSingle(World, SphereTrace2Start, SphereTrace2End, 15.0f, DrawDebugType, bSphereTrace2GotHit, SphereTrace2HitOut, FColor::Blue, FColor::Yellow, 3.0f);

								if (bSphereTrace2GotHit)
								{
									if (CharacterAnimInstance)
									{
										if (UClass* AnimClass = CharacterAnimInstance->GetClass())
										{
											if (AnimClass->ImplementsInterface(UParkourABPInterface::StaticClass()))
											{
												if (bIsLeft)
												{
													FVector FootLocation = SphereTrace2HitOut.ImpactPoint + (FRotationMatrix(UParkourFunctionLibrary::NormalReverseRotationZ(SphereTrace2HitOut.ImpactNormal)).GetUnitAxis(EAxis::X) * -17);
													IParkourABPInterface::Execute_SetLeftFootLocation(Cast<UObject>(CharacterAnimInstance), FootLocation);
												}
												else
												{
													FVector FootLocation = SphereTrace2HitOut.ImpactPoint + (FRotationMatrix(UParkourFunctionLibrary::NormalReverseRotationZ(SphereTrace2HitOut.ImpactNormal)).GetUnitAxis(EAxis::X) * -17);
													IParkourABPInterface::Execute_SetRightFootLocation(Cast<UObject>(CharacterAnimInstance), FootLocation);
												}
											}
										}
									}

									break;
								}
							}
						}
					}
				}
			}
		}
	}
}