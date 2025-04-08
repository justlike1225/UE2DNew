// My2DGameDesign/Private/Enemies/EnemyCharacterBase.cpp (Refactored)

#include "Enemies/EnemyCharacterBase.h"
#include "Components/HealthComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperZDAnimationComponent.h"
#include "PaperZDAnimInstance.h" // <-- 需要基础动画实例类
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AniInstance/EnemyAnimInstanceBase.h"
#include "Interfaces/AnimationListener//EnemyMovementAnimListener.h"
#include "Interfaces/AnimationListener//EnemyStateAnimListener.h"
#include "Interfaces/AnimationListener//EnemyMeleeAttackAnimListener.h"
#include "Interfaces/AnimationListener//EnemyRangedAttackAnimListener.h"
#include "Interfaces/AnimationListener//EnemyTeleportAnimListener.h"


// 构造函数 (基本不变)
AEnemyCharacterBase::AEnemyCharacterBase()
{
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    AIControllerClass = AAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    bIsFacingRight = true;
}

// BeginPlay: 修改为调用新的缓存函数
void AEnemyCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // 尝试查找并缓存基础动画实例指针
    CacheBaseAnimInstance(); // <-- 调用新的缓存函数

    if(HealthComponent) {
        HealthComponent->OnDeath.AddDynamic(this, &AEnemyCharacterBase::HandleDeath);
    } else {
        UE_LOG(LogTemp, Error, TEXT("EnemyCharacterBase '%s' has no valid HealthComponent!"), *GetName());
    }
}

// PossessedBy (基本不变)
void AEnemyCharacterBase::PossessedBy(AController* NewController)
{
   Super::PossessedBy(NewController);
    // ... (原有的 AI 和行为树启动逻辑不变) ...
     AAIController* AIController = Cast<AAIController>(NewController);
        if (AIController)
        {
            UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();

            if (BehaviorTree && BehaviorTree->BlackboardAsset)
            {
                 if(BlackboardComp)
                 {
                     BlackboardComp->InitializeBlackboard(*(BehaviorTree->BlackboardAsset));
                 }
                 else
                 {
                      bool bSuccess = AIController->UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp);
                      if(!bSuccess || !BlackboardComp)
                      {
                          UE_LOG(LogTemp, Error, TEXT("EnemyCharacterBase (%s): Failed to Use or Initialize Blackboard!"), *GetName());
                          return;
                      }
                 }
                 // 设置 SelfActor 键
                 BlackboardComp->SetValueAsObject(FName("SelfActor"), this);
                  // 启动行为树
                 bool bRunSuccess = AIController->RunBehaviorTree(BehaviorTree);
                if(!bRunSuccess) {
                   UE_LOG(LogTemp, Error, TEXT("EnemyCharacterBase (%s): Failed to run Behavior Tree '%s'!"), *GetName(), *BehaviorTree->GetName());
                } else {
                    UE_LOG(LogTemp, Log, TEXT("EnemyCharacterBase (%s): Possessed by AIController (%s), running Behavior Tree '%s'."),
                        *GetName(), *AIController->GetName(), *BehaviorTree->GetName());
                }

            }
             else
             {
                 UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase (%s): AIController possessed, but BehaviorTree or BlackboardAsset is not set!"), *GetName());
             }
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase (%s): Possessed by a non-AI Controller."), *GetName());
        }

}


// --- 接口实现 ---

// IDamageable (修改: 调用新的 GetStateAnimListener)
float AEnemyCharacterBase::ApplyDamage_Implementation(float DamageAmount, AActor* DamageCauser, AController* InstigatorController, const FHitResult& HitResult)
{
    if (!HealthComponent) return 0.f;

    float ActualDamage = HealthComponent->TakeDamage(DamageAmount, DamageCauser, InstigatorController);

    if (ActualDamage > 0.f && !HealthComponent->IsDead()) {
        // --- 修改: 获取 State Listener ---
        TScriptInterface<IEnemyStateAnimListener> StateListener = GetStateAnimListener(); // 调用 Provider 获取接口
        if (StateListener) { // 检查接口有效性
            FVector HitDirection = FVector::ZeroVector;
            if (DamageCauser) {
                HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
            } else if (HitResult.IsValidBlockingHit()) {
                 HitDirection = -HitResult.ImpactNormal;
             }
            bool bShouldInterrupt = true; // TODO: 添加打断逻辑判断

            StateListener->Execute_OnTakeHit(StateListener.GetObject(), ActualDamage, HitDirection, bShouldInterrupt); // 通过接口调用
             // UE_LOG(LogTemp, Verbose, ...);

            // (可选) AI 打断逻辑
            // ...
        } else {
            // UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase (%s): No valid IEnemyStateAnimListener found."), *GetName());
        }
    }
    return ActualDamage;
}


// --- 新: 实现 IEnemySpecificAnimListenerProvider 的 Getter 函数 ---
// 返回尝试转换基础动画实例到目标接口的结果
TScriptInterface<IEnemyMovementAnimListener> AEnemyCharacterBase::GetMovementAnimListener_Implementation() const {
    return TScriptInterface<IEnemyMovementAnimListener>(CachedAnimInstancePtr.Get());
}

TScriptInterface<IEnemyStateAnimListener> AEnemyCharacterBase::GetStateAnimListener_Implementation() const {
    return TScriptInterface<IEnemyStateAnimListener>(CachedAnimInstancePtr.Get());
}

TScriptInterface<IEnemyMeleeAttackAnimListener> AEnemyCharacterBase::GetMeleeAttackAnimListener_Implementation() const {
    // 基类默认实现：尝试转换。如果子类的 AnimInstance 实现了接口，这里就能返回有效的。
    return TScriptInterface<IEnemyMeleeAttackAnimListener>(CachedAnimInstancePtr.Get());
}

TScriptInterface<IEnemyRangedAttackAnimListener> AEnemyCharacterBase::GetRangedAttackAnimListener_Implementation() const {
    return TScriptInterface<IEnemyRangedAttackAnimListener>(CachedAnimInstancePtr.Get());
}

TScriptInterface<IEnemyTeleportAnimListener> AEnemyCharacterBase::GetTeleportAnimListener_Implementation() const {
    return TScriptInterface<IEnemyTeleportAnimListener>(CachedAnimInstancePtr.Get());
}
// --- Provider Getter 实现结束 ---


// IFacingDirectionProvider (保留)
FVector AEnemyCharacterBase::GetFacingDirection_Implementation() const {
    return bIsFacingRight ? FVector::ForwardVector : -FVector::ForwardVector; // 简化为直接用 +/- Forward
}


// --- 内部逻辑函数 ---

// HandleDeath (修改: 调用新的 GetStateAnimListener)
void AEnemyCharacterBase::HandleDeath(AActor* Killer) {
    UE_LOG(LogTemp, Log, TEXT("EnemyCharacterBase (%s): HandleDeath."), *GetName());
    // ... (停止 AI, 禁用物理等逻辑不变) ...
     AController* MyController = GetController();
        if(MyController)
        {
            MyController->StopMovement();
            AAIController* AIController = Cast<AAIController>(MyController);
            if (AIController && AIController->GetBrainComponent())
            {
                AIController->GetBrainComponent()->StopLogic("Character Died");
            }
        }
        SetActorEnableCollision(false);
        if(GetCharacterMovement())
        {
            GetCharacterMovement()->StopMovementImmediately();
            GetCharacterMovement()->DisableMovement();
            GetCharacterMovement()->SetComponentTickEnabled(false);
        }


    // --- 修改: 获取 State Listener ---
    TScriptInterface<IEnemyStateAnimListener> StateListener = GetStateAnimListener(); // 调用 Provider 获取接口
    if (StateListener) { // 检查接口有效性
        StateListener->Execute_OnDeathState(StateListener.GetObject(), Killer); // 通过接口调用
         // UE_LOG(LogTemp, Verbose, ...);
    } else {
         UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase (%s): No valid IEnemyStateAnimListener found during HandleDeath."), *GetName());
     }

    SetLifeSpan(5.0f); // 保留设置销毁时间
}

// SetFacingDirection (保留)
void AEnemyCharacterBase::SetFacingDirection(bool bFaceRight) {
    if (bIsFacingRight == bFaceRight) return;
    bIsFacingRight = bFaceRight;
    if (UPaperFlipbookComponent* flipbookcmp = GetSprite()) {
        FVector CurrentScale = flipbookcmp->GetRelativeScale3D();
        CurrentScale.X = bIsFacingRight ? FMath::Abs(CurrentScale.X) : -FMath::Abs(CurrentScale.X);
        flipbookcmp->SetRelativeScale3D(CurrentScale);
    }
}


// CacheBaseAnimInstance (修改: 缓存基础实例)
void AEnemyCharacterBase::CacheBaseAnimInstance() { // <-- 重命名并修改逻辑
    if (UPaperZDAnimationComponent* AnimComp = GetAnimationComponent()) {
        CachedAnimInstancePtr = AnimComp->GetAnimInstance(); // <-- 缓存基础指针
        if (!CachedAnimInstancePtr.IsValid()) {
            UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase '%s': Failed to get AnimInstance."), *GetName());
        } else {
             UE_LOG(LogTemp, Log, TEXT("EnemyCharacterBase '%s': Cached AnimInstance '%s'."), *GetName(), *CachedAnimInstancePtr->GetName());
        }
    } else {
        UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase '%s': PaperZDAnimationComponent not found."), *GetName());
    }
}


// GetEnemyAnimInstance (保留，但其用处可能减少)
UEnemyAnimInstanceBase* AEnemyCharacterBase::GetEnemyAnimInstance() const {
     // 注意：这里仍然尝试转换为 UEnemyAnimInstanceBase。
     // 如果你创建了很多子类 AnimInstance，这个函数可能需要调整或弃用，
     // 除非 UEnemyAnimInstanceBase 仍然包含你需要直接访问的公共函数或属性。
    return Cast<UEnemyAnimInstanceBase>(CachedAnimInstancePtr.Get());
}