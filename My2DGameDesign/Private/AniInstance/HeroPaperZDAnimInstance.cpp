// 文件路径: My2DGameDesign/Private/AniInstance/HeroPaperZDAnimInstance.cpp
#include "AniInstance/HeroPaperZDAnimInstance.h"
#include "Actors/PaperZDCharacter_SpriteHero.h" // 需要包含 Actor 头文件
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "My2DGameDesign/My2DGameDesign.h" // 包含项目主头文件以访问 AnimationJumpNodeName

// --- 修改 OnInit_Implementation ---
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
	// bIsHurt = false; // <--- 移除这一行
	bIsDead = false;
	ComboCount = 0;
	Velocity = FVector::ZeroVector;
	GroundSpeed = 0.0f;
	VerticalSpeed = 0.0f;
}

// --- 修改 OnTakeHit_Implementation ---
void UHeroPaperZDAnimInstance::OnTakeHit_Implementation(float DamageAmount, const FVector& HitDirection, bool bInterruptsCurrentAction)
{
	// 如果需要中断且角色未死亡，则直接跳转到受击状态
	// 动画状态机本身会处理是否已经在 Hurt 状态（通常会阻止重复进入）
	if (bInterruptsCurrentAction && !bIsDead)
	{
		UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: OnTakeHit - Jumping to HeroHurt state."));
		JumpToNode(AnimationJumpNodeName::HeroHurt); // 直接跳转
		// 移除之前关于 bIsHurt 的检查和设置逻辑
	}
	else if (bIsDead)
	{
		UE_LOG(LogTemp, Verbose, TEXT("HeroAnimInstance: OnTakeHit ignored (Dead)."));
	}
	else if (!bInterruptsCurrentAction)
	{
		UE_LOG(LogTemp, Verbose, TEXT("HeroAnimInstance: OnTakeHit ignored (Interrupt not requested)."));
	}
}

// --- 修改 ExitHurtAnimStateEvent ---
void UHeroPaperZDAnimInstance::ExitHurtAnimStateEvent()
{


	// 直接尝试获取 Owning Actor 并调用其恢复函数
	AActor* OwningActor = GetOwningActor();
	if (APaperZDCharacter_SpriteHero* OwnerHero = Cast<APaperZDCharacter_SpriteHero>(OwningActor))
	{
		UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: Notifying character %s to recover from incapacitated state via ExitHurtAnimStateEvent."), *OwnerHero->GetName());
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
}



void UHeroPaperZDAnimInstance::OnIntentStateChanged_Implementation(bool bNewIsWalking, bool bNewIsRunning)
{
	// 只检查是否死亡。如果处于 Hurt 状态，动画状态机应阻止转换到 Walk/Run。
	if (bIsDead) return;
	this->bIsWalking = bNewIsWalking;
	this->bIsRunning = bNewIsRunning;
	UE_LOG(LogTemp, Verbose, TEXT("HeroAnimInstance: Intent state changed (Walk: %d, Run: %d)."), bNewIsWalking, bNewIsRunning);
}

void UHeroPaperZDAnimInstance::OnDashStateChanged_Implementation(bool bNewIsDashing)
{
    // 只检查是否死亡。
	if (bIsDead) return;
	this->bIsDashing = bNewIsDashing;
	if (bIsDashing)
	{
		UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: Dash state changed (Dashing: true) - Jumping to Dash node."));
		JumpToNode(AnimationJumpNodeName::Dash);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: Dash state changed (Dashing: false)."));
	}
}

void UHeroPaperZDAnimInstance::OnCombatStateChanged_Implementation(int32 NewComboCount)
{
    // 只检查是否死亡。
	if (bIsDead) return;
	this->ComboCount = NewComboCount;
	UE_LOG(LogTemp, Verbose, TEXT("HeroAnimInstance: Combat state changed (ComboCount: %d)."), NewComboCount);
}

void UHeroPaperZDAnimInstance::OnJumpRequested_Implementation()
{
    // 只检查是否死亡。
	if (bIsDead) return;
	UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: Jump requested - Jumping to Jump node."));
	JumpToNode(AnimationJumpNodeName::Jump);
}

void UHeroPaperZDAnimInstance::OnAirAttackStateChanged_Implementation(bool bNewIsAirAttacking)
{
    // 只检查是否死亡。
	if (bIsDead) return;
	this->bIsAirAttacking = bNewIsAirAttacking;
	UE_LOG(LogTemp, Verbose, TEXT("HeroAnimInstance: Air attack state changed (IsAirAttacking: %d)."), bNewIsAirAttacking);
}

void UHeroPaperZDAnimInstance::OnFallingRequested_Implementation()
{
	JumpToNode(AnimationJumpNodeName::HeroFalling);
}

// --- OnDeathState_Implementation ---
void UHeroPaperZDAnimInstance::OnDeathState_Implementation(AActor* Killer)
{
	UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: OnDeathState called. Setting bIsDead=true."));
	if(bIsDead) return; 

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
	UE_LOG(LogTemp, Log, TEXT("HeroAnimInstance: Jumped to HeroDeath node."));
}


void UHeroPaperZDAnimInstance::OnTick_Implementation(float DeltaTime)
{
    Super::OnTick_Implementation(DeltaTime);
	if (MovementComponentPtr.IsValid())
	{
		UCharacterMovementComponent* MoveComp = MovementComponentPtr.Get();
		// 如果角色死了，运动状态应固定或由死亡动画控制
		if (!bIsDead)
		{
		    bIsMovingOnGround = MoveComp->IsMovingOnGround();
		    bIsFalling = MoveComp->IsFalling();
		    Velocity = MoveComp->Velocity;
			// GroundSpeed = FVector::DotProduct(Velocity, MoveComp->GetOwner()->GetActorForwardVector()); // 这个计算可能不准确，如果角色可以侧向移动
            GroundSpeed = Velocity.Size2D(); // 使用 2D 速度大小更通用
			VerticalSpeed = Velocity.Z;
		}
		else // 如果死亡
		{
            // 保持这些状态为静止/非活动状态，除非死亡动画需要它们变化
            bIsMovingOnGround = false;
            bIsFalling = false;
            Velocity = FVector::ZeroVector;
			GroundSpeed = 0.0f;
			VerticalSpeed = 0.0f;
		}
	}
	else // 获取移动组件失败的逻辑
	{
		AActor* OwningActor = GetOwningActor();
		if (OwningActor) { MovementComponentPtr = OwningActor->FindComponentByClass<UCharacterMovementComponent>(); }
		if (!MovementComponentPtr.IsValid()) // 如果再次获取失败
		{
            // 设置为默认静止/非活动状态
			bIsMovingOnGround = false;
			bIsFalling = true; // 或 false，取决于你认为获取失败时应该是什么状态
			Velocity = FVector::ZeroVector;
			GroundSpeed = 0.0f;
			VerticalSpeed = 0.0f;
            if(OwningActor) UE_LOG(LogTemp, Warning, TEXT("HeroAnimInstance::Tick - Could not get MovementComponent for %s."), *OwningActor->GetName());
		}
	}
}