// 文件路径: My2DGameDesign/Private/AniInstance/HeroPaperZDAnimInstance.cpp
#include "AniInstance/HeroPaperZDAnimInstance.h"

#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "My2DGameDesign/My2DGameDesign.h" // 包含项目主头文件以访问 AnimationJumpNodeName

void UHeroPaperZDAnimInstance::ExitHurtAnimStateEvent()
{
	// 1. 重置动画实例内部的 bIsHurt 标志，允许状态机转换
	if (this->bIsHurt)
	{
		this->bIsHurt = false;
        UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: Resetting bIsHurt flag to false."));
	}

	// 2. 获取 Owning Actor 并尝试调用其恢复函数
	AActor* OwningActor = GetOwningActor();
	if (APaperZDCharacter_SpriteHero* OwnerHero = Cast<APaperZDCharacter_SpriteHero>(OwningActor))
	{
        UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: Notifying character %s to recover from incapacitated state."), *OwnerHero->GetName());
		OwnerHero->NotifyHurtRecovery(); // <--- 通知角色恢复行动能力
	}
    else if (OwningActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("HeroAnimInstance: Owning Actor %s is not APaperZDCharacter_SpriteHero, cannot notify hurt recovery."), *OwningActor->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HeroAnimInstance: Could not get Owning Actor in ExitHurtAnimStateEvent."));
    }

	// GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, TEXT("Hurt State Exited in AnimInstance")); // 可以修改或保留日志
}

void UHeroPaperZDAnimInstance::OnTakeHit_Implementation(float DamageAmount, const FVector& HitDirection, bool bInterruptsCurrentAction)
{
	if (bInterruptsCurrentAction && !bIsDead)
	{
		// 简化：主要负责设置 bIsHurt 和跳转动画状态
		if (!this->bIsHurt) // 防止在 Hurt 状态内重复触发跳转
		{
			UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: OnTakeHit - Setting bIsHurt=true and jumping to HeroHurt state."));
			this->bIsHurt = true;
			JumpToNode(AnimationJumpNodeName::HeroHurt);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: OnTakeHit - Hit while already hurt. bIsHurt remains true."));
		}
	
	}
}
void UHeroPaperZDAnimInstance::OnInit_Implementation()
{
	Super::OnInit_Implementation();
	AActor* OwningActor = GetOwningActor();
	if (OwningActor) { MovementComponentPtr = OwningActor->FindComponentByClass<UCharacterMovementComponent>(); }
	// 初始化状态
	bIsMovingOnGround = false;
	bIsFalling = true;
	bIsWalking = false;
	bIsRunning = false;
	bIsAirAttacking = false;
	bIsDashing = false;
	bIsHurt = false; // <--- 初始化新增状态
	bIsDead = false; // <--- 初始化新增状态
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
		// --- 修改：如果角色死了，地面和坠落状态可能需要根据动画固定 ---
		if (!bIsDead)
		{
		    bIsMovingOnGround = MoveComp->IsMovingOnGround();
		    bIsFalling = MoveComp->IsFalling();
		    Velocity = MoveComp->Velocity;
			GroundSpeed = FVector::DotProduct(Velocity, MoveComp->GetOwner()->GetActorForwardVector()); // 或者 Velocity.Size2D()
			VerticalSpeed = Velocity.Z;
		}
		else // 如果死亡
		{
            bIsMovingOnGround = false; // 死亡时通常不在地面移动
            bIsFalling = false; // 死亡时通常不处理坠落逻辑
            Velocity = FVector::ZeroVector;
			GroundSpeed = 0.0f;
			VerticalSpeed = 0.0f;
		}
		// --- 修改结束 ---
	}
	else // 获取移动组件失败的逻辑
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
	// 在死亡或受击状态下，不响应移动意图
	if (bIsDead || bIsHurt) return;
	this->bIsWalking = bNewIsWalking;
	this->bIsRunning = bNewIsRunning;
}

void UHeroPaperZDAnimInstance::OnDashStateChanged_Implementation(bool bNewIsDashing)
{
    // 在死亡或受击状态下，不响应冲刺
	if (bIsDead || bIsHurt) return;
	this->bIsDashing = bNewIsDashing;
	if (bIsDashing)
	{
		JumpToNode(AnimationJumpNodeName::Dash);
	}
}

void UHeroPaperZDAnimInstance::OnCombatStateChanged_Implementation(int32 NewComboCount)
{
    // 在死亡或受击状态下，不更新连击数
	if (bIsDead || bIsHurt) return;
	this->ComboCount = NewComboCount;
}

void UHeroPaperZDAnimInstance::OnJumpRequested_Implementation()
{
    // 在死亡或受击状态下，不响应跳跃
	if (bIsDead || bIsHurt) return;
	JumpToNode(AnimationJumpNodeName::Jump);
}

void UHeroPaperZDAnimInstance::OnAirAttackStateChanged_Implementation(bool bNewIsAirAttacking)
{
    // 在死亡或受击状态下，可能不响应空袭
	if (bIsDead || bIsHurt) return;
	this->bIsAirAttacking = bNewIsAirAttacking;
}





/** 处理死亡事件 */
void UHeroPaperZDAnimInstance::OnDeathState_Implementation(AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: OnDeathState called. Setting bIsDead=true."));
	if(bIsDead) return; // 防止重复处理

	this->bIsDead = true; // 标记为死亡状态

	// 重置所有其他可能冲突的状态
	this->bIsHurt = false;
	this->bIsMovingOnGround = false;
	this->bIsFalling = false; // 死亡时通常不认为在坠落
	this->bIsWalking = false;
	this->bIsRunning = false;
	this->bIsDashing = false;
	this->bIsAirAttacking = false;
	this->ComboCount = 0;
	this->GroundSpeed = 0.0f;
	this->VerticalSpeed = 0.0f;

 JumpToNode(AnimationJumpNodeName::HeroDeath); 
  
}


