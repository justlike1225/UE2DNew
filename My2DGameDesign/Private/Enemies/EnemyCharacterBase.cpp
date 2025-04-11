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
