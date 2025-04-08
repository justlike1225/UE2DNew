// My2DGameDesign/Private/AI/EnemyAIControllerBase.cpp

#include "AI/EnemyAIControllerBase.h"    // 引入对应的头文件
#include "BehaviorTree/BehaviorTreeComponent.h" // 行为树组件
#include "BehaviorTree/BlackboardComponent.h" // 黑板组件
#include "Perception/AIPerceptionComponent.h" // 感知组件
#include "Perception/AISenseConfig_Sight.h"   // 视觉感知配置
#include "BehaviorTree/BehaviorTree.h"       // 行为树资产
#include "BehaviorTree/BlackboardData.h"     // 黑板资产
#include "Enemies/EnemyCharacterBase.h"      // 需要获取敌人基类以访问其 BehaviorTree 属性
#include "GameFramework/Pawn.h"              // 需要 Pawn 类型
#include "BrainComponent.h"                  // 需要停止 AI 逻辑


// 定义黑板键名常量 (确保与 .h 文件中的声明一致)
const FName AEnemyAIControllerBase::TargetActorKeyName = FName("TargetActor");
const FName AEnemyAIControllerBase::CanSeeTargetKeyName = FName("CanSeePlayer"); // 保持与之前设计一致
const FName AEnemyAIControllerBase::SelfActorKeyName = FName("SelfActor");


// 构造函数
AEnemyAIControllerBase::AEnemyAIControllerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) // 调用父类构造函数
{
	// --- 创建 AI 核心组件 ---
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

	// --- 配置视觉感知 (Sight Sense) ---
    // 创建一个视觉感知配置对象
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
        // **重要**: 这些数值需要根据你的游戏进行调整!**
		SightConfig->SightRadius = 1200.0f; // 能看到多远
		SightConfig->LoseSightRadius = 1600.0f; // 丢失视觉需要多远 (通常比 SightRadius 稍大，防止目标在边缘闪烁)
		SightConfig->PeripheralVisionAngleDegrees = 75.0f; // 视野有多宽 (角度)
		SightConfig->SetMaxAge(5.0f); // 感知信息保留多久 (秒)，如果5秒没再看到，信息就过期了

        // 设置能探测到的目标类型 (基于阵营 Team Attitude)
		SightConfig->DetectionByAffiliation.bDetectEnemies = true; // 能探测标记为敌人的 Actor
		SightConfig->DetectionByAffiliation.bDetectNeutrals = false; // 不探测中立 Actor
		SightConfig->DetectionByAffiliation.bDetectFriendlies = false; // 不探测友方 Actor
        // 注意：你需要设置 Actor 的阵营 (Team ID) 才能让这个生效。
        // 可以通过实现 IGenericTeamAgentInterface 接口来设置。

        // 将配置好的视觉感知添加到 AIPerceptionComponent 中
		AIPerceptionComponent->ConfigureSense(*SightConfig);
        // 设置视觉为主要的感知方式 (如果还有听觉等其他感知，需要指定哪个是主要的)
		AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

        UE_LOG(LogTemp, Log, TEXT("EnemyAIControllerBase: Sight sense configured."));
	}
     else
     {
         UE_LOG(LogTemp, Error, TEXT("EnemyAIControllerBase: Failed to create SightConfig for AIPerceptionComponent!"));
     }

	// --- 绑定感知更新事件 ---
    // 当 AIPerceptionComponent 的感知信息更新时 (看到或丢失目标)，调用本类的 HandleTargetPerceptionUpdated 函数
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIControllerBase::HandleTargetPerceptionUpdated);
}

// OnPossess: 当控制器控制一个 Pawn 时调用
void AEnemyAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn); // 调用父类逻辑

    UE_LOG(LogTemp, Log, TEXT("EnemyAIControllerBase (%s): Possessing Pawn (%s)."), *GetName(), InPawn ? *InPawn->GetName() : TEXT("None"));

	// 尝试将被控制的 Pawn 转换为我们的敌人基类
	AEnemyCharacterBase* EnemyCharacter = Cast<AEnemyCharacterBase>(InPawn);
	if (!EnemyCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyAIControllerBase::OnPossess - Failed to cast Possessed Pawn to AEnemyCharacterBase!"));
		return;
	}

    // 从被控制的 EnemyCharacter 获取它配置的行为树资产
    EnemyBehaviorTree = EnemyCharacter->BehaviorTree; // 假设 AEnemyCharacterBase 有 TObjectPtr<UBehaviorTree> BehaviorTree; 属性

    // 检查行为树和它关联的黑板资产是否都有效
	if (EnemyBehaviorTree && EnemyBehaviorTree->BlackboardAsset)
	{
		// 尝试使用敌人指定的黑板资产来初始化此控制器的黑板组件
		UBlackboardComponent* BBComp = BlackboardComponent.Get(); // 先从 TObjectPtr 获取原始指针
		bool bInitialized = UseBlackboard(EnemyBehaviorTree->BlackboardAsset, BBComp); // 将原始指针的引用传递给 UseBlackboard

		// UseBlackboard 可能会改变 BBComp 指向的实例 (如果原来是 nullptr，它会创建一个新的)
		// 所以我们需要用可能更新过的 BBComp 来更新我们的成员变量 BlackboardComponent
		BlackboardComponent = BBComp; // <--- 更新 TObjectPtr 成员

        if(bInitialized && BlackboardComponent) // 确保初始化成功且黑板组件有效
        {
            UE_LOG(LogTemp, Log, TEXT("EnemyAIControllerBase: Blackboard '%s' initialized successfully."), *EnemyBehaviorTree->BlackboardAsset->GetName());

            // 在黑板上设置 SelfActor 键的值为被控制的 Pawn
            BlackboardComponent->SetValueAsObject(SelfActorKeyName, InPawn);
             UE_LOG(LogTemp, Verbose, TEXT("EnemyAIControllerBase: Set '%s' key on Blackboard."), *SelfActorKeyName.ToString());

            // 启动行为树！这是AI开始执行逻辑的关键步骤
           BehaviorTreeComponent->StartTree(*EnemyBehaviorTree);
           

        }
        else // 黑板初始化失败
        {
             UE_LOG(LogTemp, Error, TEXT("EnemyAIControllerBase: Failed to initialize or use Blackboard asset '%s'! Behavior Tree cannot run."),
                 *EnemyBehaviorTree->BlackboardAsset->GetName());
        }
	}
    else // 没有找到有效的行为树或黑板资产
    {
        if (!EnemyBehaviorTree) { UE_LOG(LogTemp, Warning, TEXT("EnemyAIControllerBase: BehaviorTree is not set on the possessed Pawn '%s'!"), *InPawn->GetName()); }
        if (EnemyBehaviorTree && !EnemyBehaviorTree->BlackboardAsset) { UE_LOG(LogTemp, Warning, TEXT("EnemyAIControllerBase: BehaviorTree '%s' is missing a BlackboardAsset!"), *EnemyBehaviorTree->GetName()); }
        UE_LOG(LogTemp, Warning, TEXT("EnemyAIControllerBase: AI will not run because Behavior Tree or Blackboard is missing."));
    }
}

// OnUnPossess: 当控制器不再控制 Pawn 时调用
void AEnemyAIControllerBase::OnUnPossess()
{
    UE_LOG(LogTemp, Log, TEXT("EnemyAIControllerBase (%s): UnPossessing."), *GetName());
    // 安全地停止行为树逻辑
    if(BehaviorTreeComponent)
    {
        // 检查行为树是否正在运行，如果是，则停止它
        if(GetBrainComponent() && BehaviorTreeComponent->IsRunning())
        {
	        BehaviorTreeComponent->StopTree(EBTStopMode::Safe); // Safe 模式会尝试完成当前任务再停止
            UE_LOG(LogTemp, Verbose, TEXT("EnemyAIControllerBase: Behavior Tree stopped."), *GetName());
        }
    }

	Super::OnUnPossess(); // 调用父类逻辑
}

// HandleTargetPerceptionUpdated: 处理感知更新事件
void AEnemyAIControllerBase::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    // 尝试将被感知的 Actor 转换为 Pawn (因为我们通常关心的是角色)
    APawn* SensedPawn = Cast<APawn>(Actor);
    if(!SensedPawn) return; // 如果不是 Pawn，我们不关心

    // 可以在这里添加更复杂的逻辑来判断 SensedPawn 是否是我们关心的目标
    // 例如，检查 Team ID，或者检查是否是玩家角色
    bool bIsConsideredTarget = SensedPawn->IsPlayerControlled(); // 简单示例：只关心玩家控制的 Pawn

    if(bIsConsideredTarget)
    {
        if (Stimulus.WasSuccessfullySensed()) // 是"看到"或"听到"等成功感知
        {
            UE_LOG(LogTemp, Verbose, TEXT("EnemyAIControllerBase (%s): Perception Updated - Sensed target '%s' successfully."), *GetName(), *Actor->GetName());
            SetTargetActorOnBlackboard(Actor); // 在黑板上设置目标
            BlackboardComponent->SetValueAsBool(CanSeeTargetKeyName, true); // 更新黑板状态：能看到目标
        }
        else // 是"丢失"感知 (例如目标移出视野范围或信息过期)
        {
             UE_LOG(LogTemp, Verbose, TEXT("EnemyAIControllerBase (%s): Perception Updated - Lost stimulus for target '%s'."), *GetName(), *Actor->GetName());
             // 检查黑板上当前的目标是否就是刚刚丢失感知的这个 Actor
             AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKeyName));
             if (CurrentTarget == Actor)
             {
                 UE_LOG(LogTemp, Log, TEXT("EnemyAIControllerBase (%s): Lost sight of current target '%s'. Clearing target on blackboard."), *GetName(), *Actor->GetName());
                 SetTargetActorOnBlackboard(nullptr); // 清除黑板上的目标
                 BlackboardComponent->SetValueAsBool(CanSeeTargetKeyName, false); // 更新黑板状态：看不到目标
             }
             // else: 如果丢失的不是当前锁定的目标，则可能忽略（避免因短暂遮挡就丢失目标）
        }
    }
    // else: 感知到的不是我们关心的目标类型，忽略
}

// SetTargetActorOnBlackboard: 在黑板上设置目标 Actor
void AEnemyAIControllerBase::SetTargetActorOnBlackboard(AActor* TargetActor)
{
    if(BlackboardComponent) // 确保黑板组件有效
    {
        if(TargetActor) // 如果传入了有效的 Actor
        {
            // 设置 TargetActorKeyName 对应的值
            BlackboardComponent->SetValueAsObject(TargetActorKeyName, TargetActor);
            // UE_LOG(LogTemp, Verbose, TEXT("EnemyAIControllerBase: Set '%s' on blackboard to '%s'."), *TargetActorKeyName.ToString(), *TargetActor->GetName());
        }
        else // 如果传入的是 nullptr
        {
            // 清除 TargetActorKeyName 对应的值
            BlackboardComponent->ClearValue(TargetActorKeyName);
            // UE_LOG(LogTemp, Verbose, TEXT("EnemyAIControllerBase: Cleared '%s' on blackboard."), *TargetActorKeyName.ToString());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EnemyAIControllerBase::SetTargetActorOnBlackboard - BlackboardComponent is invalid!"));
    }
}