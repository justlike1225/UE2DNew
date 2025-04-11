// My2DGameDesign/Private/Components/HealthComponent.cpp

#include "Components/HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	MaxHealth = DefaultMaxHealth;
	CurrentHealth = MaxHealth;
	bIsDead = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	MaxHealth = DefaultMaxHealth;
	CurrentHealth = MaxHealth;
	bIsDead = false;

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	AActor* MyOwner = GetOwner();
	if (MyOwner)
	{
	}
}

float UHealthComponent::TakeDamage(float DamageAmount, AActor* DamageCauser, AController* InstigatorController)
{
	if (bIsDead || DamageAmount <= 0.f)
	{
		return 0.f;
	}

	float HealthBeforeDamage = CurrentHealth;
	CurrentHealth = FMath::Max(CurrentHealth - DamageAmount, 0.0f);
	float ActualDamage = HealthBeforeDamage - CurrentHealth;

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		HandleDeath(DamageCauser);
	}

	return ActualDamage;
}

void UHealthComponent::HandleDeath(AActor* Killer)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	OnDeath.Broadcast(Killer);
}
