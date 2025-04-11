#pragma once
#include "CoreMinimal.h"
#include "PaperZDAnimInstance.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "HeroPaperZDAnimInstance.generated.h"

class UCharacterMovementComponent;

UCLASS()
class MY2DGAMEDESIGN_API UHeroPaperZDAnimInstance
	: public UPaperZDAnimInstance, public ICharacterAnimationStateListener
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsMovingOnGround = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsFalling = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsWalking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	bool bIsRunning = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	float GroundSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Movement",
		meta = (AllowPrivateAccess = "true"))
	float VerticalSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Dash",
		meta = (AllowPrivateAccess = "true"))
	bool bIsDashing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Combat",
		meta = (AllowPrivateAccess = "true"))
	int32 ComboCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character State|Combat",
		meta = (AllowPrivateAccess = "true"))
	bool bIsAirAttacking = false;

	UPROPERTY()
	TWeakObjectPtr<UCharacterMovementComponent> MovementComponentPtr;

	virtual void OnInit_Implementation() override;
	virtual void OnTick_Implementation(float DeltaTime) override;

	virtual void OnIntentStateChanged_Implementation(bool bNewIsWalking, bool bNewIsRunning) override;
	virtual void OnDashStateChanged_Implementation(bool bNewIsDashing) override;
	virtual void OnCombatStateChanged_Implementation(int32 NewComboCount) override;
	virtual void OnJumpRequested_Implementation() override;
	virtual void OnAirAttackStateChanged_Implementation(bool bNewIsAirAttacking) override;
};
