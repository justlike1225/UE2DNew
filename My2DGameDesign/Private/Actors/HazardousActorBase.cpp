


#include "Actors/HazardousActorBase.h"

#include "Components/BoxComponent.h" 
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"
#include "Actors/PaperZDCharacter_SpriteHero.h" 
#include "Interfaces/Damageable.h"             
#include "TimerManager.h"
#include "Engine/World.h"


AHazardousActorBase::AHazardousActorBase()
{
 	
	PrimaryActorTick.bCanEverTick = false; 

	
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	if (CollisionComp) 
	{
		RootComponent = CollisionComp; 

		
		CollisionComp->SetCollisionProfileName(TEXT("OverlapOnlyPawn")); 
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly); 
		CollisionComp->SetGenerateOverlapEvents(true);
		CollisionComp->SetSimulatePhysics(false); 
		CollisionComp->CanCharacterStepUpOn = ECB_No;

		
		CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AHazardousActorBase::OnHazardOverlapBegin);
		CollisionComp->OnComponentEndOverlap.AddDynamic(this, &AHazardousActorBase::OnHazardOverlapEnd);
	}

	
	SpriteComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComp"));
	if (SpriteComp && RootComponent) 
	{
		SpriteComp->SetupAttachment(RootComponent);
		SpriteComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
	}
}


void AHazardousActorBase::BeginPlay()
{
	Super::BeginPlay();

}

void AHazardousActorBase::OnHazardOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    
	APaperZDCharacter_SpriteHero* PlayerHero = Cast<APaperZDCharacter_SpriteHero>(OtherActor);

    
	if (PlayerHero && !OverlappingHeroPtr.IsValid())
	{
		OverlappingHeroPtr = PlayerHero; 

        UE_LOG(LogTemp, Log, TEXT("Hazard %s: Player %s entered."), *GetNameSafe(this), *PlayerHero->GetName());

		
		if (bDamageOnInitialOverlap)
		{
            UE_LOG(LogTemp, Verbose, TEXT("Hazard %s: Applying initial damage."), *GetNameSafe(this));
			ApplyPeriodicDamage(); 
		}

		
        
        const float InitialDelay = bDamageOnInitialOverlap ? DamageInterval : 0.0f;

        if (DamageInterval > 0) 
        {
            GetWorldTimerManager().SetTimer(
                DamageTimerHandle,         
                this,                      
                &AHazardousActorBase::ApplyPeriodicDamage, 
                DamageInterval,            
                true,                      
                InitialDelay               
            );
             UE_LOG(LogTemp, Verbose, TEXT("Hazard %s: Damage timer started with interval %.2f and initial delay %.2f."), *GetNameSafe(this), DamageInterval, InitialDelay);
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("Hazard %s: DamageInterval is zero or negative, timer not started."), *GetNameSafe(this));
        }
	}
     else if (PlayerHero && OverlappingHeroPtr.IsValid())
     {
         
         UE_LOG(LogTemp, Warning, TEXT("Hazard %s: Another player %s overlapped while %s is already tracked? Ignoring."), *GetNameSafe(this), *PlayerHero->GetName(), *OverlappingHeroPtr->GetName());
     }
}

void AHazardousActorBase::OnHazardOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    
	APaperZDCharacter_SpriteHero* PlayerHero = Cast<APaperZDCharacter_SpriteHero>(OtherActor);

    
	if (PlayerHero && PlayerHero == OverlappingHeroPtr.Get())
	{
        UE_LOG(LogTemp, Log, TEXT("Hazard %s: Player %s exited."), *GetNameSafe(this), *PlayerHero->GetName());

		
		OverlappingHeroPtr.Reset();

		
		GetWorldTimerManager().ClearTimer(DamageTimerHandle);
        UE_LOG(LogTemp, Verbose, TEXT("Hazard %s: Damage timer cleared."), *GetNameSafe(this));
	}
}

void AHazardousActorBase::ApplyPeriodicDamage()
{
	
	if (!OverlappingHeroPtr.IsValid())
	{
        
        if (DamageTimerHandle.IsValid()) 
        {
             UE_LOG(LogTemp, Warning, TEXT("Hazard %s: ApplyPeriodicDamage called but OverlappingHeroPtr is invalid. Stopping timer."), *GetNameSafe(this));
		    GetWorldTimerManager().ClearTimer(DamageTimerHandle);
        }
		return;
	}

	APaperZDCharacter_SpriteHero* PlayerHero = OverlappingHeroPtr.Get();

	
	if (!IsValid(PlayerHero))
	{
		UE_LOG(LogTemp, Error, TEXT("Hazard %s: PlayerHero pointer was valid but became null unexpectedly! Stopping timer."), *GetNameSafe(this));
		OverlappingHeroPtr.Reset();
		GetWorldTimerManager().ClearTimer(DamageTimerHandle);
		return;
	}


	
	if (PlayerHero->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
	{
		UE_LOG(LogTemp, Verbose, TEXT("Hazard %s: Applying %.2f damage to %s via IDamageable."), *GetNameSafe(this), DamageAmount, *PlayerHero->GetName());

		
        
        
		FHitResult EmptyHit; 
		IDamageable::Execute_ApplyDamage(PlayerHero, DamageAmount, this, nullptr, EmptyHit);
        
	}
	else
	{
        
		UE_LOG(LogTemp, Error, TEXT("Hazard %s: Target player %s does NOT implement IDamageable! Cannot apply damage."), *GetNameSafe(this), *PlayerHero->GetName());
		
        OverlappingHeroPtr.Reset();
		GetWorldTimerManager().ClearTimer(DamageTimerHandle);
	}
}