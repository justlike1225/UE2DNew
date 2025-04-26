#include "AniInstance/HeroPaperZDAnimInstance.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "My2DGameDesign/My2DGameDesign.h"

void UHeroPaperZDAnimInstance::OnRageDashStarted_Implementation()
{
	if (bIsDead) return;

	this->bIsRageDashing = true;

	this->bIsWalking = false;
	this->bIsRunning = false;
	this->ComboCount = 0;
	this->bIsAirAttacking = false;
}
void UHeroPaperZDAnimInstance::ExitRageDashAnimStateEvent()
{
	if (this->bIsRageDashing) // 检查是否需要重置
	{
		this->bIsRageDashing = false;
	}
}

void UHeroPaperZDAnimInstance::OnUpwardSweepStarted_Implementation()
{
	if (bIsDead) return; 

	this->bIsPerformingUpwardSweep = true;
	
	this->bIsWalking = false;
	this->bIsRunning = false;
	this->ComboCount = 0;
	this->bIsAirAttacking = false;
	this->bIsDashing = false;
	this->bIsRageDashing = false;

}

void UHeroPaperZDAnimInstance::ExitUpwardSweepAnimStateEvent()
{
	if (this->bIsPerformingUpwardSweep) 
	{
		this->bIsPerformingUpwardSweep = false;
		
	}
}
void UHeroPaperZDAnimInstance::OnInit_Implementation()
{
	Super::OnInit_Implementation();
	AActor* OwningActor = GetOwningActor();
	if (OwningActor) { MovementComponentPtr = OwningActor->FindComponentByClass<UCharacterMovementComponent>(); }


	bIsMovingOnGround = false;
	bIsFalling = true;
	bIsWalking = false;
	bIsRunning = false;
	bIsAirAttacking = false;
	bIsDashing = false;
	bIsRageDashing = false; 
	bIsDead = false;
	ComboCount = 0;
	Velocity = FVector::ZeroVector;
	GroundSpeed = 0.0f;
	VerticalSpeed = 0.0f;
}


void UHeroPaperZDAnimInstance::OnTakeHit_Implementation(float DamageAmount, const FVector& HitDirection,
                                                        bool bInterruptsCurrentAction)
{
	if (bInterruptsCurrentAction && !bIsDead)
	{
		UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: OnTakeHit - Jumping to HeroHurt state."));
		JumpToNode(AnimationJumpNodeName::HeroHurt);
	}
}


void UHeroPaperZDAnimInstance::ExitHurtAnimStateEvent()
{
	AActor* OwningActor = GetOwningActor();
	if (APaperZDCharacter_SpriteHero* OwnerHero = Cast<APaperZDCharacter_SpriteHero>(OwningActor))
	{
		OwnerHero->NotifyHurtRecovery();
	}
}


void UHeroPaperZDAnimInstance::OnIntentStateChanged_Implementation(bool bNewIsWalking, bool bNewIsRunning)
{
	if (bIsDead) return;
	this->bIsWalking = bNewIsWalking;
	this->bIsRunning = bNewIsRunning;
}

void UHeroPaperZDAnimInstance::OnDashStateChanged_Implementation(bool bNewIsDashing)
{
	if (bIsDead) return;
	this->bIsDashing = bNewIsDashing;
	if (bIsDashing)
	{
		JumpToNode(AnimationJumpNodeName::Dash);
	}
}

void UHeroPaperZDAnimInstance::OnCombatStateChanged_Implementation(int32 NewComboCount)
{
	if (bIsDead) return;
	this->ComboCount = NewComboCount;
}

void UHeroPaperZDAnimInstance::OnJumpRequested_Implementation()
{
	if (bIsDead) return;
	JumpToNode(AnimationJumpNodeName::Jump);
}

void UHeroPaperZDAnimInstance::OnAirAttackStateChanged_Implementation(bool bNewIsAirAttacking)
{
	if (bIsDead) return;
	this->bIsAirAttacking = bNewIsAirAttacking;
}

void UHeroPaperZDAnimInstance::OnFallingRequested_Implementation()
{
}


void UHeroPaperZDAnimInstance::OnDeathState_Implementation(AActor* Killer)
{
	if (bIsDead) return;

	this->bIsDead = true;


	this->bIsMovingOnGround = false;
	this->bIsFalling = false;
	this->bIsWalking = false;
	this->bIsRunning = false;
	this->bIsDashing = false;
	this->bIsAirAttacking = false;
	this->ComboCount = 0;
	this->GroundSpeed = 0.0f;
	this->VerticalSpeed = 0.0f;

	JumpToNode(AnimationJumpNodeName::HeroDeath);
}


void UHeroPaperZDAnimInstance::OnTick_Implementation(float DeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);
	if (MovementComponentPtr.IsValid())
	{
		UCharacterMovementComponent* MoveComp = MovementComponentPtr.Get();

		if (!bIsDead)
		{
			bIsMovingOnGround = MoveComp->IsMovingOnGround();
			bIsFalling = MoveComp->IsFalling();
			Velocity = MoveComp->Velocity;

			GroundSpeed = Velocity.Size2D();
			VerticalSpeed = Velocity.Z;
		}
		else
		{
			bIsMovingOnGround = false;
			bIsFalling = false;
			Velocity = FVector::ZeroVector;
			GroundSpeed = 0.0f;
			VerticalSpeed = 0.0f;
		}
	}
	else
	{
		if (AActor* OwningActor = GetOwningActor())
		{
			MovementComponentPtr = OwningActor->FindComponentByClass<UCharacterMovementComponent>();
		}
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
