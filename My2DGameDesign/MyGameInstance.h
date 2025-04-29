// MyGameInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameSaveData.h" // 包含我们之前创建的 SaveGame 类
#include "MyGameInstance.generated.h"

UCLASS()
class MY2DGAMEDESIGN_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// --- 存档相关 ---

	/** 保存当前游戏状态到指定槽位 */
	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	bool SaveGameData();

	/** 从指定槽位加载游戏数据到 GameInstance (但不应用到角色) */
	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	bool LoadGameData();

	/** 将 GameInstance 中缓存的存档数据应用到当前玩家角色上 */
	UFUNCTION(BlueprintCallable, Category = "SaveLoad")
	void ApplyLoadedDataToPlayer();

	/** 检查存档是否存在 */
	UFUNCTION(BlueprintPure, Category = "SaveLoad")
	bool DoesSaveGameExist() const;

	/** 获取加载的存档数据 (只读，用于检查或读取关卡名等) */
	UFUNCTION(BlueprintPure, Category = "SaveLoad")
	UMyGameSaveData* GetLoadedSaveData() const { return CurrentSaveData; }

protected:
	/** 用于在加载关卡之间临时持有加载的存档数据 */
	UPROPERTY(Transient) 
	TObjectPtr<UMyGameSaveData> CurrentSaveData;

	/** 存档槽名称 */
	UPROPERTY(EditDefaultsOnly, Category = "SaveLoad")
	FString SaveSlotName = TEXT("MyGameSlot_0");

	/** 用户索引 */
	UPROPERTY(EditDefaultsOnly, Category = "SaveLoad")
	uint32 UserIndex = 0;

	
};