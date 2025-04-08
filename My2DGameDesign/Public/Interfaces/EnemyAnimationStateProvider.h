// My2DGameDesign/Public/Interfaces/EnemyAnimationStateProvider.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ScriptInterface.h"           // 需要 TScriptInterface
// 注意：这里包含的是敌人的监听器接口头文件！
#include "Interfaces/EnemyAnimationStateListener.h"
#include "EnemyAnimationStateProvider.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEnemyAnimationStateProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief 接口，允许 Actor (特指敌人) 提供对其 *敌人* 动画状态监听器 (IEnemyAnimationStateListener) 的访问。
 */
class MY2DGAMEDESIGN_API IEnemyAnimationStateProvider
{
	GENERATED_BODY()
public:
	/**
	 * @brief 获取敌人动画状态监听器接口。
	 * @return 返回一个 TScriptInterface<IEnemyAnimationStateListener>。
	 * 实现者 (如 AEnemyCharacterBase) 应返回其动画实例实现的监听器接口。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider | Enemy")
	TScriptInterface<IEnemyAnimationStateListener> GetEnemyAnimStateListener() const; // 函数名也区分一下
};