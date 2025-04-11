#include "AniInstance/HeroPaperZDAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "My2DGameDesign/My2DGameDesign.h"

void UHeroPaperZDAnimInstance::OnInit_Implementation()
{
	Super::OnInit_Implementation();


	AActor* OwningActor = GetOwningActor();
	if (OwningActor)
	{
		MovementComponentPtr = OwningActor->FindComponentByClass<UCharacterMovementComponent>();
		if (!MovementComponentPtr.IsValid())
		{
		}
	}


	bIsMovingOnGround = false;
	bIsFalling = true;
	bIsWalking = false;
	bIsRunning = false;
	bIsAirAttacking = false;
	bIsDashing = false;
	ComboCount = 0;
	Velocity = FVector::ZeroVector;
	GroundSpeed = 0.0f;
	VerticalSpeed = 0.0f;
}

void UHeroPaperZDAnimInstance::OnTick_Implementation(float DeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);


	if (MovementComponentPtr.IsValid())
	{
		UCharacterMovementComponent* MoveComp = MovementComponentPtr.Get();
		bIsMovingOnGround = MoveComp->IsMovingOnGround();
		bIsFalling = MoveComp->IsFalling();
		Velocity = MoveComp->Velocity;
		GroundSpeed = FVector::DotProduct(Velocity, MoveComp->GetOwner()->GetActorForwardVector());

		VerticalSpeed = Velocity.Z;
	}
	else
	{
		AActor* OwningActor = GetOwningActor();
		if (OwningActor) { MovementComponentPtr = OwningActor->FindComponentByClass<UCharacterMovementComponent>(); }


		if (!MovementComponentPtr.IsValid())
		{
			bIsMovingOnGround = false;
			bIsFalling = true;
			Velocity = FVector::ZeroVector;
			GroundSpeed = 0.0f;
			VerticalSpeed = 0.0f;
		}
	}
}


void UHeroPaperZDAnimInstance::OnIntentStateChanged_Implementation(bool bNewIsWalking, bool bNewIsRunning)
{
	this->bIsWalking = bNewIsWalking;
	this->bIsRunning = bNewIsRunning;
}


void UHeroPaperZDAnimInstance::OnDashStateChanged_Implementation(bool bNewIsDashing)
{
	this->bIsDashing = bNewIsDashing;
	if (bIsDashing)
	{
		JumpToNode(AnimationJumpNodeName::Dash);
	}
}


void UHeroPaperZDAnimInstance::OnCombatStateChanged_Implementation(int32 NewComboCount)
{
	this->ComboCount = NewComboCount;
}


void UHeroPaperZDAnimInstance::OnJumpRequested_Implementation()
{
	JumpToNode(AnimationJumpNodeName::Jump);
}


void UHeroPaperZDAnimInstance::OnAirAttackStateChanged_Implementation(bool bNewIsAirAttacking)
{
	this->bIsAirAttacking = bNewIsAirAttacking;
}
