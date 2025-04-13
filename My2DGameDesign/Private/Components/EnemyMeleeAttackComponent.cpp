#include "Components/EnemyMeleeAttackComponent.h"
#include "DataAssets/Enemy/EnemyMeleeAttackSettingsDA.h"
#include "Enemies/EnemyCharacterBase.h"
#include "Interfaces/MeleeShapeProvider.h"
#include "Components/PrimitiveComponent.h"
#include "Interfaces/AnimationListenerProvider/EnemySpecificAnimListenerProvider.h"
#include "Interfaces/AnimationListener/EnemyMeleeAttackAnimListener.h"
#include "Interfaces/Damageable.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Utils/CombatGameplayStatics.h"
#include "Enum/EnemyMeleeAttackType.h"
UEnemyMeleeAttackComponent::UEnemyMeleeAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);

	bCanAttack = true;
	bIsAttacking = false;
	ActiveCollisionShapeName = NAME_None;
}

void UEnemyMeleeAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwner());
	if (!OwnerEnemyCharacter.IsValid())
	{
		SetActive(false);
		return;
	}

	if (!AttackSettings)
	{
	}
}

void UEnemyMeleeAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
		GetWorld()->GetTimerManager().ClearTimer(ActiveCollisionTimerHandle);
	}
	if (ActiveCollisionShapeName != NAME_None)
	{
		DeactivateMeleeCollision(ActiveCollisionShapeName);
	}
	Super::EndPlay(EndPlayReason);
}

bool UEnemyMeleeAttackComponent::ExecuteAttack(EEnemyMeleeAttackType AttackType, AActor* Target) // <--- 新函数实现
{
	
	if (!bCanAttack || bIsAttacking || !AttackSettings || !OwnerEnemyCharacter.IsValid())
	{
		return false;
	}
	
	int32 AnimationAttackIndex = 0;
	switch(AttackType)
	{
		case EEnemyMeleeAttackType::Attack1:
			AnimationAttackIndex = 1;
			break;
		case EEnemyMeleeAttackType::Attack2:
			AnimationAttackIndex = 2;
			break;
		case EEnemyMeleeAttackType::Default:
		default:
		
			AnimationAttackIndex = 1;
			UE_LOG(LogTemp, Warning, TEXT("UEnemyMeleeAttackComponent: Received Default AttackType, using index 1."));
			break;
	}

	
	bIsAttacking = true;
	bCanAttack = false;
	BeginAttackSwing();
	StartAttackCooldown();

	// 调用动画监听器，传递“决定好”的索引
	TScriptInterface<IEnemyMeleeAttackAnimListener> Listener = GetAnimListener();
	if (Listener)
	{
		// 使用 AnimationAttackIndex
		Listener->Execute_OnMeleeAttackStarted(Listener.GetObject(), Target, AnimationAttackIndex);
		UE_LOG(LogTemp, Log, TEXT("UEnemyMeleeAttackComponent: Started Attack Index %d via AnimListener."), AnimationAttackIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UEnemyMeleeAttackComponent: Could not get AnimListener to start attack animation!"));
	}

	return true;
}


void UEnemyMeleeAttackComponent::ActivateMeleeCollision(FName ShapeIdentifier, float Duration)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || Duration <= 0 || ShapeIdentifier == NAME_None)
	{
		return;
	}

	if (ActiveCollisionShapeName != NAME_None && ActiveCollisionShapeName != ShapeIdentifier)
	{
		DeactivateMeleeCollision(ActiveCollisionShapeName);
	}

	UPrimitiveComponent* ShapeToActivate = nullptr;
	if (IMeleeShapeProvider* ShapeProvider = Cast<IMeleeShapeProvider>(OwnerActor))
	{
		ShapeToActivate = IMeleeShapeProvider::Execute_GetMeleeShapeComponent(OwnerActor, ShapeIdentifier);
	}
	else
	{
		return;
	}

	if (ShapeToActivate)
	{
		if (ShapeIdentifier != ActiveCollisionShapeName)
		{
			BeginAttackSwing();
			ShapeToActivate->OnComponentBeginOverlap.AddDynamic(this, &UEnemyMeleeAttackComponent::HandleAttackOverlap);
		}

		ShapeToActivate->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ActiveCollisionShapeName = ShapeIdentifier;

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(ActiveCollisionTimerHandle);
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUFunction(this, FName("DeactivateMeleeCollision"), ActiveCollisionShapeName);
			GetWorld()->GetTimerManager().SetTimer(ActiveCollisionTimerHandle, TimerDelegate, Duration, false);
		}
	}
}

void UEnemyMeleeAttackComponent::DeactivateMeleeCollision(FName ShapeIdentifier)
{
	if (ShapeIdentifier == NAME_None || ShapeIdentifier != ActiveCollisionShapeName)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	UPrimitiveComponent* ShapeToDeactivate = nullptr;
	if (IMeleeShapeProvider* ShapeProvider = Cast<IMeleeShapeProvider>(OwnerActor))
	{
		ShapeToDeactivate = IMeleeShapeProvider::Execute_GetMeleeShapeComponent(OwnerActor, ShapeIdentifier);
	}

	if (ShapeToDeactivate)
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(ActiveCollisionTimerHandle);
		}

		ShapeToDeactivate->OnComponentBeginOverlap.
		                   RemoveDynamic(this, &UEnemyMeleeAttackComponent::HandleAttackOverlap);
		ShapeToDeactivate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ActiveCollisionShapeName = NAME_None;
	}
}

void UEnemyMeleeAttackComponent::StartAttackCooldown()
{
	if (AttackSettings && AttackSettings->AttackCooldown > 0 && GetWorld())
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(AttackCooldownTimer))
		{
			GetWorld()->GetTimerManager().SetTimer(
				AttackCooldownTimer,
				this,
				&UEnemyMeleeAttackComponent::OnAttackCooldownFinished,
				AttackSettings->AttackCooldown,
				false
			);
		}
	}
	else
	{
		OnAttackCooldownFinished();
	}
}

void UEnemyMeleeAttackComponent::OnAttackCooldownFinished()
{
	bCanAttack = true;
	bIsAttacking = false;
	if (ActiveCollisionShapeName != NAME_None)
	{
		DeactivateMeleeCollision(ActiveCollisionShapeName);
	}
}

TScriptInterface<IEnemyMeleeAttackAnimListener> UEnemyMeleeAttackComponent::GetAnimListener() const
{
	if (OwnerEnemyCharacter.IsValid())
	{
		if (IEnemySpecificAnimListenerProvider* Provider = Cast<IEnemySpecificAnimListenerProvider>(
			OwnerEnemyCharacter.Get()))
		{
			TScriptInterface<IEnemyMeleeAttackAnimListener> Listener = Provider->Execute_GetMeleeAttackAnimListener(
				OwnerEnemyCharacter.Get());
			if (Listener)
			{
				return Listener;
			}
		}
	}
	return nullptr;
}

void UEnemyMeleeAttackComponent::BeginAttackSwing()
{
	HitActorsThisSwing.Empty();
}

void UEnemyMeleeAttackComponent::HandleAttackOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	AActor* OwnerActor = GetOwner();

	if (!OtherActor || OtherActor == OwnerActor || !bIsAttacking)
	{
		return;
	}

	if (HitActorsThisSwing.Contains(OtherActor))
	{
		return;
	}

	if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
	{
		
		if (!UCombatGameplayStatics::CanDamageActor(OwnerActor, OtherActor))
		{
			return; // 如果不能伤害，直接返回
		}
		HitActorsThisSwing.Add(OtherActor);

		float DamageToApply = 0.0f;
		if (AttackSettings)
		{
			DamageToApply = AttackSettings->AttackDamage;
		}

		AController* InstigatorController = nullptr;
		if (AEnemyCharacterBase* OwnerEnemy = Cast<AEnemyCharacterBase>(OwnerActor))
		{
			InstigatorController = OwnerEnemy->GetController();
		}

		IDamageable::Execute_ApplyDamage(OtherActor, DamageToApply, OwnerActor, InstigatorController, SweepResult);
	}
}
