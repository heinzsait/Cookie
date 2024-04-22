// Copyright Epic Games, Inc. All Rights Reserved.

#include "CookieCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/CookieCharacterMovementComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ACookieCharacter

ACookieCharacter::ACookieCharacter(const FObjectInitializer& ObjectInitializer)
	: ACharacter(ObjectInitializer.SetDefaultSubobjectClass<UCookieCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 5000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->GravityScale = 1.75f;
	GetCharacterMovement()->MaxAcceleration = 1500;
	GetCharacterMovement()->bUseSeparateBrakingFriction = true;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 700.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

void ACookieCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	AnimInstance = GetMesh()->GetAnimInstance();
	
	CookieCharacterMovement = Cast<UCookieCharacterMovementComponent>(GetCharacterMovement());
}

void ACookieCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ACookieCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACookieCharacter::JumpPressed);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACookieCharacter::JumpReleased);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACookieCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ACookieCharacter::MoveStopped);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACookieCharacter::Look);
		
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ACookieCharacter::Dash);
		
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ACookieCharacter::SprintPressed);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACookieCharacter::SprintReleased);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

UCookieCharacterMovementComponent* ACookieCharacter::GetCookieCharacterMovement() const
{
	return CookieCharacterMovement;
}

void ACookieCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	MovementVector = Value.Get<FVector2D>();

	if (Controller && CookieCharacterMovement && CookieCharacterMovement->CanMove())
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACookieCharacter::MoveStopped()
{
	MovementVector = FVector2D::ZeroVector;
}

float ACookieCharacter::GetMoveHorizontalAxis()
{
	return MovementVector.X;
}

void ACookieCharacter::SprintPressed()
{
	if (CookieCharacterMovement)
	{
		CookieCharacterMovement->EnableSprint();
	}
}

void ACookieCharacter::SprintReleased()
{
	if (CookieCharacterMovement)
	{
		CookieCharacterMovement->DisableSprint();
	}
}

void ACookieCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACookieCharacter::JumpPressed()
{
	if (CookieCharacterMovement)
	{
		CookieCharacterMovement->ProcessJump();
	}
}

void ACookieCharacter::JumpReleased()
{
	if (CookieCharacterMovement->Velocity.Z > 0)
	{
		CookieCharacterMovement->Velocity = FVector(CookieCharacterMovement->Velocity.X, CookieCharacterMovement->Velocity.Y, 0);
	}
}

void ACookieCharacter::Dash()
{
	if (CookieCharacterMovement)
	{
		CookieCharacterMovement->Dash();
	}
}

void ACookieCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (CookieCharacterMovement)
	{
		CookieCharacterMovement->OnLanded();
	}
}