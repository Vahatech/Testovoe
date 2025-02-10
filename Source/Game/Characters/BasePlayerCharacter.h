// Copyright Vahatech CC. All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"

#include "EnhancedInputComponent.h"
#include "GameplayAbilitySpecHandle.h"

#include "BasePlayerCharacter.generated.h"

class UGameplayAbility;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAbilitySystemComponent;
struct FInputActionValue;

/**
 * Base class for player characters that:
 *	Adds gameplay ability system and startup default abilities
 *	Adds input bindings (with using enhanced input)
 *	Implements move and look features to character with using input
 */
UCLASS(config = Game)
class ABasePlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	ABasePlayerCharacter();

	/** Returns CameraBoom subobject **/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected:

	// Begin ACharacter override
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	//~End ACharacter override

	/** Adds default abilities to player */
	void InitDefaultAbilities();

	/** Removes given default abilities */
	void DeinitDefaultAbilities() const;

	/** Adds mapping context to player input */
	void InitMappingContext() const;

	/** Removes mapping context from player input */
	void DeinitMappingContext() const;

	/** Add input action bindings to player input component */
	void InitInputActionBindings(UInputComponent* PlayerInputComponent);

	/** Remove input action bindings from Player Input component that was bound by this class */
	void DeInitInputActionBindings() const;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

protected:

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom = nullptr;
    
	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera = nullptr;

	/** Component to add/remove/activate abilities, add/remove effects, etc. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UAbilitySystemComponent* AbilitySystemComponent = nullptr;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UInputMappingContext> DefaultMappingContext = nullptr;
  
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UInputAction> JumpAction = nullptr;
    
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UInputAction> MoveAction = nullptr;
    
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UInputAction> LookAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GAS, meta = (AllowPrivateAccess = "true"))
	TArray<TSoftClassPtr<UGameplayAbility>> DefaultAbilities;

	/** Default abilities that was given to player */
	TArray<FGameplayAbilitySpecHandle> GivenDefaultAbilities;

	/** Input action handles that was bind to player input component */
	int32 JumpActionBindingHandle_Started = -1;
	int32 JumpActionBindingHandle_Completed = -1;

	int32 MoveActionBindingHandle = -1;
	int32 LookActionBindingHandle = -1;
};
