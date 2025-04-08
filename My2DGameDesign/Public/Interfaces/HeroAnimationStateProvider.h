// My2DGameDesign/Public/Interfaces/AnimationStateProvider.h
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UObject/ScriptInterface.h" // 需要 TScriptInterface
#include "Interfaces/CharacterAnimationStateListener.h" // 需要 ICharacterAnimationStateListener
#include "HeroAnimationStateProvider.generated.h"

UINTERFACE(MinimalAPI, Blueprintable) // Blueprintable 如果需要从蓝图获取
class UHeroAnimationStateProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief 接口，允许 Actor 提供对其动画状态监听器 (ICharacterAnimationStateListener) 的访问。
 */
class MY2DGAMEDESIGN_API IHeroAnimationStateProvider
{
	GENERATED_BODY()
public:
	/**
	 * @brief 获取动画状态监听器接口。
	 * @return 返回一个 TScriptInterface<ICharacterAnimationStateListener>。
	 * 如果 Actor 没有有效的监听器，应返回一个无效的 TScriptInterface (例如默认构造的)。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation Provider")
	TScriptInterface<ICharacterAnimationStateListener> GetAnimStateListener() const;
};