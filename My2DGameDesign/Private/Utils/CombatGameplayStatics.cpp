// 文件路径: Private/Utils/CombatGameplayStatics.cpp
#include "Utils/CombatGameplayStatics.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h" // 需要 Controller 来获取 TeamAgent
#include "GameFramework/Pawn.h"       // 可能需要 Pawn

const IGenericTeamAgentInterface* UCombatGameplayStatics::GetTeamAgentInterfaceFromActor(const AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	// 优先尝试从 Actor 的 Controller 获取 TeamAgent
	const APawn* Pawn = Cast<APawn>(Actor);
	if (Pawn && Pawn->GetController())
	{
		const IGenericTeamAgentInterface* ControllerTeamAgent = Cast<IGenericTeamAgentInterface>(Pawn->GetController());
		if (ControllerTeamAgent)
		{
			return ControllerTeamAgent;
		}
	}

	// 如果 Controller 没有实现，或者 Actor 不是 Pawn，再尝试直接从 Actor 获取
	const IGenericTeamAgentInterface* ActorTeamAgent = Cast<IGenericTeamAgentInterface>(Actor);
	return ActorTeamAgent; // 返回找到的接口，或者 nullptr
}

bool UCombatGameplayStatics::CanDamageActor(const AActor* Attacker, const AActor* Victim)
{
	// 基本检查
	if (!Attacker || !Victim || Attacker == Victim)
	{
		
		return true;
	}

	// 获取双方的 TeamAgent 接口
	const IGenericTeamAgentInterface* AttackerTeamAgent = GetTeamAgentInterfaceFromActor(Attacker);
	const IGenericTeamAgentInterface* VictimTeamAgent = GetTeamAgentInterfaceFromActor(Victim);

	// 如果双方都有有效的 TeamAgent 接口
	if (AttackerTeamAgent && VictimTeamAgent)
	{
		// 获取双方的 Team ID
		FGenericTeamId AttackerTeamId = AttackerTeamAgent->GetGenericTeamId();
		FGenericTeamId VictimTeamId = VictimTeamAgent->GetGenericTeamId();

		// 使用引擎内置的函数或直接比较 ID 来判断态度
		ETeamAttitude::Type Attitude = FGenericTeamId::GetAttitude(AttackerTeamId, VictimTeamId);
       

		// 如果是友方，则不能造成伤害
		if (Attitude == ETeamAttitude::Friendly)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Friendly fire prevented: %s (Team %d) cannot damage %s (Team %d). Attitude: Friendly."),
                   *Attacker->GetName(), AttackerTeamId.GetId(),
                   *Victim->GetName(), VictimTeamId.GetId());
			return false; // 不能伤害友方
		}
	}
	

	// 默认情况下 (非友方，或无法判断阵营)，允许造成伤害
	return true;
}