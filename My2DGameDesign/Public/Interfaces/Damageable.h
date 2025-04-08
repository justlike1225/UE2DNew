// My2DGameDesign/Public/Interfaces/Damageable.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

// UInterface boilerplate - 这是UE反射系统需要的标准写法
UINTERFACE(MinimalAPI, Blueprintable) // MinimalAPI 通常足够, Blueprintable 允许蓝图类也实现此接口
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief C++ 接口，用于标记和定义可以接受伤害的 Actor。
 * 实现此接口的类需要提供 ApplyDamage_Implementation 函数。
 */
class MY2DGAMEDESIGN_API IDamageable
{
	GENERATED_BODY()

public:
	/**
	 * @brief 对此 Actor 应用伤害的核心函数。
	 * @param DamageAmount 造成的伤害数值。
	 * @param DamageCauser 造成此次伤害的源头 Actor (例如发射子弹的角色, 或者触发陷阱的机关)。
	 * @param InstigatorController 造成伤害行为的控制者 (例如玩家控制器或AI控制器)。
	 * @param HitResult (可选) 如果伤害是由物理碰撞（如子弹击中）产生的，这里会包含详细的命中信息。
	 * @return 返回实际生效的伤害量。如果你的游戏有伤害减免、格挡等逻辑，可以在实现中修改返回值。默认可以直接返回传入的 DamageAmount。
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage | Combat") // BlueprintNativeEvent 意味着C++和蓝图都可以实现; BlueprintCallable 允许在蓝图中调用此接口函数
	float ApplyDamage(float DamageAmount, AActor* DamageCauser, AController* InstigatorController, const FHitResult& HitResult);
};