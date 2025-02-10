// Copyright Vahatech CC. All Rights Reserved.

#include "BasePlayerCharacter.h"

#include "Engine/LocalPlayer.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

#include "AbilitySystemComponent.h"

ABasePlayerCharacter::ABasePlayerCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true; 

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false;

	// Create an ability system component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
}

void ABasePlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitDefaultAbilities();
}

void ABasePlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeinitDefaultAbilities();

	DeInitInputActionBindings();
	DeinitMappingContext();

	Super::EndPlay(EndPlayReason);
}

void ABasePlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	InitMappingContext();
}

void ABasePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (PlayerInputComponent == nullptr)
	{
		return;
	}

	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InitInputActionBindings(PlayerInputComponent);
}

void ABasePlayerCharacter::InitDefaultAbilities()
{
	if (!HasAuthority())
	{
		return;
	}

	if (DefaultAbilities.IsEmpty())
	{
		return;
	}

	// Give default abilities to player
	for (auto& AbilityToGive : DefaultAbilities)
	{
		if (TSubclassOf<UGameplayAbility> LoadedAbilityToGive = AbilityToGive.LoadSynchronous())
		{
			GivenDefaultAbilities.Add(AbilitySystemComponent->K2_GiveAbility(LoadedAbilityToGive));
		}
	}
}

void ABasePlayerCharacter::DeinitDefaultAbilities() const
{
	if (!HasAuthority())
	{
		return;
	}

	if (DefaultAbilities.IsEmpty())
	{
		return;
	}

	// Give default abilities to player
	for (auto& AbilityToClear : GivenDefaultAbilities)
	{
		AbilitySystemComponent->ClearAbility(AbilityToClear);
	}
}

void ABasePlayerCharacter::InitMappingContext() const
{
	// Add Input Mapping Context
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController == nullptr)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (Subsystem == nullptr)
	{
		return;
	}

	if (const UInputMappingContext* DefaultLoadedMappingContext = DefaultMappingContext.LoadSynchronous())
	{
		Subsystem->AddMappingContext(DefaultLoadedMappingContext, 0);
	}
}

void ABasePlayerCharacter::DeinitMappingContext() const
{
	// Add Input Mapping Context
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController == nullptr)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (Subsystem == nullptr)
	{
		return;
	}

	if (UInputMappingContext* DefaultLoadedMappingContext = DefaultMappingContext.LoadSynchronous())
	{
		Subsystem->RemoveMappingContext(DefaultLoadedMappingContext);
	}
}

void ABasePlayerCharacter::InitInputActionBindings(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
		return;
	}

	if (UInputAction* LoadedJumpAction = JumpAction.LoadSynchronous())
	{
		// Jumping
		JumpActionBindingHandle_Started = EnhancedInputComponent->BindAction(LoadedJumpAction, ETriggerEvent::Started, this, &ThisClass::Jump).GetHandle();
		JumpActionBindingHandle_Completed = EnhancedInputComponent->BindAction(LoadedJumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping).GetHandle();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to load JumpAction! Please check settings and set correct Enhanced Input action into JumpAction."), *GetNameSafe(this));
	}

	if (UInputAction* LoadedMoveAction = MoveAction.LoadSynchronous())
	{
		// Moving
		MoveActionBindingHandle = EnhancedInputComponent->BindAction(LoadedMoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move).GetHandle();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to load JumpAction! Please check settings and set correct Enhanced Input action into JumpAction."), *GetNameSafe(this));
	}

	if (UInputAction* LoadedLookAction = LookAction.LoadSynchronous())
	{
		// Looking
		LookActionBindingHandle = EnhancedInputComponent->BindAction(LoadedLookAction, ETriggerEvent::Triggered, this, &ThisClass::Look).GetHandle();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to load JumpAction! Please check settings and set correct Enhanced Input action into JumpAction."), *GetNameSafe(this));
	}
}

void ABasePlayerCharacter::DeInitInputActionBindings() const
{
	if (InputComponent == nullptr)
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
		return;
	}

	// Jumping
	EnhancedInputComponent->RemoveBindingByHandle(JumpActionBindingHandle_Started);
	EnhancedInputComponent->RemoveBindingByHandle(JumpActionBindingHandle_Completed);

	// Moving
	EnhancedInputComponent->RemoveBindingByHandle(MoveActionBindingHandle);

	// Looking
	EnhancedInputComponent->RemoveBindingByHandle(LookActionBindingHandle);
}

void ABasePlayerCharacter::Move(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}

	// input is a Vector2D
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// add movement 
	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ABasePlayerCharacter::Look(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}

	// input is a Vector2D
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	// add yaw and pitch input to controller
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}
