#include "Components/EnemyRangedAttackComponent.h"
#include "DataAssets/Enemy/EnemyRangedAttackSettingsDA.h"
#include "Actors/EnemyProjectileBase.h"
#include "Enemies/EnemyCharacterBase.h"

#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"

UEnemyRangedAttackComponent::UEnemyRangedAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);

	bCanAttack = true;
	bIsAttacking = false;
}

void UEnemyRangedAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwner());
	if (!OwnerEnemyCharacter.IsValid())
	{
		return;
	}

	if (!AttackSettings)
	{
	}
	else if (!AttackSettings->ProjectileClass)
	{
	}
}

void UEnemyRangedAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
	}
	Super::EndPlay(EndPlayReason);
}

bool UEnemyRangedAttackComponent::ExecuteAttack(AActor* Target)
{
	if (!bCanAttack || bIsAttacking || !Target || !AttackSettings || !AttackSettings->ProjectileClass || !
		OwnerEnemyCharacter.IsValid())
	{
		return false;
	}
	// ... (可选的距离检查等) ...

	bIsAttacking = true;
	bCanAttack = false;
	CurrentTarget = Target;
	StartAttackCooldown();

	IEnemySpecificAnimListenerProvider* Provider = Cast<IEnemySpecificAnimListenerProvider>(OwnerEnemyCharacter.Get());
	if (Provider)
	{
		TScriptInterface<IEnemyRangedAttackAnimListener> RangedListener = Provider->Execute_GetRangedAttackAnimListener(
			OwnerEnemyCharacter.Get());

		if (RangedListener)
		{
			RangedListener->Execute_OnRangedAttackStarted(RangedListener.GetObject(), Target);
		}
		else
		{
		}
	}

	return true;
}

void UEnemyRangedAttackComponent::HandleSpawnProjectile()
{
	if (!bIsAttacking || !CurrentTarget.IsValid() || !AttackSettings || !AttackSettings->ProjectileClass || !
		OwnerEnemyCharacter.IsValid() || !GetWorld())
	{
		return;
	}

	FVector SpawnLocation;
	FRotator SpawnRotation;
	if (!CalculateSpawnTransform(SpawnLocation, SpawnRotation))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwnerEnemyCharacter.Get();
	SpawnParams.Instigator = OwnerEnemyCharacter.Get();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AEnemyProjectileBase* NewProjectile = GetWorld()->SpawnActor<AEnemyProjectileBase>(
		AttackSettings->ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (NewProjectile)
	{
		float Speed = AttackSettings->ProjectileSpeed;
		float Damage = AttackSettings->AttackDamage;
		float LifeSpan = AttackSettings->ProjectileLifeSpan;
		AActor* Shooter = OwnerEnemyCharacter.Get();
		AController* ShooterController = OwnerEnemyCharacter->GetController();
		FVector Direction = (CurrentTarget->GetActorLocation() - SpawnLocation).GetSafeNormal();

		NewProjectile->InitializeProjectile(Direction, Speed, Damage, LifeSpan, Shooter, ShooterController);
	}
	
}

void UEnemyRangedAttackComponent::StartAttackCooldown()
{
	if (AttackSettings && AttackSettings->AttackCooldown > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AttackCooldownTimer,
			this,
			&UEnemyRangedAttackComponent::OnAttackCooldownFinished,
			AttackSettings->AttackCooldown,
			false
		);
	}
	else
	{
		OnAttackCooldownFinished();
	}
}

void UEnemyRangedAttackComponent::OnAttackCooldownFinished()
{
	bCanAttack = true;
	bIsAttacking = false;
	CurrentTarget = nullptr;
}

bool UEnemyRangedAttackComponent::CalculateSpawnTransform(FVector& OutSpawnLocation, FRotator& OutSpawnRotation) const
{
	if (!OwnerEnemyCharacter.IsValid() || !CurrentTarget.IsValid() || !AttackSettings)
	{
		return false;
	}

	FVector OwnerLocation = OwnerEnemyCharacter->GetActorLocation();
	FRotator OwnerRotation = OwnerEnemyCharacter->GetActorRotation();

	FVector Offset = AttackSettings->SpawnOffset;
	FVector FacingDirection = OwnerEnemyCharacter->GetFacingDirection();
	if (FacingDirection.X < 0)
	{
		Offset.X *= -1.0f;
	}
	FVector WorldOffset = OwnerRotation.RotateVector(Offset);

	OutSpawnLocation = OwnerLocation + WorldOffset;

	FVector DirectionToTarget = (CurrentTarget->GetActorLocation() - OutSpawnLocation).GetSafeNormal();
	OutSpawnRotation = UKismetMathLibrary::FindLookAtRotation(OutSpawnLocation, CurrentTarget->GetActorLocation());

	return true;
}
