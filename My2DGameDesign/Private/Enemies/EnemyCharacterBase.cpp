#include "Enemies/EnemyCharacterBase.h"
#include "Components/HealthComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperZDAnimationComponent.h"
#include "PaperZDAnimInstance.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "AI/AIC/EnemyAIControllerBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DataAssets/CharacterMovementSettingsDA.h"
#include "Interfaces/AnimationListener//EnemyMovementAnimListener.h"
#include "Interfaces/AnimationListener//EnemyStateAnimListener.h"
#include "Interfaces/AnimationListener//EnemyMeleeAttackAnimListener.h"
#include "Interfaces/AnimationListener//EnemyRangedAttackAnimListener.h"
#include "Interfaces/AnimationListener//EnemyTeleportAnimListener.h"
#include "Interfaces/UI/HealthBarWidgetInterface.h"

AEnemyCharacterBase::AEnemyCharacterBase()
{
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	AIControllerClass = AEnemyAIControllerBase::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidgetComponent"));
	HealthBarWidgetComponent->SetupAttachment(RootComponent);
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	HealthBarWidgetComponent->SetDrawSize(FIntPoint(100, 15));
	HealthBarWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarWidgetComponent->SetVisibility(false);

	bIsFacingRight = bAssetFacesRightByDefault;
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bUseControllerDesiredRotation = false;
		MoveComp->bOrientRotationToMovement = false;
		MoveComp->RotationRate = FRotator(0.f, 0.f, 0.f);
		MoveComp->bConstrainToPlane = true;
		MoveComp->SetPlaneConstraintAxisSetting(EPlaneConstraintAxisSetting::Y);
		MoveComp->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
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
		HealthComponent->OnHealthChanged.AddDynamic(this, &AEnemyCharacterBase::OnHealthChangedHandler);

		if (HealthBarWidgetComponent && HealthBarWidgetClass)
		{
			HealthBarWidgetComponent->SetWidgetClass(HealthBarWidgetClass);
			UUserWidget* HealthWidget = HealthBarWidgetComponent->GetUserWidgetObject();
			if (HealthWidget)
			{
				OnHealthChangedHandler(HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
				HealthBarWidgetComponent->SetVisibility(true);
			}
			else
			{
				HealthBarWidgetComponent->SetVisibility(false);
			}
		}
		else
		{
			if (HealthBarWidgetComponent) HealthBarWidgetComponent->SetVisibility(false);
		}
	}
	else
	{
		if (HealthBarWidgetComponent) HealthBarWidgetComponent->SetVisibility(false);
	}

	ApplyMovementSettings();
}

void AEnemyCharacterBase::OnHealthChangedHandler(float CurrentHealth, float MaxHealth)
{
	if (!HealthBarWidgetComponent) return;
	UUserWidget* HealthWidget = HealthBarWidgetComponent->GetUserWidgetObject();
	if (!HealthWidget) return;

	// 检查 Widget 是否实现了我们的接口
	if (HealthWidget->GetClass()->ImplementsInterface(UHealthBarWidgetInterface::StaticClass()))
	{
		if (GEngine) // 保留你的调试日志
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Calling Interface UpdateHealth: %.0f / %.0f"), CurrentHealth, MaxHealth));
		}
		// 安全地调用接口函数
		IHealthBarWidgetInterface::Execute_UpdateHealth(HealthWidget, CurrentHealth, MaxHealth);
		HealthBarWidgetComponent->SetVisibility(CurrentHealth > 0);
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Widget does NOT implement IHealthBarWidgetInterface!"));
		}
		// 即使接口未实现，显隐逻辑可能仍需执行
		HealthBarWidgetComponent->SetVisibility(CurrentHealth > 0);
	}
}

void AEnemyCharacterBase::ApplyMovementSettings()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	if (MovementSettings)
	{
		MoveComp->MaxWalkSpeed = MovementSettings->MaxWalkSpeed;
		MoveComp->MaxAcceleration = MovementSettings->MaxAcceleration;
		MoveComp->GroundFriction = MovementSettings->GroundFriction;
		MoveComp->BrakingDecelerationWalking = MovementSettings->BrakingDecelerationWalking;
		MoveComp->JumpZVelocity = MovementSettings->JumpZVelocity;
		MoveComp->AirControl = MovementSettings->AirControl;
		MoveComp->GravityScale = MovementSettings->GravityScale;
		MoveComp->MaxFlySpeed = MovementSettings->MaxFlySpeed;
	}
	else
	{
		MoveComp->MaxWalkSpeed = 150.0f;
		MoveComp->JumpZVelocity = 350.0f;
		MoveComp->GravityScale = 1.2f;
		MoveComp->AirControl = 0.5f;
		MoveComp->BrakingDecelerationWalking = 0.5f;
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
		return 0.f;
	}

	float ActualDamage = HealthComponent->TakeDamage(DamageAmount, DamageCauser, InstigatorController);

	if (ActualDamage > 0.f && !HealthComponent->IsDead())
	{
		TScriptInterface<IEnemyStateAnimListener> StateListener = IEnemySpecificAnimListenerProvider::Execute_GetStateAnimListener(this);

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
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
		MoveComp->DisableMovement();
		MoveComp->SetComponentTickEnabled(false);
	}
	TScriptInterface<IEnemyStateAnimListener> StateListener = IEnemySpecificAnimListenerProvider::Execute_GetStateAnimListener(this);
	if (StateListener)
	{
		StateListener->Execute_OnDeathState(StateListener.GetObject(), Killer);
	}

	if (HealthBarWidgetComponent)
	{
		HealthBarWidgetComponent->SetVisibility(false);
	}

	SetLifeSpan(5.0f);
}

void AEnemyCharacterBase::SetFacingDirection(bool bWantsToFaceRight)
{
	if (bIsFacingRight == bWantsToFaceRight)
	{
		return;
	}

	bIsFacingRight = bWantsToFaceRight;

	UPaperFlipbookComponent* SpriteComponent = GetSprite();
	if (SpriteComponent)
	{
		FVector CurrentScale = SpriteComponent->GetRelativeScale3D();
		float AbsScaleX = FMath::Abs(CurrentScale.X);
		if (FMath::IsNearlyZero(AbsScaleX)) { AbsScaleX = 1.0f; }
		
		bool bDirectionMatchesAsset = (bWantsToFaceRight == bAssetFacesRightByDefault);
		float TargetSign = bDirectionMatchesAsset ? 1.0f : -1.0f;
		float NewScaleX = AbsScaleX * TargetSign;

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
