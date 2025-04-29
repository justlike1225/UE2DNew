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

	if (FMath::IsNearlyZero(MaxHealth))
	{
		MaxHealth = DefaultMaxHealth;
	}
	if (FMath::IsNearlyZero(CurrentHealth))
	{
		CurrentHealth = MaxHealth;
	}

	// 注意：不要强制赋值，CurrentHealth可能是恢复存档过来的
	bIsDead = (CurrentHealth <= 0.0f);

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::RestoreHealth(float NewHealth, float NewMaxHealth)
{
	// 先更新最大值，确保 MaxHealth >= 0.1
	MaxHealth = FMath::Max(0.1f, NewMaxHealth);
	// 再根据新的最大值和传入值设置当前值，并确保在 [0, MaxHealth] 区间内
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	// 根据当前生命值更新死亡状态
	bIsDead = (CurrentHealth <= 0.0f);
	// **重要：** 广播事件，通知 UI 和其他系统更新显示
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	UE_LOG(LogTemp, Log, TEXT("HealthComponent: Health restored to %.1f / %.1f"), CurrentHealth, MaxHealth);
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
