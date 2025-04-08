// My2DGameDesign/Private/Enemies/EnemyCharacterBase.cpp

#include "Enemies/EnemyCharacterBase.h"       // 引入对应的头文件
#include "Components/HealthComponent.h"      // 引入生命组件头文件
#include "PaperFlipbookComponent.h"          // 用于获取和设置 Sprite
#include "PaperZDAnimationComponent.h"       // 用于获取动画组件
#include "PaperZDAnimInstance.h"           // PaperZD 动画实例基类
#include "AIController.h"                  // AI 控制器基类
#include "BrainComponent.h"                // AI 大脑组件，用于停止行为树
#include "GameFramework/CharacterMovementComponent.h" // 角色移动组件
#include "BehaviorTree/BehaviorTree.h"     // 行为树类
#include "BehaviorTree/BlackboardComponent.h" // 黑板组件
// 引入我们将要创建的动画实例基类头文件 (虽然文件还没创建，先包含进来)
#include "AniInstance/EnemyAnimInstanceBase.h"
// 引入动画状态监听器接口头文件
#include "Interfaces/CharacterAnimationStateListener.h"


// 构造函数
AEnemyCharacterBase::AEnemyCharacterBase()
{
	// 创建生命组件实例，并将其附加到这个 Actor
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // --- AI 控制设置 ---
    // 设置默认的 AI 控制器类，如果子类没有指定，则使用这个
    // 注意：这里通常设置一个基类，具体的 AIController 可能在子类或蓝图中指定
    AIControllerClass = AAIController::StaticClass();
    // 设置 AI 控制器何时自动控制此 Pawn：在关卡中放置或动态生成时
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // 初始化朝向状态
    bIsFacingRight = true; // 游戏开始时默认朝右
}

// BeginPlay: 游戏开始时的初始化
void AEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay(); // 调用父类的 BeginPlay

	// 尝试查找并缓存动画监听器接口
    CacheAnimationStateListener();

    // 绑定 HealthComponent 的 OnDeath 委托到我们的 HandleDeath 函数
    if(HealthComponent)
    {
        // 当 HealthComponent 广播 OnDeath 事件时，会自动调用本类的 HandleDeath 函数
        HealthComponent->OnDeath.AddDynamic(this, &AEnemyCharacterBase::HandleDeath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyCharacterBase '%s' has no valid HealthComponent!"), *GetName());
    }
}

// PossessedBy: 当被 AI 控制器控制时
void AEnemyCharacterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // 尝试将新的控制器转换为 AI 控制器
    AAIController* AIController = Cast<AAIController>(NewController);
    if (AIController)
    {
        // 获取 AI 控制器关联的黑板组件
        UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent(); // 或者 AIController->UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp);

        // 检查行为树资产是否已经设置（通常在子类或蓝图默认值中设置）
        if (BehaviorTree && BehaviorTree->BlackboardAsset)
        {
            // 使用行为树的黑板资产来初始化黑板组件
            // 确保 AI 控制器使用的黑板与行为树期望的黑板是同一个
             if(BlackboardComp)
             {
                 BlackboardComp->InitializeBlackboard(*(BehaviorTree->BlackboardAsset));
             }
             else // 如果控制器还没有黑板组件，尝试创建一个并初始化
             {
                  bool bSuccess = AIController->UseBlackboard(BehaviorTree->BlackboardAsset, BlackboardComp);
                  if(!bSuccess || !BlackboardComp)
                  {
                      UE_LOG(LogTemp, Error, TEXT("EnemyCharacterBase (%s): Failed to Use or Initialize Blackboard!"), *GetName());
                      return; // 没有黑板无法运行行为树
                  }
             }

             // (可选) 在黑板上设置一些初始值
             // 设置 SelfActor 键，让行为树知道它自己是谁
             BlackboardComp->SetValueAsObject(FName("SelfActor"), this);
             UE_LOG(LogTemp, Verbose, TEXT("EnemyCharacterBase (%s): Set 'SelfActor' on Blackboard."), *GetName());

            // 启动行为树执行
            bool bRunSuccess = AIController->RunBehaviorTree(BehaviorTree);
            if(bRunSuccess)
            {
                 UE_LOG(LogTemp, Log, TEXT("EnemyCharacterBase (%s): Possessed by AIController (%s), successfully running Behavior Tree '%s'."),
                    *GetName(), *AIController->GetName(), *BehaviorTree->GetName());
            }
            else
            {
                 UE_LOG(LogTemp, Error, TEXT("EnemyCharacterBase (%s): Failed to run Behavior Tree '%s'!"), *GetName(), *BehaviorTree->GetName());
            }
        }
         else
         {
             UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase (%s): AIController (%s) possessed, but BehaviorTree or BlackboardAsset is not set on the character! AI will not run."),
                *GetName(), *AIController->GetName());
         }
    }
    else // 如果控制者不是 AIController
    {
         UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase (%s): Possessed by a non-AI Controller (%s)."),
             *GetName(), NewController ? *NewController->GetName() : TEXT("None"));
    }
}


// --- 接口实现 ---

// IDamageable 接口实现：直接将伤害请求转发给 HealthComponent 处理
float AEnemyCharacterBase::ApplyDamage_Implementation(float DamageAmount, AActor* DamageCauser, AController* InstigatorController, const FHitResult& HitResult)
{
	if(HealthComponent)
    {
       // 调用 HealthComponent 的 TakeDamage 函数来实际处理伤害逻辑
       return HealthComponent->TakeDamage(DamageAmount, DamageCauser, InstigatorController);
    }
    UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase (%s): ApplyDamage called but HealthComponent is invalid!"), *GetName());
    return 0.f; // 如果没有生命组件，则不受伤害
}

// IAnimationStateProvider 接口实现：返回缓存的动画监听器接口
TScriptInterface<ICharacterAnimationStateListener> AEnemyCharacterBase::GetAnimStateListener_Implementation() const
{
	// 直接返回在 BeginPlay 或 Cache 函数中缓存的接口指针
    return AnimationStateListener;
}

// IFacingDirectionProvider 接口实现：根据内部状态返回朝向向量
FVector AEnemyCharacterBase::GetFacingDirection_Implementation() const
{
    // 根据 bIsFacingRight 状态返回世界空间中的前方向量或后方向量
	return bIsFacingRight ? GetActorForwardVector() : -GetActorForwardVector();
    // 注意：这里直接用了 Actor 的 ForwardVector。对于 2D PaperZD，可能 GetActorForwardVector() 总是 (1,0,0)。
    // 更好的方式可能是直接返回 FVector(1,0,0) 或 FVector(-1,0,0)
    // return bIsFacingRight ? FVector::ForwardVector : -FVector::ForwardVector;
}


// --- 内部逻辑函数 ---

// HandleDeath: 处理死亡逻辑 (由 HealthComponent 的 OnDeath 委托调用)
void AEnemyCharacterBase::HandleDeath(AActor* Killer)
{
    UE_LOG(LogTemp, Log, TEXT("EnemyCharacterBase (%s): HandleDeath is executing."), *GetName());

    // 1. 停止 AI 逻辑
    AController* MyController = GetController();
    if(MyController)
    {
        MyController->StopMovement(); // 停止任何当前移动命令
        AAIController* AIController = Cast<AAIController>(MyController);
        if (AIController && AIController->GetBrainComponent()) // 检查 BrainComponent 是否有效
        {
            AIController->GetBrainComponent()->StopLogic("Character Died"); // 停止行为树等逻辑
            UE_LOG(LogTemp, Verbose, TEXT("EnemyCharacterBase (%s): AI Brain logic stopped."), *GetName());
        }
        // AI 控制器可能需要解除对 Pawn 的控制
        // AIController->UnPossess(); // 可以考虑解除控制
    }

    // 2. 禁用物理和移动
    SetActorEnableCollision(false); // 禁用 Actor 的主碰撞
    if(GetCharacterMovement())
    {
        GetCharacterMovement()->StopMovementImmediately(); // 立即停止移动
        GetCharacterMovement()->DisableMovement();         // 禁用移动能力
        GetCharacterMovement()->SetComponentTickEnabled(false); // 停止移动组件的 Tick
    }
     UE_LOG(LogTemp, Verbose, TEXT("EnemyCharacterBase (%s): Collision and Movement component disabled."), *GetName());

    // 3. 通知动画实例播放死亡动画
    // 我们需要确保 AnimationStateListener 是有效的，并且实现了我们期望的死亡通知函数
    // 假设我们之后会在 IEnemyAnimationStateListener 中定义 OnDeathState
    if (AnimationStateListener)
    {
        // 强转 TScriptInterface 到具体接口类型来调用（更安全的方式）
        IEnemyAnimationStateListener* Listener = Cast<IEnemyAnimationStateListener>(AnimationStateListener.GetObject());
        if(Listener)
        {
             // 调用死亡状态接口函数
             Listener->Execute_OnDeathState(AnimationStateListener.GetObject(), Killer); // 使用 Execute_ 前缀调用接口函数更安全
             UE_LOG(LogTemp, Verbose, TEXT("EnemyCharacterBase (%s): Notified Animation Listener about death."), *GetName());
        }
         else
         {
              UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase (%s): AnimationStateListener does not implement IEnemyAnimationStateListener!"), *GetName());
         }
    }
     else
     {
         UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase (%s): No valid AnimationStateListener found during HandleDeath."), *GetName());
     }


    // 4. 设置 Actor 在一段时间后自动销毁
    SetLifeSpan(5.0f); // 例如，5秒后自动从关卡中移除
    UE_LOG(LogTemp, Verbose, TEXT("EnemyCharacterBase (%s): Set lifespan to 5 seconds for automatic cleanup."), *GetName());

    // 注意：这里的死亡处理比较通用。具体的死亡效果（如粒子、声音、碎裂）
    // 通常在动画通知 (AnimNotify) 或者监听 OnDeath 委托的其他系统中处理。
}

// SetFacingDirection: 设置视觉朝向 (翻转 Sprite)
void AEnemyCharacterBase::SetFacingDirection(bool bFaceRight)
{
    // 如果方向没有改变，则不做任何事
    if (bIsFacingRight == bFaceRight) return;

    bIsFacingRight = bFaceRight; // 更新内部状态
    if (UPaperFlipbookComponent* flipbookcmp = GetSprite()) // 获取 PaperZD 的 Sprite 组件
    {
        FVector CurrentScale = flipbookcmp->GetRelativeScale3D();
        // 直接设置 Scale.X 为 1.0 或 -1.0 来控制朝向
        CurrentScale.X = bIsFacingRight ? FMath::Abs(CurrentScale.X) : -FMath::Abs(CurrentScale.X);
        flipbookcmp->SetRelativeScale3D(CurrentScale); // 应用新的缩放值
    }
     // 调试日志，确认方向设置
     // UE_LOG(LogTemp, Verbose, TEXT("EnemyCharacterBase (%s): SetFacingDirection called. Now facing %s"), *GetName(), bFaceRight ? TEXT("Right") : TEXT("Left"));
}


// CacheAnimationStateListener: 查找并缓存动画监听器接口
void AEnemyCharacterBase::CacheAnimationStateListener()
{
    // 获取 PaperZD 动画组件
	if (UPaperZDAnimationComponent* AnimComp = GetAnimationComponent())
	{
        // 从动画组件获取当前的动画实例
		UPaperZDAnimInstance* BaseAnimInstance = AnimComp->GetAnimInstance();
		if (BaseAnimInstance)
		{
            // 尝试将动画实例转换为我们期望的监听器接口 (这里暂时复用英雄的)
            AnimationStateListener = TScriptInterface<ICharacterAnimationStateListener>(BaseAnimInstance);
            // 检查转换是否成功，以及获取到的对象是否有效
            if (AnimationStateListener.GetInterface() && AnimationStateListener.GetObject())
            {
                UE_LOG(LogTemp, Log, TEXT("EnemyCharacterBase '%s': Successfully cached AnimationStateListener on AnimInstance '%s'."),
                       *GetName(), *BaseAnimInstance->GetName());
            }
            else
            {
                 AnimationStateListener = nullptr; // 如果转换失败，确保接口指针为空
                 UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase '%s': AnimInstance '%s' does NOT implement the required Animation State Listener interface (ICharacterAnimationStateListener/IEnemyAnimationStateListener)."),
                       *GetName(), *BaseAnimInstance->GetName());
                 // 你可能需要确保你的 UEnemyAnimInstanceBase 实现了这个接口
            }
		}
		else
        {
             UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase '%s': Failed to get AnimInstance from AnimationComponent."), *GetName());
        }
	}
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("EnemyCharacterBase '%s': PaperZDAnimationComponent not found."), *GetName());
    }
}

// GetEnemyAnimInstance: 获取特定类型的动画实例
UEnemyAnimInstanceBase* AEnemyCharacterBase::GetEnemyAnimInstance() const
{
    if (UPaperZDAnimationComponent* AnimComp = GetAnimationComponent())
	{
        // 尝试将获取到的动画实例转换为我们期望的 UEnemyAnimInstanceBase 类型
		return Cast<UEnemyAnimInstanceBase>(AnimComp->GetAnimInstance());
    }
    return nullptr; // 如果无法获取或转换失败，返回空指针
}