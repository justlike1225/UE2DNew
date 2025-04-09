// My2DGameDesign/Public/Enemies/EvilCreature.h
#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyCharacterBase.h"
#include "Interfaces/MeleeShapeProvider.h"
#include "EvilCreature.generated.h"

// 前向声明
class UEnemyMeleeAttackComponent;
class UTeleportComponent;
class UCapsuleComponent; // <-- 添加胶囊体前向声明
class UPrimitiveComponent; // <-- 添加基础碰撞体前向声明 (Overlap函数参数需要)
struct FHitResult; // <-- 添加命中结果前向声明

// 定义用于识别攻击形状的名称 (即使现在只有一个，也方便未来扩展)
namespace EvilCreatureAttackShapeNames
{
	const FName Melee1(TEXT("Melee1")); // 普通近战攻击范围
	// 如果未来有特殊攻击范围，可以在这里添加
	 const FName Melee2(TEXT("Melee2"));
}


UCLASS()
class MY2DGAMEDESIGN_API AEvilCreature : public AEnemyCharacterBase,public IMeleeShapeProvider
{
	GENERATED_BODY()

public:
	AEvilCreature();
	// --- IMeleeShapeProvider 接口实现声明 ---
	virtual UPrimitiveComponent* GetMeleeShapeComponent_Implementation(FName ShapeIdentifier) const override;

	// --- 组件 Getters ---
	UFUNCTION(BlueprintPure, Category = "Components | Combat")
	UEnemyMeleeAttackComponent* GetMeleeAttackComponent() const { return MeleeAttackComponent; }

	UFUNCTION(BlueprintPure, Category = "Components | Ability")
	UTeleportComponent* GetTeleportComponent() const { return TeleportComponent; }

    // --- 新增: 获取攻击形状组件的函数 (供Component或其他系统调用) ---
    UFUNCTION(BlueprintPure, Category = "Components | Combat")
    UCapsuleComponent* GetMeleeHitShapeComponent(FName ShapeIdentifier) const;


protected:
	// --- 添加的组件 ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UEnemyMeleeAttackComponent> MeleeAttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Ability", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTeleportComponent> TeleportComponent;

  
    /** 用于检测近战攻击命中的碰撞体 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Combat", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCapsuleComponent> MeleeHit1;


    // --- 新增: 碰撞处理函数 ---
    /** 当 MeleeHit1 检测到重叠时调用 */
    UFUNCTION()
    void HandleMeleeHit(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

	virtual void BeginPlay() override; // 重写 BeginPlay 来进行一些初始化检查
};