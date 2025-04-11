// My2DGameDesign/Private/Components/EnemyMeleeAttackComponent.cpp
#include "Components/EnemyMeleeAttackComponent.h"
#include "DataAssets/Enemy/EnemyMeleeAttackSettingsDA.h"
#include "Enemies/EnemyCharacterBase.h"
#include "Interfaces/MeleeShapeProvider.h" // <--- 添加: 需要查询形状
#include "Components/PrimitiveComponent.h"
#include "Interfaces/AnimationListenerProvider/EnemySpecificAnimListenerProvider.h"
#include "Interfaces/AnimationListener/EnemyMeleeAttackAnimListener.h"
#include "Interfaces/Damageable.h" // <--- 添加: 需要应用伤害
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h" // <--- 添加: 需要获取 Controller
// #include "Kismet/GameplayStatics.h"

// ... (构造函数, BeginPlay 保持不变) ...
UEnemyMeleeAttackComponent::UEnemyMeleeAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);

	bCanAttack = true;
	bIsAttacking = false;
    ActiveCollisionShapeName = NAME_None;
}

void UEnemyMeleeAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerEnemyCharacter = Cast<AEnemyCharacterBase>(GetOwner());
	if (!OwnerEnemyCharacter.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyMeleeAttackComponent '%s' requires its owner to be derived from AEnemyCharacterBase!"), *GetName());
        SetActive(false);
		return;
	}

    if(!AttackSettings)
    {
         UE_LOG(LogTemp, Warning, TEXT("EnemyMeleeAttackComponent '%s' on Actor '%s' is missing AttackSettings Data Asset!"), *GetName(), *OwnerEnemyCharacter->GetName());
    }
}

void UEnemyMeleeAttackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 清理可能仍在运行的计时器
    if(GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimer);
        GetWorld()->GetTimerManager().ClearTimer(ActiveCollisionTimerHandle); // 清理碰撞计时器
    }
    // --- 添加: 确保解绑 ---
    if (ActiveCollisionShapeName != NAME_None)
    {
        DeactivateMeleeCollision(ActiveCollisionShapeName); // 会尝试解绑
    }
	Super::EndPlay(EndPlayReason);
}


bool UEnemyMeleeAttackComponent::ExecuteAttack(AActor* Target)
{
    if (!bCanAttack || bIsAttacking || !AttackSettings || !OwnerEnemyCharacter.IsValid())
    {
        return false;
    }
    // ... (其他逻辑不变) ...
    UE_LOG(LogTemp, Log, TEXT("EnemyMeleeAttackComponent '%s': Executing Melee Attack."), *GetName());

    bIsAttacking = true;
    bCanAttack = false;
    BeginAttackSwing();
    StartAttackCooldown();

    TScriptInterface<IEnemyMeleeAttackAnimListener> Listener = GetAnimListener();
	if (Listener)
	{
		// UE_LOG(LogTemp, Log, TEXT("MeleeAttackComponent: Attempting to call OnMeleeAttackStarted on VALID Listener. Target: %s"), Target ? *Target->GetName() : TEXT("None"));
		Listener->Execute_OnMeleeAttackStarted(Listener.GetObject(), Target);
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("MeleeAttackComponent: Could not get valid MeleeAnimListener from owner when trying to ExecuteAttack!"));
	}
    return true;
}

void UEnemyMeleeAttackComponent::ActivateMeleeCollision(FName ShapeIdentifier, float Duration)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || Duration <= 0 || ShapeIdentifier == NAME_None) return;

    // 先尝试关闭上一个激活的碰撞体 (确保解绑旧的)
    if (ActiveCollisionShapeName != NAME_None && ActiveCollisionShapeName != ShapeIdentifier)
    {
        DeactivateMeleeCollision(ActiveCollisionShapeName);
    }

	UPrimitiveComponent* ShapeToActivate = nullptr;
	if (IMeleeShapeProvider* ShapeProvider = Cast<IMeleeShapeProvider>(OwnerActor))
	{
		ShapeToActivate = IMeleeShapeProvider::Execute_GetMeleeShapeComponent(OwnerActor, ShapeIdentifier);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateMeleeCollision: Owner '%s' does not implement IMeleeShapeProvider."), *OwnerActor->GetName());
		return;
	}

	if (ShapeToActivate)
	{
        // --- 只有当形状不同或者之前没有激活时才绑定 ---
        if (ShapeIdentifier != ActiveCollisionShapeName)
        {
            // 清理之前的命中记录
            BeginAttackSwing();
            // 绑定新的委托
            ShapeToActivate->OnComponentBeginOverlap.AddDynamic(this, &UEnemyMeleeAttackComponent::HandleAttackOverlap);
            UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent: Bound overlap event for shape '%s'."), *ShapeIdentifier.ToString());
        }

		UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent: Activating collision shape '%s' for %.2f seconds."), *ShapeIdentifier.ToString(), Duration);
		ShapeToActivate->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ActiveCollisionShapeName = ShapeIdentifier;

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(ActiveCollisionTimerHandle);
			FTimerDelegate TimerDelegate;
            // --- 注意: FTimerDelegate 不能直接绑定带参数的函数，我们绑定一个不带参数的辅助函数或 Lambda ---
            // 简单的做法是绑定一个函数，该函数知道要关闭哪个 Shape
            // 或者，因为 DeactivateMeleeCollision 现在会检查 ActiveCollisionShapeName，我们可以直接绑定它
			TimerDelegate.BindUFunction(this, FName("DeactivateMeleeCollision"), ActiveCollisionShapeName); // 传递当前激活的名称
			GetWorld()->GetTimerManager().SetTimer(ActiveCollisionTimerHandle, TimerDelegate, Duration, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyMeleeAttackComponent: Owner '%s' (IMeleeShapeProvider) did not provide a shape for identifier '%s'."), *OwnerActor->GetName(), *ShapeIdentifier.ToString());
	}
}


void UEnemyMeleeAttackComponent::DeactivateMeleeCollision(FName ShapeIdentifier)
{
     // 检查是否是当前激活的形状，或者是否传入了要关闭的形状
     if (ShapeIdentifier == NAME_None || ShapeIdentifier != ActiveCollisionShapeName) return;

     AActor* OwnerActor = GetOwner();
     if(!OwnerActor) return;

     UPrimitiveComponent* ShapeToDeactivate = nullptr;
     if (IMeleeShapeProvider* ShapeProvider = Cast<IMeleeShapeProvider>(OwnerActor))
	 {
		ShapeToDeactivate = IMeleeShapeProvider::Execute_GetMeleeShapeComponent(OwnerActor, ShapeIdentifier);
	 }

	 if (ShapeToDeactivate)
	 {
         // 清除关闭计时器句柄
          if (GetWorld())
          {
             GetWorld()->GetTimerManager().ClearTimer(ActiveCollisionTimerHandle);
          }

         UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent: Deactivating collision shape '%s'."), *ShapeIdentifier.ToString());

         // --- 解绑事件 ---
         ShapeToDeactivate->OnComponentBeginOverlap.RemoveDynamic(this, &UEnemyMeleeAttackComponent::HandleAttackOverlap);
         UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent: Unbound overlap event for shape '%s'."), *ShapeIdentifier.ToString());

		 ShapeToDeactivate->SetCollisionEnabled(ECollisionEnabled::NoCollision);
         ActiveCollisionShapeName = NAME_None; // 清除激活记录
	 }
}


void UEnemyMeleeAttackComponent::StartAttackCooldown()
{
	if (AttackSettings && AttackSettings->AttackCooldown > 0 && GetWorld())
	{
         if (!GetWorld()->GetTimerManager().IsTimerActive(AttackCooldownTimer))
         {
            // UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent '%s': Starting cooldown for %.2f seconds."), *GetName(), AttackSettings->AttackCooldown);
            GetWorld()->GetTimerManager().SetTimer(
                AttackCooldownTimer,
                this,
                &UEnemyMeleeAttackComponent::OnAttackCooldownFinished,
                AttackSettings->AttackCooldown,
                false
            );
         }
	}
	else
	{
		OnAttackCooldownFinished(); // 立即结束冷却
	}
}

void UEnemyMeleeAttackComponent::OnAttackCooldownFinished()
{
    // UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent '%s': Cooldown finished."), *GetName());
	bCanAttack = true;
    bIsAttacking = false;
    // --- 添加: 确保冷却结束时，激活的碰撞体也被关闭和解绑 ---
    if(ActiveCollisionShapeName != NAME_None)
    {
        DeactivateMeleeCollision(ActiveCollisionShapeName);
    }
}

TScriptInterface<IEnemyMeleeAttackAnimListener> UEnemyMeleeAttackComponent::GetAnimListener() const
{
    // ... (保持不变) ...
    if (OwnerEnemyCharacter.IsValid())
	{
		if (IEnemySpecificAnimListenerProvider* Provider = Cast<IEnemySpecificAnimListenerProvider>(OwnerEnemyCharacter.Get()))
		{
			TScriptInterface<IEnemyMeleeAttackAnimListener> Listener = Provider->Execute_GetMeleeAttackAnimListener(OwnerEnemyCharacter.Get());
			if (Listener)
			{
				return Listener;
			}
		}
	}
	return nullptr;
}

void UEnemyMeleeAttackComponent::BeginAttackSwing()
{
    HitActorsThisSwing.Empty();
    // UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent: Cleared HitActorsThisSwing for new swing."));
}

// --- 新增的碰撞处理函数实现 ---
void UEnemyMeleeAttackComponent::HandleAttackOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    AActor* OwnerActor = GetOwner(); // 获取组件的拥有者

    // --- 安全检查 ---
    if (!OtherActor || OtherActor == OwnerActor || !bIsAttacking)
    {
        // 1. 确保碰到的不是自己
        // 2. 确保当前处于攻击流程中 (bIsAttacking 为 true)
        return;
    }

    // --- 防止重复命中 ---
    if (HitActorsThisSwing.Contains(OtherActor))
    {
        // UE_LOG(LogTemp, Verbose, TEXT("EnemyMeleeAttackComponent: Actor '%s' already hit this swing."), *OtherActor->GetName());
        return; // 已经打过了
    }

    // --- 检查目标是否可受击 ---
    if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
    {
        // --- 记录命中 ---
        HitActorsThisSwing.Add(OtherActor);

        // --- 获取伤害值和攻击者信息 ---
        float DamageToApply = 0.0f;
        if (AttackSettings)
        {
            DamageToApply = AttackSettings->AttackDamage;
        }
        else
        {
             UE_LOG(LogTemp, Warning, TEXT("HandleAttackOverlap: MeleeAttackComponent on %s is missing AttackSettings DA! Applying 0 damage."), *GetNameSafe(OwnerActor));
        }

        AController* InstigatorController = nullptr;
        if(AEnemyCharacterBase* OwnerEnemy = Cast<AEnemyCharacterBase>(OwnerActor))
        {
             InstigatorController = OwnerEnemy->GetController();
        }

        // --- 施加伤害 ---
        UE_LOG(LogTemp, Log, TEXT("EnemyMeleeAttackComponent on '%s' hit '%s' via shape '%s' for %.1f damage."),
               OwnerActor ? *OwnerActor->GetName() : TEXT("Unknown"),
               *OtherActor->GetName(),
               *OverlappedComponent->GetName(),
               DamageToApply);

        IDamageable::Execute_ApplyDamage(OtherActor, DamageToApply, OwnerActor, InstigatorController, SweepResult);

        // --- (可选) 播放命中特效/声音 ---
        // ...
    }
    // else: 如果碰到的 Actor 不能接受伤害 (比如墙壁或其他敌人)，则忽略
}