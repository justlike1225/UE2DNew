// MyGameSaveData.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MyGameSaveData.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UMyGameSaveData : public USaveGame
{
    GENERATED_BODY()

public:
    // --- 需要保存的数据 ---

    // 玩家上次所在的关卡名称 (非常重要，用于继续游戏时加载正确的关卡)
    UPROPERTY(SaveGame)
    FName LastLevelName;

    // 玩家状态
    UPROPERTY(SaveGame)
    float PlayerCurrentHealth;
    UPROPERTY(SaveGame)
    float PlayerMaxHealth; // 保存最大值也很重要，以防升级等改变了最大值
    UPROPERTY(SaveGame)
    float PlayerCurrentRage;
    UPROPERTY(SaveGame)
    float PlayerMaxRage;

  
    // 关卡进度相关 (示例)
    UPROPERTY(SaveGame)
    int32 CurrentLevelIndex = 1; // 例如，标记当前是第几关

  


  
    FString SaveSlotName = TEXT("MyGameSlot_0"); // 存档槽名称
    uint32 UserIndex = 0; // 用户索引（单人游戏通常为0）

   
    UMyGameSaveData()
    {
        LastLevelName = FName("None"); // 初始值
        PlayerCurrentHealth = 100.f;
        PlayerMaxHealth = 100.f;
        PlayerCurrentRage = 0.f;
        PlayerMaxRage = 100.f;
        CurrentLevelIndex = 1;
    }
};