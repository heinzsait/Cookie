// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ParkourMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values for this component's properties
UParkourMovementComponent::UParkourMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
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

	// ...
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

		if (UWorld* World = GetWorld())
		{
			ArrowActor = World->SpawnActor(ArrowActorClass);
			ArrowActor->SetActorTransform(Character->GetActorTransform());
			ArrowActor->SetOwner(Character);
			FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::KeepRelative, true);
			ArrowActor->AttachToComponent(CharacterMesh, AttachmentTransformRules);
			ArrowActor->SetActorRelativeLocation(FVector(ArrowLocationX, 0, (ArrowLocationZ - CharacterHeightDifference)));

			if (CharacterCameraBoom)
			{
				DefaultCameraBoomTargetArmLength = CharacterCameraBoom->TargetArmLength;
				DefaultCameraBoomRelativeLocation = CharacterCameraBoom->GetRelativeLocation();
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

void UParkourMovementComponent::ParkourAction(bool bAutoClimb)
{

}

