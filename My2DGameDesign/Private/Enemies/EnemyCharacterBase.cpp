#include "Enemies/EnemyCharacterBase.h"
#include "Components/HealthComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperZDAnimationComponent.h"
#include "PaperZDAnimInstance.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DataAssets/CharacterMovementSettingsDA.h"
#include "Interfaces/AnimationListener//EnemyMovementAnimListener.h"
#include "Interfaces/AnimationListener//EnemyStateAnimListener.h"
#include "Interfaces/AnimationListener//EnemyMeleeAttackAnimListener.h"
#include "Interfaces/AnimationListener//EnemyRangedAttackAnimListener.h"
#include "Interfaces/AnimationListener//EnemyTeleportAnimListener.h"


AEnemyCharacterBase::AEnemyCharacterBase()
{
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bIsFacingRight = bAssetFacesRightByDefault;

	
	// --- 关键配置：阻止移动组件自动旋转角色 ---
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bUseControllerDesiredRotation = false;

		
		MoveComp->bOrientRotationToMovement = false;

		MoveComp->RotationRate = FRotator(0.f, 0.f, 0.f);

		
		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y); // 通常约束在 Y=0 的平面
		MoveComp->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f)); // 平面法线指向 Y 轴
		bUseControllerRotationYaw = false;
	}
}

void AEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	CacheBaseAnimInstance();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AEnemyCharacterBase::HandleDeath);
	}
	ApplyMovementSettings();
}

// 新增辅助函数实现
void AEnemyCharacterBase::ApplyMovementSettings()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		UE_LOG(LogTemp, Error, TEXT("ApplyMovementSettings: CharacterMovementComponent is missing on %s!"), *GetNameSafe(this));
		return;
	}

	if (MovementSettings) // 检查数据资产是否有效
	{
		UE_LOG(LogTemp, Log, TEXT("ApplyMovementSettings: Applying MovementSettings DA '%s' to %s"), *GetNameSafe(MovementSettings), *GetNameSafe(this));

		// --- 从数据资产应用设置到移动组件 ---
		MoveComp->MaxWalkSpeed = MovementSettings->MaxWalkSpeed;
		// 敌人通常不区分跑/走，但如果需要，可以读取 MaxRunSpeed
		MoveComp->MaxAcceleration = MovementSettings->MaxAcceleration;
		MoveComp->GroundFriction = MovementSettings->GroundFriction;
		MoveComp->BrakingDecelerationWalking = MovementSettings->BrakingDecelerationWalking;
		MoveComp->JumpZVelocity = MovementSettings->JumpZVelocity;
		MoveComp->AirControl = MovementSettings->AirControl;
		MoveComp->GravityScale = MovementSettings->GravityScale;
		MoveComp ->MaxFlySpeed = MovementSettings->MaxFlySpeed; 
		// ... 应用其他你添加到数据资产的属性 ...
	}
	else
	{
		// 如果没有指定数据资产，可以选择:
		// 1. 记录警告，使用移动组件的默认值。
		// 2. 在这里设置一套硬编码的默认值。
		UE_LOG(LogTemp, Warning, TEXT("ApplyMovementSettings: MovementSettings DataAsset is not assigned to %s. Using default CharacterMovementComponent values."), *GetNameSafe(this));
		
		 MoveComp->MaxWalkSpeed = 150.0f;
		 MoveComp->JumpZVelocity = 350.0f;
		 MoveComp->GravityScale = 1.2f;
		 MoveComp ->AirControl = 0.5f;
		 MoveComp ->BrakingDecelerationWalking = 0.5f;
		 
		
	}
}
void AEnemyCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AAIController* AIController = Cast<AAIController>(NewController);
	if (AIController)
	{
		UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();

		if (BehaviorTree && BehaviorTree->BlackboardAsset)
		{
			if (BlackboardComp)
			{
				BlackboardComp->InitializeBlackboard(*(BehaviorTree->BlackboardAsset));
			}
			else
			{
				bool bSuccess = AIController->UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp);
				if (!bSuccess || !BlackboardComp)
				{
					return;
				}
			}
			BlackboardComp->SetValueAsObject(FName("SelfActor"), this);
			bool bRunSuccess = AIController->RunBehaviorTree(BehaviorTree);
		
		}
	
	}
}
float AEnemyCharacterBase::ApplyDamage_Implementation(float DamageAmount, AActor* DamageCauser,
													  AController* InstigatorController, const FHitResult& HitResult)
{
	if (!HealthComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemyCharacterBase::ApplyDamage: HealthComponent is missing!"));
		return 0.f;
	}

	// 让 HealthComponent 处理实际的伤害计算和死亡逻辑
	float ActualDamage = HealthComponent->TakeDamage(DamageAmount, DamageCauser, InstigatorController);

	// 如果造成了实际伤害，并且角色还未死亡，则尝试通知动画状态监听器播放受击动画
	if (ActualDamage > 0.f && !HealthComponent->IsDead())
	{
		// --- 开始修改 ---
		// TScriptInterface<IEnemyStateAnimListener> StateListener = GetStateAnimListener(); // <--- 旧的调用方式

		// 使用 Execute_ 版本调用接口函数，并在 'this' 对象上执行
		TScriptInterface<IEnemyStateAnimListener> StateListener = IEnemySpecificAnimListenerProvider::Execute_GetStateAnimListener(this);
		// --- 修改结束 ---

		if (StateListener) // 确保获取到了有效的监听器
		{
			// 计算受击方向
			FVector HitDirection = FVector::ZeroVector;
			if (DamageCauser) // 如果有伤害来源 Actor
			{
				// 方向是从来源指向自身
				HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
			}
			else if (HitResult.IsValidBlockingHit()) // 如果没有来源 Actor，但有有效的命中结果
			{
				// 方向是命中法线的反方向
				HitDirection = -HitResult.ImpactNormal;
			}
			// 假设受击总是应该中断当前动作（可以根据需要修改为配置项）
			bool bShouldInterrupt = true;

			// 正确地调用 StateListener 接口的 OnTakeHit 函数
			StateListener->Execute_OnTakeHit(StateListener.GetObject(), ActualDamage, HitDirection, bShouldInterrupt);
		}
		else
		{
			// 如果获取不到 StateListener，可能需要记录日志
			UE_LOG(LogTemp, Warning, TEXT("AEnemyCharacterBase::ApplyDamage: Could not get StateAnimListener for %s."), *GetNameSafe(this));
		}
	}
	return ActualDamage;
}

TScriptInterface<IEnemyMovementAnimListener> AEnemyCharacterBase::GetMovementAnimListener_Implementation() const
{
	return TScriptInterface<IEnemyMovementAnimListener>(CachedAnimInstancePtr.Get());
}

TScriptInterface<IEnemyStateAnimListener> AEnemyCharacterBase::GetStateAnimListener_Implementation() const
{
	return TScriptInterface<IEnemyStateAnimListener>(CachedAnimInstancePtr.Get());
}

TScriptInterface<IEnemyMeleeAttackAnimListener> AEnemyCharacterBase::GetMeleeAttackAnimListener_Implementation() const
{
	return TScriptInterface<IEnemyMeleeAttackAnimListener>(CachedAnimInstancePtr.Get());
}

TScriptInterface<IEnemyRangedAttackAnimListener> AEnemyCharacterBase::GetRangedAttackAnimListener_Implementation() const
{
	return TScriptInterface<IEnemyRangedAttackAnimListener>(CachedAnimInstancePtr.Get());
}

TScriptInterface<IEnemyTeleportAnimListener> AEnemyCharacterBase::GetTeleportAnimListener_Implementation() const
{
	return TScriptInterface<IEnemyTeleportAnimListener>(CachedAnimInstancePtr.Get());
}

FVector AEnemyCharacterBase::GetFacingDirection_Implementation() const
{
	return bIsFacingRight ? FVector::ForwardVector : -FVector::ForwardVector;
}

void AEnemyCharacterBase::HandleDeath(AActor* Killer)
{
	// 停止 AI 逻辑
	AController* MyController = GetController();
	if (MyController)
	{
		MyController->StopMovement(); // 停止当前移动指令
		AAIController* AIController = Cast<AAIController>(MyController);
		if (AIController && AIController->GetBrainComponent())
		{
			// 停止行为树
			AIController->GetBrainComponent()->StopLogic("Character Died");
			UE_LOG(LogTemp, Log, TEXT("%s: AI BrainComponent stopped."), *GetNameSafe(this));
		}
	}

	// 禁用碰撞和移动
	SetActorEnableCollision(false); // 禁用 Actor 的主碰撞
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately(); // 立即停止移动
		MoveComp->DisableMovement();         // 禁用移动模式
		MoveComp->SetComponentTickEnabled(false); // 禁用移动组件 Tick
		UE_LOG(LogTemp, Log, TEXT("%s: Collision and Movement disabled."), *GetNameSafe(this));
	}

	// 通知动画状态机进入死亡状态
	UE_LOG(LogTemp, Log, TEXT("%s: Attempting to notify AnimInstance of death."), *GetNameSafe(this));

	// --- 开始修改 ---
	// TScriptInterface<IEnemyStateAnimListener> StateListener = GetStateAnimListener(); // <--- 旧的调用方式

	// 使用 Execute_ 版本调用接口函数，并在 'this' 对象上执行
	TScriptInterface<IEnemyStateAnimListener> StateListener = IEnemySpecificAnimListenerProvider::Execute_GetStateAnimListener(this);
	// --- 修改结束 ---

	if (StateListener) // 检查是否成功获取监听器
	{
		// 正确地调用 StateListener 接口的 OnDeathState 函数
		StateListener->Execute_OnDeathState(StateListener.GetObject(), Killer);
		UE_LOG(LogTemp, Log, TEXT("%s: Notified StateAnimListener via Execute_OnDeathState."), *GetNameSafe(this));
	}
	else
	{
		// 如果获取不到 StateListener，记录日志
		UE_LOG(LogTemp, Warning, TEXT("AEnemyCharacterBase::HandleDeath: Could not get StateAnimListener for %s."), *GetNameSafe(this));
		// 即使没有动画监听器，死亡逻辑（如销毁）也应继续
	}

	// 设置 Actor 的生命周期，让它在一段时间后自动从场景中移除
	SetLifeSpan(5.0f); // 例如 5 秒后销毁
	UE_LOG(LogTemp, Log, TEXT("%s: Set lifespan to 5 seconds."), *GetNameSafe(this));
}


// 设置角色朝向 (包含日志)
void AEnemyCharacterBase::SetFacingDirection(bool bWantsToFaceRight)
{
	
	if (bIsFacingRight == bWantsToFaceRight)
	{
		UE_LOG(LogTemp, Log, TEXT("    SetFacingDirection: Already facing the desired direction (bIsFacingRight=%s). Returning."), bIsFacingRight ? TEXT("true") : TEXT("false")); // <<< LOG B: 无需转向
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("    SetFacingDirection: Current bIsFacingRight=%s, Changing direction."), bIsFacingRight ? TEXT("true") : TEXT("false")); // <<< LOG C: 准备转向
	bIsFacingRight = bWantsToFaceRight; // 更新内部状态

	UPaperFlipbookComponent* SpriteComponent = GetSprite(); // 获取 Sprite 组件
	if (SpriteComponent)
	{
		FVector CurrentScale = SpriteComponent->GetRelativeScale3D();
		float AbsScaleX = FMath::Abs(CurrentScale.X);
		if (FMath::IsNearlyZero(AbsScaleX)) { AbsScaleX = 1.0f; } // 防止缩放为0
		
		bool bDirectionMatchesAsset = (bWantsToFaceRight == bAssetFacesRightByDefault);
		float TargetSign = bDirectionMatchesAsset ? 1.0f : -1.0f; // 计算符号
		float NewScaleX = AbsScaleX * TargetSign; // 计算新的 X 轴缩放


		SpriteComponent->SetRelativeScale3D(FVector(NewScaleX, CurrentScale.Y, CurrentScale.Z));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("    SetFacingDirection: Failed to get SpriteComponent!")); // <<< LOG F: 获取 Sprite 失败
	}


}

void AEnemyCharacterBase::CacheBaseAnimInstance()
{
	if (UPaperZDAnimationComponent* AnimComp = GetAnimationComponent())
	{
		CachedAnimInstancePtr = AnimComp->GetAnimInstance();
	}
}
