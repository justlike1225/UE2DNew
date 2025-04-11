// My2DGameDesign/Private/AniInstance/HeroPaperZDAnimInstance.cpp
#include "AniInstance/HeroPaperZDAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h" // 需要移动组件
#include "GameFramework/Actor.h" // 需要 GetOwningActor
#include "Engine/Engine.h" // 用于 GEngine 屏幕打印 (可选)
#include "My2DGameDesign/My2DGameDesign.h"

// OnInit: 获取移动组件引用
void UHeroPaperZDAnimInstance::OnInit_Implementation()
{
	Super::OnInit_Implementation(); // 调用父类实现

	// 尝试获取 Owning Actor
	AActor* OwningActor = GetOwningActor();
	if (OwningActor)
	{
		// 查找并缓存移动组件的弱指针
		MovementComponentPtr = OwningActor->FindComponentByClass<UCharacterMovementComponent>();
		if (!MovementComponentPtr.IsValid())
		{
			UE_LOG(LogTemp, Warning,
			       TEXT(
				       "HeroPaperZDAnimInstance::OnInit - Could not find CharacterMovementComponent on Owning Actor '%s'."
			       ), *OwningActor->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("HeroPaperZDAnimInstance::OnInit - Owning Actor is null!"));
	}

	// 初始化状态变量 (可选，接口更新会覆盖)
	bIsMovingOnGround = false;
	bIsFalling = true; // 假设初始在空中
	bIsWalking = false;
	bIsRunning = false;
	bIsAirAttacking = false; // 初始化
	bIsDashing = false;
	ComboCount = 0;
	Velocity = FVector::ZeroVector;
	GroundSpeed = 0.0f;
	VerticalSpeed = 0.0f;
}

// OnTick: 只更新高频物理状态
void UHeroPaperZDAnimInstance::OnTick_Implementation(float DeltaTime)
{
	Super::OnTick_Implementation(DeltaTime); // 调用父类实现

	// --- 更新需要每帧检查的物理状态 ---
	if (MovementComponentPtr.IsValid())
	{
		UCharacterMovementComponent* MoveComp = MovementComponentPtr.Get(); // 获取强指针
		bIsMovingOnGround = MoveComp->IsMovingOnGround();
		bIsFalling = MoveComp->IsFalling();
		Velocity = MoveComp->Velocity;
		GroundSpeed = FVector::DotProduct(Velocity, MoveComp->GetOwner()->GetActorForwardVector()); // 计算前向速度，考虑方向
		// 或者简单地用 XY 平面速度 FVector(Velocity.X, Velocity.Y, 0).Size();
		VerticalSpeed = Velocity.Z;
	}
	else // 如果移动组件无效，尝试重新获取一次
	{
		AActor* OwningActor = GetOwningActor();
		if (OwningActor) { MovementComponentPtr = OwningActor->FindComponentByClass<UCharacterMovementComponent>(); }

		// 如果仍然无效，重置物理状态
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

// --- 接口函数的 C++ 实现 ---

// 更新行走/奔跑状态
void UHeroPaperZDAnimInstance::OnIntentStateChanged_Implementation(bool bNewIsWalking, bool bNewIsRunning)
{
	// UE_LOG(LogTemp, Verbose, TEXT("HeroPaperZDAnimInstance::OnIntentStateChanged - Walking: %s, Running: %s"),
	//        (bNewIsWalking ? TEXT("true") : TEXT("false")), (bNewIsRunning ? TEXT("true") : TEXT("false")));
	this->bIsWalking = bNewIsWalking;
	this->bIsRunning = bNewIsRunning;
}

// 更新冲刺状态
void UHeroPaperZDAnimInstance::OnDashStateChanged_Implementation(bool bNewIsDashing)
{
	// UE_LOG(LogTemp, Verbose, TEXT("HeroPaperZDAnimInstance::OnDashStateChanged - Dashing: %s"),
	//        (bNewIsDashing ? TEXT("true") : TEXT("false")));
	this->bIsDashing = bNewIsDashing;
	if (bIsDashing)
	{
		JumpToNode(AnimationJumpNodeName::Dash); // 跳转到冲刺动画节点
	}
}

// 更新连击数状态
void UHeroPaperZDAnimInstance::OnCombatStateChanged_Implementation(int32 NewComboCount)
{
	// UE_LOG(LogTemp, Verbose, TEXT("HeroPaperZDAnimInstance::OnCombatStateChanged - ComboCount: %d"), NewComboCount);
	this->ComboCount = NewComboCount;
}


void UHeroPaperZDAnimInstance::OnJumpRequested_Implementation()
{
	// 直接跳转到名为 "Jump" 入口节点
	UE_LOG(LogTemp, Log, TEXT("HeroPaperZDAnimInstance: OnJumpRequested_Implementation - Jumping to 'Jump' node."));
	JumpToNode(AnimationJumpNodeName::Jump);
}

// 更新空中攻击状态
void UHeroPaperZDAnimInstance::OnAirAttackStateChanged_Implementation(bool bNewIsAirAttacking)
{
	
	
	this->bIsAirAttacking = bNewIsAirAttacking;
  
}
