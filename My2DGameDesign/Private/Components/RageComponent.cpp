
#include "Components/RageComponent.h"
#include "GameFramework/Actor.h"

URageComponent::URageComponent()
{
	PrimaryComponentTick.bCanEverTick = false; 
	SetIsReplicatedByDefault(false);
	MaxRage = FMath::Max(0.1f, MaxRage); 
	CurrentRage =100.0f;
}

void URageComponent::RestoreRage(float NewRage, float NewMaxRage)
{
	MaxRage = FMath::Max(0.1f, NewMaxRage);
	CurrentRage = FMath::Clamp(NewRage, 0.0f, NewMaxRage);
	
	OnRageChanged.Broadcast(NewRage, NewMaxRage); // 通知UI更新
}

void URageComponent::BeginPlay()
{
	Super::BeginPlay();

	


	
	OnRageChanged.Broadcast(CurrentRage, MaxRage);
}

void URageComponent::AddRage(float Amount)
{
	if (Amount <= 0.0f) 
	{
		return;
	}

	
	const float PreviousRage = CurrentRage;
	CurrentRage = FMath::Clamp(CurrentRage + Amount, 0.0f, MaxRage);

	
	if (!FMath::IsNearlyEqual(PreviousRage, CurrentRage))
	{
		OnRageChanged.Broadcast(CurrentRage, MaxRage);
		
	}
}

bool URageComponent::ConsumeRage(float AmountToConsume)
{
	if (AmountToConsume <= 0.0f) 
	{
		return true; 
	}

	if (CurrentRage >= AmountToConsume)
	{
		const float PreviousRage = CurrentRage;
		CurrentRage -= AmountToConsume;
		
		CurrentRage = FMath::Max(0.0f, CurrentRage);

		
		OnRageChanged.Broadcast(CurrentRage, MaxRage);
		
		return true; 
	}
	else
	{
		
		return false; 
	}
}
 