#include "AniInstance/EnemyAnimInstanceBase.h"
#include "Enemies/EnemyCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Actor.h"

UEnemyAnimInstanceBase::UEnemyAnimInstanceBase()
{
	Speed = 0.0f;
	bIsFalling = true;
	bIsMoving = false;
	bIsHurt = false;
	bIsDead = false;
}

void UEnemyAnimInstanceBase::OnInit_Implementation()
{
	Super::OnInit_Implementation();

	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwningActor());
	if (OwnerEnemyCharacter.IsValid())
	{
		OwnerMovementComponent = OwnerEnemyCharacter->GetCharacterMovement();
		if (!OwnerMovementComponent.IsValid())
		{
		}
	}
	
}

void UEnemyAnimInstanceBase::OnTick_Implementation(float DeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);


	if (OwnerMovementComponent.IsValid())
	{
		bIsFalling = OwnerMovementComponent->IsFalling();
	}
	else if (OwnerEnemyCharacter.IsValid())
	{
		OwnerMovementComponent = OwnerEnemyCharacter->GetCharacterMovement();
		if (OwnerMovementComponent.IsValid()) { bIsFalling = OwnerMovementComponent->IsFalling(); }
		else
		{
			bIsFalling = true;
			Speed = 0.f;
			bIsMoving = false;
		}
	}
	else
	{
		bIsFalling = true;
		Speed = 0.f;
		bIsMoving = false;
	}
}


void UEnemyAnimInstanceBase::OnMovementStateChanged_Implementation(float InSpeed, bool bInIsFalling, bool bInIsMoving)
{
	this->Speed = InSpeed;
	this->bIsFalling = bInIsFalling;
	this->bIsMoving = bInIsMoving;
}


void UEnemyAnimInstanceBase::OnDeathState_Implementation(AActor* Killer)
{
	this->bIsDead = true;


	this->bIsHurt = false;
	this->bIsMoving = false;
	this->Speed = 0.0f;


	JumpToNode(FName("DeathEntry"));
}


void UEnemyAnimInstanceBase::OnTakeHit_Implementation(float DamageAmount, const FVector& HitDirection,
                                                      bool bInterruptsCurrentAction)
{
	if (bInterruptsCurrentAction && !bIsDead)
	{
		this->bIsHurt = true;


		this->bIsMoving = false;


		JumpToNode(FName("HurtEntry"));
	}
}
