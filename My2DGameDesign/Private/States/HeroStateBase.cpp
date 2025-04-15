// 文件: Private/States/HeroStateBase.cpp
#include "States/HeroStateBase.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "Interfaces/AnimationListenerProvider/HeroAnimationStateProvider.h"
#include "Components/HealthComponent.h"
#include "States/HurtState.h"
#include "States/DeadState.h"

void UHeroStateBase::InitState(APaperZDCharacter_SpriteHero* InHeroContext)
{
	if (!InHeroContext)
	{
		return;
	}
	HeroContext = InHeroContext;

	MovementComponent = HeroContext->GetCharacterMovement();
	if (!MovementComponent.IsValid())
	{
		// Handle missing movement component if necessary
	}

	IHeroAnimationStateProvider* AnimProvider = Cast<IHeroAnimationStateProvider>(HeroContext);
	if (AnimProvider)
	{
		AnimListener = AnimProvider->GetAnimStateListener_Implementation();
		if (!AnimListener)
		{
            // Handle missing anim listener if necessary
		}
	}
	else
	{
        // Handle missing provider if necessary
	}
}

void UHeroStateBase::OnEnterState_Implementation() {}
void UHeroStateBase::OnExitState_Implementation() {}
void UHeroStateBase::TickState_Implementation(float DeltaTime) {}
void UHeroStateBase::HandleMoveInput_Implementation(const FInputActionValue& Value) {}
void UHeroStateBase::HandleJumpInputPressed_Implementation() {}
void UHeroStateBase::HandleJumpInputReleased_Implementation() {}
void UHeroStateBase::HandleAttackInput_Implementation() {}
void UHeroStateBase::HandleDashInput_Implementation() {}
void UHeroStateBase::HandleRunInputPressed_Implementation() {}
void UHeroStateBase::HandleRunInputReleased_Implementation() {}
void UHeroStateBase::HandleLanded_Implementation(const FHitResult& Hit)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Landed in HeroStateBase"));
}
void UHeroStateBase::HandleWalkingOffLedge_Implementation() {}
void UHeroStateBase::HandleHurtRecovery_Implementation() {}
void UHeroStateBase::HandleAttackEnd_Implementation() {}
void UHeroStateBase::HandleDashEnd_Implementation() {}

void UHeroStateBase::HandleTakeDamage_Implementation()
{
	if (HeroContext && HeroContext->GetHealthComponent() && !HeroContext->GetHealthComponent()->IsDead())
	{
		TrySetState(UHurtState::StaticClass());
	}
}

void UHeroStateBase::HandleDeath_Implementation()
{
	TrySetState(UDeadState::StaticClass());
}

void UHeroStateBase::TrySetState(TSubclassOf<UHeroStateBase> NewStateClass)
{
	if (HeroContext && NewStateClass)
	{
		HeroContext->ChangeState(NewStateClass);
	}
}