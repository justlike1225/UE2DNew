// 文件: Private/States/HeroStateBase.cpp
#include "States/HeroStateBase.h"
// #include "Actors/PaperZDCharacter_SpriteHero.h" // <--- 不再需要直接包含具体角色类
#include "Interfaces/Context/HeroStateContext.h"   // <--- 包含接口
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "Components/HealthComponent.h" // <--- 包含 HealthComponent
#include "States/HurtState.h"
#include "States/DeadState.h"
#include "Engine/Engine.h" // For GEngine debug messages if needed
#include "Interfaces/ActionInterruptSource.h"

// 修改 InitState 函数签名和实现
void UHeroStateBase::InitState(TScriptInterface<IHeroStateContext> InHeroContext)
{
	if (!InHeroContext)
	{
		UE_LOG(LogTemp, Error, TEXT("HeroStateBase::InitState - Received null context!"));
		return;
	}
	HeroContext = InHeroContext;

	// 通过接口获取组件
	MovementComponent = HeroContext->Execute_GetMovementComponent(HeroContext.GetObject());
	AnimListener = HeroContext->Execute_GetAnimStateListener(HeroContext.GetObject());
	HealthComponent = HeroContext->Execute_GetHealthComponent(HeroContext.GetObject()); // <--- 获取生命组件
}


void UHeroStateBase::OnEnterState_Implementation()
{
}

void UHeroStateBase::OnExitState_Implementation()
{
}

void UHeroStateBase::TickState_Implementation(float DeltaTime)
{
}

void UHeroStateBase::HandleMoveInput_Implementation(const FInputActionValue& Value)
{
}

void UHeroStateBase::HandleJumpInputPressed_Implementation()
{
}

void UHeroStateBase::HandleJumpInputReleased_Implementation()
{
}

void UHeroStateBase::HandleAttackInput_Implementation()
{
}

void UHeroStateBase::HandleDashInput_Implementation()
{
}

void UHeroStateBase::HandleRunInputPressed_Implementation()
{
}

void UHeroStateBase::HandleRunInputReleased_Implementation()
{
}

void UHeroStateBase::HandleLanded_Implementation(const FHitResult& Hit)
{
}

void UHeroStateBase::HandleWalkingOffLedge_Implementation()
{
}

void UHeroStateBase::HandleHurtRecovery_Implementation()
{
}

void UHeroStateBase::HandleAttackEnd_Implementation()
{
}

void UHeroStateBase::HandleDashEnd_Implementation()
{
}

// 修改 TakeDamage 实现
void UHeroStateBase::HandleTakeDamage_Implementation()
{
	// 检查是否死亡 (通过接口获取 HealthComponent)
	if (HealthComponent.IsValid() && !HealthComponent->IsDead())
	{
		// 通过接口请求状态切换到 Hurt
		TrySetState(UHurtState::StaticClass());
	}
	// 如果已经死亡，则不会进入 Hurt 状态 (死亡检查由 HandleDeath 处理)
}

// 修改 Death 实现
void UHeroStateBase::HandleDeath_Implementation()
{
	// 通过接口请求状态切换到 Dead
	TrySetState(UDeadState::StaticClass());
}

// 修改 TrySetState 实现
void UHeroStateBase::TrySetState(TSubclassOf<UHeroStateBase> NewStateClass)
{
	if (HeroContext && NewStateClass)
	{
		// 通过接口调用 RequestStateChange
		HeroContext->Execute_RequestStateChange(HeroContext.GetObject(), NewStateClass);
	}
}
