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
	bIsFacingRight = true;
}

void AEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	CacheBaseAnimInstance();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AEnemyCharacterBase::HandleDeath);
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
			if (!bRunSuccess)
			{
			}
			else
			{
			}
		}
		else
		{
		}
	}
}

float AEnemyCharacterBase::ApplyDamage_Implementation(float DamageAmount, AActor* DamageCauser,
                                                      AController* InstigatorController, const FHitResult& HitResult)
{
	if (!HealthComponent)
	{
		return 0.f;
	}

	float ActualDamage = HealthComponent->TakeDamage(DamageAmount, DamageCauser, InstigatorController);

	if (ActualDamage > 0.f && !HealthComponent->IsDead())
	{
		TScriptInterface<IEnemyStateAnimListener> StateListener = GetStateAnimListener();
		if (StateListener)
		{
			FVector HitDirection = FVector::ZeroVector;
			if (DamageCauser)
			{
				HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
			}
			else if (HitResult.IsValidBlockingHit())
			{
				HitDirection = -HitResult.ImpactNormal;
			}
			bool bShouldInterrupt = true;

			StateListener->Execute_OnTakeHit(StateListener.GetObject(), ActualDamage, HitDirection, bShouldInterrupt);
		}
		else
		{
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
	AController* MyController = GetController();
	if (MyController)
	{
		MyController->StopMovement();
		AAIController* AIController = Cast<AAIController>(MyController);
		if (AIController && AIController->GetBrainComponent())
		{
			AIController->GetBrainComponent()->StopLogic("Character Died");
		}
	}
	SetActorEnableCollision(false);
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetComponentTickEnabled(false);
	}

	TScriptInterface<IEnemyStateAnimListener> StateListener = GetStateAnimListener();
	if (StateListener)
	{
		StateListener->Execute_OnDeathState(StateListener.GetObject(), Killer);
	}


	SetLifeSpan(5.0f);
}

void AEnemyCharacterBase::SetFacingDirection(bool bWantsToFaceRight)
{
	// 1. 检查是否真的需要转向 (如果当前朝向和目标朝向一致，则无需操作)
	if (bIsFacingRight == bWantsToFaceRight)
	{
		return;
	}

	// 2. 更新内部状态变量，记录当前朝向
	bIsFacingRight = bWantsToFaceRight;

	// 3. 获取角色的 Sprite 组件 (PaperFlipbookComponent)
	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (SpriteComponent)
	{
		// 4. 获取当前的缩放值
		FVector CurrentScale = SpriteComponent->GetRelativeScale3D();
		// 获取 X 轴缩放的绝对值 (保持原始大小)
		float AbsScaleX = FMath::Abs(CurrentScale.X);
		// 防止原始缩放为 0 导致问题
		if (FMath::IsNearlyZero(AbsScaleX))
		{
			AbsScaleX = 1.0f;
		}

		// 5. 计算目标缩放符号 (正数或负数)
		float TargetSign;

		// 检查期望的朝向 (bWantsToFaceRight) 是否与美术资源默认朝向 (bAssetFacesRightByDefault) 一致
		// bAssetFacesRightByDefault 是你在 AEnemyCharacterBase.h 中定义的一个布尔值，
		// 用于处理美术资源本身是朝左还是朝右绘制的情况。
		bool bDirectionMatchesAsset = (bWantsToFaceRight == bAssetFacesRightByDefault);
		// 如果一致，目标符号为正 (1.0)，不一致则为负 (-1.0)
		TargetSign = bDirectionMatchesAsset ? 1.0f : -1.0f;

		// 6. 计算新的 X 轴缩放值
		float NewScaleX = AbsScaleX * TargetSign;
		// 7. 应用新的缩放值到 Sprite 组件，完成视觉上的翻转
		SpriteComponent->SetRelativeScale3D(FVector(NewScaleX, CurrentScale.Y, CurrentScale.Z));
	}
}

void AEnemyCharacterBase::CacheBaseAnimInstance()
{
	if (UPaperZDAnimationComponent* AnimComp = GetAnimationComponent())
	{
		CachedAnimInstancePtr = AnimComp->GetAnimInstance();
	}
}
