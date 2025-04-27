#include "Components/Skills/UpwardSweepComponent.h"
#include "Actors/PaperZDCharacter_SpriteHero.h"
#include "Components/RageComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/AnimationListener/CharacterAnimationStateListener.h"
#include "DataAssets/HeroDA/HeroUpwardSweepSettingsDA.h"
#include "EnhancedInputComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h" 
#include "Utils/CombatGameplayStatics.h"
#include "Interfaces/Damageable.h"
#include "Interfaces/FacingDirectionProvider.h"
#include "Interfaces/ActionInterruptSource.h"
#include "PaperFlipbookComponent.h" 
#include "Components/DashComponent.h"
#include "Components/HealthComponent.h"
#include "Components/HeroCombatComponent.h"
#include "Components/Skills/RageDashComponent.h"

UUpwardSweepComponent::UUpwardSweepComponent()
{
    PrimaryComponentTick.bCanEverTick = false; 
    SetIsReplicatedByDefault(false); 
}
void UUpwardSweepComponent::BeginPlay()
{
    Super::BeginPlay();
    OwnerHero = Cast<APaperZDCharacter_SpriteHero>(GetOwner());
    if (OwnerHero.IsValid())
    {
        OwnerRageComponent = OwnerHero->GetRageComponent();
        OwnerMovementComponent = OwnerHero->GetCharacterMovement();
         if (IHeroAnimationStateProvider* Provider = Cast<IHeroAnimationStateProvider>(OwnerHero.Get()))
        {
             OwnerAnimListener = Provider->Execute_GetAnimStateListener(OwnerHero.Get());
        }
        OwnerHero->OnActionWillInterrupt.AddDynamic(this, &UUpwardSweepComponent::StopSweepTrace); 
        OwnerHero->OnActionWillInterrupt.AddDynamic(this, &UUpwardSweepComponent::FinishUpwardSweep);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UpwardSweepComponent must be attached to APaperZDCharacter_SpriteHero!"));
        SetActive(false);
        return;
    }
    if (!OwnerRageComponent.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UpwardSweepComponent requires Owner to have URageComponent!"));
        SetActive(false);
    }
    if (!OwnerMovementComponent.IsValid())
    {
         UE_LOG(LogTemp, Error, TEXT("UpwardSweepComponent requires Owner to have UCharacterMovementComponent!"));
         SetActive(false);
    }
     if (!OwnerAnimListener)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpwardSweepComponent could not find Owner's AnimationStateListener."));
    }
    if (!UpwardSweepSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpwardSweepComponent on %s is missing UpwardSweepSettingsDA! Skill may fail."), *GetNameSafe(GetOwner()));
    }
}
void UUpwardSweepComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetWorld())
    {
        GetWorldTimerManager().ClearTimer(UpwardSweepCooldownTimer);
        GetWorldTimerManager().ClearTimer(AttackTraceTimerHandle);
    }
    if (OwnerHero.IsValid() && OwnerHero->OnActionWillInterrupt.IsBound())
    {
        OwnerHero->OnActionWillInterrupt.RemoveDynamic(this, &UUpwardSweepComponent::StopSweepTrace);
         OwnerHero->OnActionWillInterrupt.RemoveDynamic(this, &UUpwardSweepComponent::FinishUpwardSweep);
    }
    Super::EndPlay(EndPlayReason);
}
void UUpwardSweepComponent::BindInputActions_Implementation(UEnhancedInputComponent* EnhancedInputComponent)
{
    if (EnhancedInputComponent && UpwardSweepAction)
    {
        EnhancedInputComponent->BindAction(UpwardSweepAction, ETriggerEvent::Started, this,
                                           &UUpwardSweepComponent::HandleUpwardSweepInputTriggered);
    }
    else
    {
        if (!UpwardSweepAction) UE_LOG(LogTemp, Warning, TEXT("UpwardSweepComponent: UpwardSweepAction is not set in configuration!"));
    }
}
void UUpwardSweepComponent::HandleUpwardSweepInputTriggered(const FInputActionValue& Value)
{
    TryExecuteUpwardSweep();
}
bool UUpwardSweepComponent::CanExecuteUpwardSweep() const
{
    if (!UpwardSweepSettings || !OwnerRageComponent.IsValid() || !OwnerMovementComponent.IsValid() || !OwnerHero.IsValid())
    {
        return false;
    }
    if (!OwnerMovementComponent->IsMovingOnGround())
    {
        return false;
    }
    UHealthComponent* HealthComp = OwnerHero->GetHealthComponent();
    if (bIsPerformingUpwardSweep || bIsUpwardSweepOnCooldown || OwnerHero->IsMovementInputBlocked() || (HealthComp && HealthComp->IsDead()))
    {
        return false;
    }
    if (OwnerRageComponent->GetCurrentRage() < UpwardSweepSettings->RageCost)
    {
        return false;
    }
    UHeroCombatComponent* CombatComp = OwnerHero->GetHeroCombatComponent();
    UDashComponent* DashComp = OwnerHero->GetDashComponent();
    URageDashComponent* RageDashComp = OwnerHero->GetRageDashComponent();
    if (CombatComp && (CombatComp->GetComboCount() > 0 || CombatComp->IsPerformingAirAttack()))
    {
        return false;
    }
    if (DashComp && DashComp->IsDashing())
    {
        return false;
    }
     if (RageDashComp && RageDashComp->IsRageDashing())
    {
        return false;
    }
    return true;
}
void UUpwardSweepComponent::TryExecuteUpwardSweep()
{
    if (CanExecuteUpwardSweep())
    {
        ExecuteUpwardSweep();
    }
     else
    {
    }
}
void UUpwardSweepComponent::ExecuteUpwardSweep()
{
    if (!ensure(UpwardSweepSettings && OwnerRageComponent.IsValid() && OwnerMovementComponent.IsValid() && OwnerHero.IsValid() && GetAnimListener()))
    {
        UE_LOG(LogTemp, Error, TEXT("UpwardSweepComponent::ExecuteUpwardSweep - Critical dependency missing!"));
        return;
    }
    OwnerRageComponent->ConsumeRage(UpwardSweepSettings->RageCost);
    bIsPerformingUpwardSweep = true;
    bIsUpwardSweepOnCooldown = true;
    OwnerHero->SetMovementInputBlocked(true); 
    GetWorldTimerManager().SetTimer(UpwardSweepCooldownTimer, this,
                                &UUpwardSweepComponent::OnUpwardSweepCooldownFinished,
                                UpwardSweepSettings->Cooldown, false);
    OwnerMovementComponent->StopMovementKeepPathing();
    OwnerMovementComponent->Velocity = FVector::ZeroVector; 
    if (IActionInterruptSource* InterruptSource = Cast<IActionInterruptSource>(OwnerHero.Get()))
    {
        InterruptSource->Execute_BroadcastActionInterrupt(OwnerHero.Get());
    }
    GetAnimListener()->Execute_OnUpwardSweepStarted(GetAnimListener().GetObject());
     UE_LOG(LogTemp, Log, TEXT("UpwardSweepComponent: Executed Upward Sweep."));
}
 void UUpwardSweepComponent::FinishUpwardSweep()
{
    if (!bIsPerformingUpwardSweep)
    {
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("UpwardSweepComponent: Finishing Upward Sweep."));
    bIsPerformingUpwardSweep = false;
    OwnerHero->SetMovementInputBlocked(false); 
    StopSweepTrace();
}
void UUpwardSweepComponent::OnUpwardSweepCooldownFinished()
{
    bIsUpwardSweepOnCooldown = false;
     UE_LOG(LogTemp, Log, TEXT("UpwardSweepComponent: Upward Sweep Cooldown Finished."));
}
void UUpwardSweepComponent::StartSweepTrace(float Duration)
{
    if (!OwnerHero.IsValid() || !GetWorld() || Duration <= KINDA_SMALL_NUMBER)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpwardSweepComponent::StartSweepTrace - Invalid conditions (Owner: %s, World: %s, Duration: %.2f)"),
            OwnerHero.IsValid() ? TEXT("Valid") : TEXT("Invalid"),
            GetWorld() ? TEXT("Valid") : TEXT("Invalid"),
            Duration);
        return;
    }
    UPaperFlipbookComponent* SpriteComp = OwnerHero->GetSprite();
     if (!SpriteComp)
    {
         UE_LOG(LogTemp, Warning, TEXT("UpwardSweepComponent::StartSweepTrace - Owner SpriteComponent is null."));
         return;
    }
    bIsTrackingAttackPoint = true;
    HitActorsThisSweep.Empty(); 
    AttackTraceStartTime = GetWorld()->GetTimeSeconds();
    AttackTraceDuration = Duration;
    FVector InitialRelativeOffset = GetInterpolatedOffset(0.0f);
    FVector FacingDirection = FVector::ForwardVector; 
    if (IFacingDirectionProvider* FacingProvider = Cast<IFacingDirectionProvider>(OwnerHero.Get()))
    {
         FacingDirection = FacingProvider->Execute_GetFacingDirection(OwnerHero.Get());
    }
    if (FacingDirection.X < 0.0f)
    {
        InitialRelativeOffset.X *= -1.0f;
    }
     FTransform ComponentTransform = SpriteComp->GetComponentTransform();
    PreviousAttackPointWorldLocation = ComponentTransform.TransformPosition(InitialRelativeOffset);
    const float TimerTickInterval = 0.0166f; 
    GetWorldTimerManager().SetTimer(
        AttackTraceTimerHandle,
        this,
        &UUpwardSweepComponent::PerformSweepTraceTick,
        TimerTickInterval,
        true, 
        0.0f  
    );
     UE_LOG(LogTemp, Log, TEXT("UpwardSweepComponent: Started Sweep Trace (Duration: %.2f)"), Duration);
}
void UUpwardSweepComponent::StopSweepTrace()
{
    if (bIsTrackingAttackPoint)
    {
        if (GetWorld())
        {
            GetWorldTimerManager().ClearTimer(AttackTraceTimerHandle);
        }
        bIsTrackingAttackPoint = false;
         UE_LOG(LogTemp, Log, TEXT("UpwardSweepComponent: Stopped Sweep Trace. Hit %d actors."), HitActorsThisSweep.Num());
    }
}
void UUpwardSweepComponent::PerformSweepTraceTick()
{
    if (!bIsTrackingAttackPoint)
    {
        StopSweepTrace(); 
        return;
    }
    if (!OwnerHero.IsValid() || !UpwardSweepSettings)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpwardSweepComponent::PerformSweepTraceTick - Owner or Settings became invalid. Stopping trace."));
        StopSweepTrace();
        return;
    }
    UWorld* World = GetWorld();
    UPaperFlipbookComponent* SpriteComp = OwnerHero->GetSprite();
    if (!World || !SpriteComp)
    {
         UE_LOG(LogTemp, Warning, TEXT("UpwardSweepComponent::PerformSweepTraceTick - World or SpriteComp became invalid. Stopping trace."));
        StopSweepTrace();
        return;
    }
    float CurrentWorldTime = World->GetTimeSeconds();
    float ElapsedTime = CurrentWorldTime - AttackTraceStartTime;
    if (ElapsedTime >= AttackTraceDuration)
    {
        StopSweepTrace(); 
        return;
    }
    float NormalizedTime = FMath::Clamp(ElapsedTime / AttackTraceDuration, 0.0f, 1.0f);
    FVector FacingDirection = FVector::ForwardVector;
    if (IFacingDirectionProvider* FacingProvider = Cast<IFacingDirectionProvider>(OwnerHero.Get()))
    {
         FacingDirection = FacingProvider->Execute_GetFacingDirection(OwnerHero.Get());
    }
    FVector CurrentRelativeOffset = GetInterpolatedOffset(NormalizedTime);
    if (FacingDirection.X < 0.0f)
    {
        CurrentRelativeOffset.X *= -1.0f;
    }
    FTransform ComponentTransform = SpriteComp->GetComponentTransform();
    FVector CurrentTipWorld = ComponentTransform.TransformPosition(CurrentRelativeOffset);
    FVector PrevTipWorld = PreviousAttackPointWorldLocation;
    FVector BladeVector = CurrentTipWorld - PrevTipWorld;
    float BladeLength = BladeVector.Size();
    if (BladeLength < KINDA_SMALL_NUMBER)
    {
        PreviousAttackPointWorldLocation = CurrentTipWorld; 
        return;
    }
    FVector BladeDir = BladeVector / BladeLength;
    float BladeHalfLength = BladeLength * 0.5f;
    const float BladeThickness = UpwardSweepSettings->Damage > 0 ? 10.0f : 5.0f; 
    FVector BoxHalfExtent(BladeHalfLength, BladeThickness * 0.5f, BladeThickness * 0.5f);
    FVector PrevCenter = PrevTipWorld + BladeDir * BladeHalfLength;
    FVector CurrCenter = PrevTipWorld + BladeVector * 0.5f; 
    FQuat BoxQuat = FRotationMatrix::MakeFromX(BladeDir).ToQuat();
    FRotator BoxOrientation = BoxQuat.Rotator();
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = { UEngineTypes::ConvertToObjectType(ECC_Pawn) }; 
    TArray<AActor*> ActorsToIgnore = { OwnerHero.Get() }; 
    TArray<FHitResult> HitResults;
    UKismetSystemLibrary::BoxTraceMultiForObjects(
        this,               
        PrevCenter,         
        CurrCenter,         
        BoxHalfExtent,      
        BoxOrientation,     
        ObjectTypes,        
        false,              
        ActorsToIgnore,     
        EDrawDebugTrace::None, 
        HitResults,
        true               
    );
    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || HitActorsThisSweep.Contains(HitActor)) 
        {
            continue;
        }
        if (!UCombatGameplayStatics::CanDamageActor(OwnerHero.Get(), HitActor))
        {
             UE_LOG(LogTemp, Verbose, TEXT("UpwardSweepComponent: Cannot damage actor %s (Team check)."), *HitActor->GetName());
            continue;
        }
        if (HitActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
        {
            float DamageToApply = UpwardSweepSettings->Damage;
            AController* InstigatorController = OwnerHero->GetController();
             UE_LOG(LogTemp, Log, TEXT("UpwardSweepComponent: Hit Damageable Actor %s."), *HitActor->GetName());
            IDamageable::Execute_ApplyDamage(HitActor, DamageToApply, OwnerHero.Get(), InstigatorController, Hit);
            HitActorsThisSweep.Add(HitActor);
        }
        else
        {
             UE_LOG(LogTemp, Verbose, TEXT("UpwardSweepComponent: Hit Actor %s does not implement IDamageable."), *HitActor->GetName());
        }
    }
    PreviousAttackPointWorldLocation = CurrentTipWorld;
}
 FVector UUpwardSweepComponent::GetInterpolatedOffset(float NormalizedTime) const
{
    int32 NumKeys = UpwardSweepKeyframeTimes.Num();
    if (UpwardSweepKeyframeOffsets.Num() != NumKeys || NumKeys < 2)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("UUpwardSweepComponent::GetInterpolatedOffset - Invalid keyframe data (Count mismatch or < 2 keys). Returning default offset."));
        return UpwardSweepKeyframeOffsets.IsValidIndex(0) ? UpwardSweepKeyframeOffsets[0] : FVector::ZeroVector;
    }
    NormalizedTime = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    int32 IndexB = -1;
    for (int32 i = 0; i < NumKeys; ++i)
    {
        if (UpwardSweepKeyframeTimes[i] >= NormalizedTime)
        {
            IndexB = i;
            break;
        }
    }
    if (IndexB <= 0) 
    {
        return UpwardSweepKeyframeOffsets[0];
    }
    else if (IndexB >= NumKeys) 
    {
        return UpwardSweepKeyframeOffsets.Last();
    }
    else 
    {
        int32 IndexA = IndexB - 1;
        float TimeA = UpwardSweepKeyframeTimes[IndexA];
        float TimeB = UpwardSweepKeyframeTimes[IndexB];
        FVector OffsetA = UpwardSweepKeyframeOffsets[IndexA];
        FVector OffsetB = UpwardSweepKeyframeOffsets[IndexB];
        if (FMath::IsNearlyEqual(TimeA, TimeB))
        {
            return OffsetA; 
        }
        float Alpha = (NormalizedTime - TimeA) / (TimeB - TimeA);
        Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f); 
        return FMath::Lerp<FVector>(OffsetA, OffsetB, Alpha);
    }
}
TScriptInterface<ICharacterAnimationStateListener> UUpwardSweepComponent::GetAnimListener() 
{
    if (OwnerAnimListener)
    {
        return OwnerAnimListener;
    }
    if (OwnerHero.IsValid())
    {
         if (IHeroAnimationStateProvider* Provider = Cast<IHeroAnimationStateProvider>(OwnerHero.Get()))
        {
            
             return  OwnerAnimListener = Provider->Execute_GetAnimStateListener(OwnerHero.Get());
        }
    }
    return nullptr; 
}
 FTimerManager& UUpwardSweepComponent::GetWorldTimerManager() const
{
    check(GetWorld());
    return GetWorld()->GetTimerManager();
}