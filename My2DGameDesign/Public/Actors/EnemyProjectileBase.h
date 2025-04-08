// My2DGameDesign/Public/Actors/EnemyProjectileBase.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyProjectileBase.generated.h" // 生成的头文件

// --- 前向声明 ---
class USphereComponent;             // 球形碰撞体 (用于碰撞检测)
class UPaperSpriteComponent;        // 2D 精灵组件 (用于视觉表现)
class UProjectileMovementComponent; // 投掷物移动组件 (处理飞行逻辑)
class AController;                  // 控制器基类

/**
 * 敌人发射的投掷物的基础 Actor 类。
 * 处理移动、碰撞和伤害施加。
 * 可以被蓝图继承以创建不同外观和效果的投掷物。
 */
UCLASS(Abstract, Blueprintable) // Abstract: 不能直接放置; Blueprintable: 可被蓝图继承
class MY2DGAMEDESIGN_API AEnemyProjectileBase : public AActor
{
	GENERATED_BODY() // UE类宏

public:
	// 构造函数
	AEnemyProjectileBase();

	/**
	 * @brief 初始化投掷物。通常在生成此 Actor 后由发射者（如 RangedAttackComponent）调用。
	 * @param Direction 发射的初始方向 (单位向量)。
	 * @param Speed 初始速度。
	 * @param Damage 命中时造成的伤害值。
	 * @param LifeSpan 投掷物的生存时间 (秒)。
	 * @param Shooter 发射此投掷物的 Actor。
	 * @param ShooterController 发射者的控制器。
	 */
	UFUNCTION(BlueprintCallable, Category = "Projectile") // 允许蓝图调用
	virtual void InitializeProjectile(const FVector& Direction, float Speed, float Damage, float LifeSpan, AActor* Shooter, AController* ShooterController);

protected:
	// --- 组件 ---

	/** 球形碰撞体，作为根组件，负责主要的碰撞检测 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	/** 2D 精灵组件，用于显示投掷物的视觉外观 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent; // 使用 PaperSprite 因为我们是2D

	/** 投掷物移动组件，处理匀速直线运动、重力（如果需要）、弹跳（如果需要）等 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	// --- 运行时变量 ---

	/** 存储从 InitializeProjectile 传入的伤害值 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Projectile | Runtime", meta = (AllowPrivateAccess = "true"))
	float CurrentDamage = 0.0f;

	/** 指向发射者的弱指针，用于避免击中自己 */
	UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Projectile | Runtime", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<AActor> InstigatorActor;

    /** 指向发射者控制器的弱指针，用于传递给伤害函数 */
    UPROPERTY(VisibleInstanceOnly, Transient, BlueprintReadOnly, Category = "Projectile | Runtime", meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<AController> InstigatorController;


	// --- 事件处理 ---

	/**
	 * 当碰撞体重叠到其他 Actor 时调用。
	 * BlueprintNativeEvent 允许 C++ 和蓝图都实现此函数。
	 * 我们将在这里处理命中逻辑。
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Projectile | Collision")
	void OnProjectileOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	virtual void OnProjectileOverlapBegin_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


    /**
     * (可选) 当投掷物停止运动时调用 (例如撞到墙壁)。
     * 可以根据需要处理撞击效果。 UProjectileMovementComponent 有 OnProjectileStop 委托。
     */
     // UFUNCTION()
     // virtual void OnProjectileStop(const FHitResult& ImpactResult);

	// --- 生命周期函数 ---
	// virtual void BeginPlay() override; // 通常初始化逻辑放在 InitializeProjectile

};