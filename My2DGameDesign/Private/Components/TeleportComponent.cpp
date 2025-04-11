#include "Components/TeleportComponent.h"
#include "DataAssets/Enemy/TeleportSettingsDA.h"
#include "Enemies/EnemyCharacterBase.h"
#include "Interfaces/AnimationListenerProvider/EnemySpecificAnimListenerProvider.h"
#include "Interfaces/AnimationListener/EnemyTeleportAnimListener.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

UTeleportComponent::UTeleportComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);

	bCanTeleport = true;
	bIsTeleporting = false;
	PendingTeleportLocation = FVector::ZeroVector;
}

void UTeleportComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwner());
	if (!OwnerEnemyCharacter.IsValid())
	{
		SetActive(false);
		return;
	}

	if (!TeleportSettings)
	{
	}
}

void UTeleportComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TeleportCooldownTimer);
		GetWorld()->GetTimerManager().ClearTimer(TeleportCastTimer);
	}
	Super::EndPlay(EndPlayReason);
}

bool UTeleportComponent::ExecuteTeleport(const FVector& TargetLocation)
{
	if (!bCanTeleport || bIsTeleporting || !TeleportSettings || !OwnerEnemyCharacter.IsValid() || !GetWorld())
	{
		return false;
	}

	bIsTeleporting = true;
	bCanTeleport = false;
	PendingTeleportLocation = TargetLocation;

	TScriptInterface<IEnemyTeleportAnimListener> Listener = GetAnimListener();
	if (Listener)
	{
		Listener->Execute_OnTeleportStateChanged(Listener.GetObject(), true);
	}


	StartTeleportCooldown();

	if (TeleportSettings->TeleportCastTime > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(
			TeleportCastTimer,
			this,
			&UTeleportComponent::PerformActualTeleport,
			TeleportSettings->TeleportCastTime,
			false
		);
	}
	else
	{
		PerformActualTeleport();
	}

	return true;
}

void UTeleportComponent::PerformActualTeleport()
{
	if (!bIsTeleporting || !OwnerEnemyCharacter.IsValid())
	{
		return;
	}

	bool bTeleported = TrySetActorLocation(PendingTeleportLocation);

	if (bTeleported)
	{
	}
	else
	{
		FinishTeleportState();
	}
}

void UTeleportComponent::FinishTeleportState()
{
	if (!bIsTeleporting)
	{
		return;
	}

	bIsTeleporting = false;

	TScriptInterface<IEnemyTeleportAnimListener> Listener = GetAnimListener();
	if (Listener)
	{
		Listener->Execute_OnTeleportStateChanged(Listener.GetObject(), false);
	}
}

void UTeleportComponent::StartTeleportCooldown()
{
	if (TeleportSettings && TeleportSettings->TeleportCooldown > 0 && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			TeleportCooldownTimer,
			this,
			&UTeleportComponent::OnTeleportCooldownFinished,
			TeleportSettings->TeleportCooldown,
			false
		);
	}
	else
	{
		OnTeleportCooldownFinished();
	}
}

void UTeleportComponent::OnTeleportCooldownFinished()
{
	bCanTeleport = true;
}

TScriptInterface<IEnemyTeleportAnimListener> UTeleportComponent::GetAnimListener() const
{
	if (OwnerEnemyCharacter.IsValid())
	{
		if (IEnemySpecificAnimListenerProvider* Provider = Cast<IEnemySpecificAnimListenerProvider>(
			OwnerEnemyCharacter.Get()))
		{
			TScriptInterface<IEnemyTeleportAnimListener> Listener = Provider->Execute_GetTeleportAnimListener(
				OwnerEnemyCharacter.Get());
			if (Listener)
			{
				return Listener;
			}
		}
	}
	return nullptr;
}

bool UTeleportComponent::TrySetActorLocation(const FVector& NewLocation)
{
	if (OwnerEnemyCharacter.IsValid())
	{
		return OwnerEnemyCharacter->TeleportTo(NewLocation, OwnerEnemyCharacter->GetActorRotation(), false, false);
	}
	return false;
}
