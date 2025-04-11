// My2DGameDesign/Private/AI/AIC/EnemyAIControllerBase.cpp

#include "AI/AIC/EnemyAIControllerBase.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h" // 需要包含这个以使用 UAISense_Sight::StaticClass()
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Enemies/EnemyCharacterBase.h"       // 确保包含 EnemyCharacterBase
#include "DataAssets/Enemy/EnemyAISettingsDA.h" // 确保包含我们创建的数据资产
#include "GameFramework/Pawn.h"
#include "BrainComponent.h"                  // 确保包含 BrainComponent

// 黑板 Key 名称定义 (保持不变)
const FName AEnemyAIControllerBase::TargetActorKeyName = FName("TargetActor");
const FName AEnemyAIControllerBase::CanSeeTargetKeyName = FName("CanSeePlayer");
const FName AEnemyAIControllerBase::SelfActorKeyName = FName("SelfActor");

// 构造函数
AEnemyAIControllerBase::AEnemyAIControllerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 创建组件 (保持不变)
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

	// 创建视觉配置对象
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{

		// 将这个（尚未完全配置的）感知配置添加到感知组件中
		// 后续会在 OnPossess 中根据数据资产填充具体数值
		AIPerceptionComponent->ConfigureSense(*SightConfig);
		AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AEnemyAIControllerBase Constructor: Failed to create SightConfig!"));
	}

	// 绑定感知更新事件 (保持不变)
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIControllerBase::HandleTargetPerceptionUpdated);
}


// 当控制器附身到一个 Pawn 时调用
void AEnemyAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 尝试将附身的 Pawn 转换为我们的敌人基类
	AEnemyCharacterBase* EnemyCharacter = Cast<AEnemyCharacterBase>(InPawn);
	if (!EnemyCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("AEnemyAIControllerBase::OnPossess: Possessed Pawn is not an AEnemyCharacterBase for %s!"), *GetNameSafe(InPawn));
		return;
	}

	// --- 动态配置感知组件 ---
	UEnemyAISettingsDA* Settings = EnemyCharacter->AISettings; // 从 Pawn 获取 AI 设置数据资产
	if (Settings && AIPerceptionComponent)
	{
		// 尝试获取之前添加到感知组件中的视觉配置对象
		UAISenseConfig_Sight* SightConfig = AIPerceptionComponent->GetSenseConfig<UAISenseConfig_Sight>();
		if (SightConfig)
		{
			UE_LOG(LogTemp, Log, TEXT("AEnemyAIControllerBase::OnPossess: Applying AISettings DA '%s' to SightConfig for %s"), *GetNameSafe(Settings), *InPawn->GetName());

			// 使用数据资产中的值来配置 SightConfig
			SightConfig->SightRadius = Settings->SightRadius;
			SightConfig->LoseSightRadius = Settings->LoseSightRadius;
			SightConfig->PeripheralVisionAngleDegrees = Settings->PeripheralVisionAngleDegrees;
			SightConfig->SetMaxAge(Settings->SightMaxAge);
			SightConfig->DetectionByAffiliation = Settings->DetectionByAffiliation;

			// 请求感知系统更新其监听器，以应用新的配置
			// 注意：在某些引擎版本或复杂情况下，可能需要更强制的更新，
			// 但 RequestStimuliListenerUpdate 通常足够了。
			AIPerceptionComponent->RequestStimuliListenerUpdate();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AEnemyAIControllerBase::OnPossess: Could not retrieve SightConfig from PerceptionComponent for %s."), *InPawn->GetName());
		}
	}
	else
	{
		if (!Settings)
		{
			UE_LOG(LogTemp, Warning, TEXT("AEnemyAIControllerBase::OnPossess: AISettings DataAsset is not set on Pawn %s."), *InPawn->GetName());
		}
		if (!AIPerceptionComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("AEnemyAIControllerBase::OnPossess: AIPerceptionComponent is missing on %s."), *GetName());
		}
		
	}
	
	EnemyBehaviorTree = EnemyCharacter->BehaviorTree; // 从 Pawn 获取行为树资源

	if (EnemyBehaviorTree && EnemyBehaviorTree->BlackboardAsset)
	{
		// 获取黑板组件引用
		UBlackboardComponent* BBComp = BlackboardComponent.Get();

		// 使用 UseBlackboard 来初始化黑板，如果需要，它会创建一个新的组件实例
		// 并且返回 true 如果成功。它比 InitializeBlackboard 更常用。
		bool bInitialized = UseBlackboard(EnemyBehaviorTree->BlackboardAsset, BBComp);

		// UseBlackboard 可能会重新分配 BBComp，所以更新我们的成员变量指针
		BlackboardComponent = BBComp;

		if (bInitialized && BlackboardComponent)
		{
			// 初始化黑板值
			BlackboardComponent->SetValueAsObject(SelfActorKeyName, InPawn);
			// 可以设置其他初始值...

			// 启动行为树
			UE_LOG(LogTemp, Log, TEXT("AEnemyAIControllerBase::OnPossess: Starting Behavior Tree '%s' for %s"), *GetNameSafe(EnemyBehaviorTree), *InPawn->GetName());
			BehaviorTreeComponent->StartTree(*EnemyBehaviorTree);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AEnemyAIControllerBase::OnPossess: Failed to initialize Blackboard or BlackboardComponent is null for %s"), *InPawn->GetName());
		}
	}
	else
	{
		if (!EnemyBehaviorTree)
		{
			UE_LOG(LogTemp, Warning, TEXT("AEnemyAIControllerBase::OnPossess: BehaviorTree is not set on Pawn %s."), *InPawn->GetName());
		}
		if (EnemyBehaviorTree && !EnemyBehaviorTree->BlackboardAsset)
		{
			UE_LOG(LogTemp, Warning, TEXT("AEnemyAIControllerBase::OnPossess: BehaviorTree on Pawn %s is missing BlackboardAsset."), *InPawn->GetName());
		}
		// 没有有效的行为树或黑板，AI 将不会执行 BT 逻辑
	}
}


// 当控制器不再附身 Pawn 时调用
void AEnemyAIControllerBase::OnUnPossess()
{
	// 停止行为树 (安全停止模式)
	if (BehaviorTreeComponent)
	{
		// 检查 BrainComponent 是否存在且行为树正在运行
		UBrainComponent* BrainComp = GetBrainComponent();
		if (BrainComp && BehaviorTreeComponent->IsRunning())
		{
			UE_LOG(LogTemp, Log, TEXT("AEnemyAIControllerBase::OnUnPossess: Stopping Behavior Tree for %s"), *GetNameSafe(GetPawn()));
			BehaviorTreeComponent->StopTree(EBTStopMode::Safe);
		}
	}

	Super::OnUnPossess(); // 调用父类实现
}


// 处理感知更新事件 (基本保持不变)
void AEnemyAIControllerBase::HandleTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 确保有黑板
    if (!BlackboardComponent)
    {
        return;
    }

	// 尝试将感知的 Actor 转为 Pawn
	APawn* SensedPawn = Cast<APawn>(Actor);
	if (!SensedPawn) return; // 如果感知的不是 Pawn，可能忽略 (根据你的需求)
	
	bool bIsConsideredTarget = false;
	if (IGenericTeamAgentInterface* MyTeamAgent = Cast<IGenericTeamAgentInterface>(this)) // 或者 GetPawn()
	{
		bIsConsideredTarget = MyTeamAgent->GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile;
		UE_LOG(LogTemp, Verbose, TEXT("Perception Update: Attitude towards %s is %s"),
			*Actor->GetName(),
			(bIsConsideredTarget ? TEXT("Hostile") : TEXT("Not Hostile")));
	}
	else
	{
		// Fallback if team interface not implemented (less ideal)
		bIsConsideredTarget = SensedPawn->IsPlayerControlled();
	}


	if (bIsConsideredTarget)
	{
		if (Stimulus.WasSuccessfullySensed()) // 看到了或感知到了
		{
			UE_LOG(LogTemp, Log, TEXT("Perception Update: Target %s Sensed."), *Actor->GetName());
			SetTargetActorOnBlackboard(Actor); // 设置目标
			BlackboardComponent->SetValueAsBool(CanSeeTargetKeyName, true); // 标记为可见
		}
		else // 丢失了感知 (例如，目标离开了视野范围或感知过期)
		{
			UE_LOG(LogTemp, Log, TEXT("Perception Update: Target %s Lost."), *Actor->GetName());
			// 检查当前黑板上的目标是否就是刚刚丢失感知的这个 Actor
			AActor* CurrentTarget = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKeyName));
			if (CurrentTarget == Actor)
			{
				// 清除目标和可见标记
				SetTargetActorOnBlackboard(nullptr);
				BlackboardComponent->SetValueAsBool(CanSeeTargetKeyName, false);
				UE_LOG(LogTemp, Log, TEXT("Perception Update: Cleared current target from Blackboard."));
			}
             else
             {
                 UE_LOG(LogTemp, Log, TEXT("Perception Update: Lost sight of %s, but it wasn't the current target (%s)."), *Actor->GetName(), *GetNameSafe(CurrentTarget));
             }
		}
	}
     else
     {
         UE_LOG(LogTemp, Verbose, TEXT("Perception Update: Sensed Actor %s is not considered a target."), *Actor->GetName());
     }
}


// 设置或清除黑板上的目标 Actor (保持不变)
void AEnemyAIControllerBase::SetTargetActorOnBlackboard(AActor* TargetActor)
{
	if (BlackboardComponent)
	{
		if (TargetActor)
		{
			// 设置目标对象
			BlackboardComponent->SetValueAsObject(TargetActorKeyName, TargetActor);
		}
		else
		{
			// 清除目标对象
			BlackboardComponent->ClearValue(TargetActorKeyName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemyAIControllerBase::SetTargetActorOnBlackboard: BlackboardComponent is null!"));
	}
}

// 获取对其他 Actor 的阵营态度 (保持不变)
ETeamAttitude::Type AEnemyAIControllerBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	// 尝试将 Other Actor 转换为具有阵营接口的对象
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);
	if (OtherTeamAgent)
	{
		// 获取对方的 TeamId
		FGenericTeamId OtherTeamId = OtherTeamAgent->GetGenericTeamId();

		// 使用引擎内置的函数来判断态度 (需要设置 TeamId)
        // 或者你可以实现自己的逻辑
        // return FGenericTeamId::GetAttitude(GetGenericTeamId(), OtherTeamId); // 推荐方式

        // 手动判断示例 (如果 TeamId 只是简单数字)
		if (OtherTeamId == TeamId) // 假设 TeamId 在头文件中设置
		{
			return ETeamAttitude::Friendly;
		}
		else
		{
            // 这里可以更细化，比如区分中立和敌对
            // 假设除了自己队伍都是敌对
			return ETeamAttitude::Hostile;
		}
	}

	// 如果对方没有实现阵营接口，则视为中立
	return ETeamAttitude::Neutral;
}