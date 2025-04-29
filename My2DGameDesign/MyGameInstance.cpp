// MyGameInstance.cpp
#include "MyGameInstance.h" // 修改路径
#include "MyGameSaveData.h"
#include "Kismet/GameplayStatics.h"
#include "Actors/PaperZDCharacter_SpriteHero.h" // 需要玩家类
#include "Components/HealthComponent.h"        // 需要组件类
#include "Components/RageComponent.h"
#include "GameFramework/PlayerController.h"     // 需要 PlayerController
bool UMyGameInstance::SaveGameData()
{
	UMyGameSaveData* SaveGameObject = nullptr;

	if (DoesSaveGameExist())
	{
		SaveGameObject = Cast<UMyGameSaveData>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));
	}
	else
	{
		SaveGameObject = Cast<UMyGameSaveData>(UGameplayStatics::CreateSaveGameObject(UMyGameSaveData::StaticClass()));
	}

	if (!SaveGameObject)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGameData: Create/Load SaveGame failed!"));
		return false;
	}

	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGameData: No PlayerController!"));
		return false;
	}

	APaperZDCharacter_SpriteHero* PlayerPawn = Cast<APaperZDCharacter_SpriteHero>(PC->GetPawn());
	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveGameData: No valid PlayerPawn!"));
		return false;
	}

	if (UHealthComponent* HealthComp = PlayerPawn->GetHealthComponent())
	{
		SaveGameObject->PlayerCurrentHealth = HealthComp->GetCurrentHealth();
		SaveGameObject->PlayerMaxHealth = HealthComp->GetMaxHealth();
	}

	if (URageComponent* RageComp = PlayerPawn->GetRageComponent())
	{
		SaveGameObject->PlayerCurrentRage = RageComp->GetCurrentRage();
		SaveGameObject->PlayerMaxRage = RageComp->GetMaxRage();
	}

	FString CurrentMapName = GetWorld()->GetMapName();
	CurrentMapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	SaveGameObject->LastLevelName = FName(*CurrentMapName);

	return UGameplayStatics::SaveGameToSlot(SaveGameObject, SaveSlotName, UserIndex);
}
bool UMyGameInstance::LoadGameData()
{
	if (!DoesSaveGameExist())
	{
		CurrentSaveData = nullptr;
		return false;
	}

	CurrentSaveData = Cast<UMyGameSaveData>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));
	return CurrentSaveData != nullptr;
}

void UMyGameInstance::ApplyLoadedDataToPlayer()
{
	if (!CurrentSaveData)
		return;

	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC)
		return;

	APaperZDCharacter_SpriteHero* PlayerPawn = Cast<APaperZDCharacter_SpriteHero>(PC->GetPawn());
	
	if (!PlayerPawn)
		return;
	//打印存档血量
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
		FString::Printf(TEXT("存档血量信息 %.1f/%.1f"), CurrentSaveData->PlayerCurrentHealth,
			CurrentSaveData->PlayerMaxHealth));
 
	if (UHealthComponent* HealthComp = PlayerPawn->GetHealthComponent())
	{
		HealthComp->RestoreHealth(CurrentSaveData->PlayerCurrentHealth, CurrentSaveData->PlayerMaxHealth);
	}

	if (URageComponent* RageComp = PlayerPawn->GetRageComponent())
	{
		RageComp->RestoreRage(CurrentSaveData->PlayerCurrentRage, CurrentSaveData->PlayerMaxRage);
	}
	//打印玩家血量
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
		FString::Printf(TEXT("Player Health: %.1f/%.1f"), PlayerPawn->GetHealthComponent()->GetCurrentHealth(),
			PlayerPawn->GetHealthComponent()->GetMaxHealth()) );


	// 重要：恢复后清空缓存，避免重复应用
	CurrentSaveData = nullptr;
}


bool UMyGameInstance::DoesSaveGameExist() const

{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);
}
