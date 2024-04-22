// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/ParkourInterface.h"
#include "ParkourMovementComponent.generated.h"

class UCharacterMovementComponent;
class UCapsuleComponent;

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
};
