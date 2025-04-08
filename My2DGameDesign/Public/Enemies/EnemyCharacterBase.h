// My2DGameDesign/Public/Enemies/EnemyCharacterBase.h

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h" // 继承自 PaperZD 角色基类，以便使用其动画系统
#include "Interfaces/Damageable.h" // 包含我们创建的伤害接口头文件
#include "Interfaces/AnimationStateProvider.h" // 包含动画状态提供者接口头文件 (复用英雄的)
#include "Interfaces/FacingDirectionProvider.h" // 包含朝向提供者接口头文件
#include "EnemyCharacterBase.generated.h"       // 包含生成的头文件

// --- 前向声明 ---
// 告诉编译器这些类存在，但不需要知道它们的完整定义，可以减少编译依赖
class UHealthComponent;
class UPaperZDAnimInstance;
class AAIController;
class UBehaviorTree;
class UEnemyAnimInstanceBase;          // 敌人的动画实例基类
class ICharacterAnimationStateListener; // 动画状态监听器接口 (我们先复用英雄的，之后可以换成 IEnemy...)


/**
 * UCLASS 标记此类为 UE 的反射系统可识别。
 * Abstract 标记此类为抽象类，不能直接在编辑器中实例化或放置在关卡中，必须被继承。
 */
UCLASS(Abstract)
class MY2DGAMEDESIGN_API AEnemyCharacterBase : public APaperZDCharacter, // 继承 PaperZD 角色
                                               public IDamageable,             // 公开继承我们创建的 IDamageable 接口
                                               public IAnimationStateProvider, // 公开继承 IAnimationStateProvider 接口
                                               public IFacingDirectionProvider // 公开继承 IFacingDirectionProvider 接口
{
	GENERATED_BODY() // UE 类所需的宏

public:
	// 构造函数
	AEnemyCharacterBase();

	/** 获取生命组件的公共访问函数 */
	UFUNCTION(BlueprintPure, Category = "Components | Health") // BlueprintPure 表明在蓝图中调用不改变状态
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

    /** 获取敌人动画实例基类的公共访问函数 */
    UFUNCTION(BlueprintPure, Category="Animation")
    UEnemyAnimInstanceBase* GetEnemyAnimInstance() const;

	// --- 接口实现函数声明 ---
	// 这些是 C++ 中实现接口函数的标准方式 (使用 override 关键字)。

	// --- IDamageable 接口实现 ---
	// 标记为 virtual 和 override，表明它正在覆盖基接口中的虚函数。
	// _Implementation 后缀是 BlueprintNativeEvent 的 C++ 实现要求。
	virtual float ApplyDamage_Implementation(float DamageAmount, AActor* DamageCauser, AController* InstigatorController, const FHitResult& HitResult) override;

    // --- IAnimationStateProvider 接口实现 ---
    // 返回动画实例所实现的监听器接口。
    virtual TScriptInterface<ICharacterAnimationStateListener> GetAnimStateListener_Implementation() const override;

    // --- IFacingDirectionProvider 接口实现 ---
    // 返回角色当前的面向向量。
    virtual FVector GetFacingDirection_Implementation() const override;
	/** 指定这个敌人使用的行为树资产 */
	UPROPERTY(EditDefaultsOnly, Category="AI | Configuration")
	TObjectPtr<UBehaviorTree> BehaviorTree; // 指向行为树资产

protected:
	// protected: 只能被本类和子类访问。

	// --- 生命周期函数 ---
	// 当 Actor 在游戏世界中开始存在时调用。
	virtual void BeginPlay() override;
    // 当一个新的 Controller (通常是 AIController) 控制了这个 Pawn 时调用。
    virtual void PossessedBy(AController* NewController) override;

	// --- 核心组件指针 ---
	// 使用 UPROPERTY 宏来让 UE 管理这些指针。
	// VisibleAnywhere: 在编辑器中任何地方都可见（但通常不可编辑）。
	// BlueprintReadOnly: 蓝图中只能读取。
	// Category: 在编辑器细节面板中的分组。
	// meta = (AllowPrivateAccess = "true"): 即使是 private/protected，也允许在此类内部直接访问。
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent; // 指向我们的生命组件

   

  

	// --- 内部状态与引用 ---

	/** 用于缓存动画实例实现的监听器接口，避免每次都去查找 */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Animation", meta=(AllowPrivateAccess = "true"))
    TScriptInterface<ICharacterAnimationStateListener> AnimationStateListener; // TScriptInterface 用于存储接口指针

    /**
     * UFUNCTION() 宏标记此函数可用于委托绑定。
     * 当 HealthComponent 的 OnDeath 委托广播时，将调用此函数。
     * virtual 允许子类重写此函数以添加特定的死亡行为。
     */
    UFUNCTION()
    virtual void HandleDeath(AActor* Killer);

    /**
     * 用于设置角色视觉朝向的内部函数。
     * @param bFaceRight true表示朝右, false表示朝左。
     * virtual 允许子类可能有不同的朝向设置方式。
     */
    virtual void SetFacingDirection(bool bFaceRight);

    /** 存储当前是否朝右的状态 */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="State | Direction", meta=(AllowPrivateAccess="true"))
    bool bIsFacingRight = true; // 默认为 true (朝右)

private:
	// private: 只能被本类访问。

    /** 辅助函数，在 BeginPlay 时尝试查找并缓存动画监听器接口 */
    void CacheAnimationStateListener();
};