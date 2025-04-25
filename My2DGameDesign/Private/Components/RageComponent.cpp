
#include "Components/RageComponent.h"
#include "GameFramework/Actor.h"

URageComponent::URageComponent()
{
	PrimaryComponentTick.bCanEverTick = false; 
	SetIsReplicatedByDefault(false); 
}

void URageComponent::BeginPlay()
{
	Super::BeginPlay();

	
	MaxRage = FMath::Max(0.1f, MaxRage); 
	CurrentRage =100.0f;

	
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
 