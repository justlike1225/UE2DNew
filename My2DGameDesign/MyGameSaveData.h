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
    UPROPERTY(BlueprintReadWrite, SaveGame, Category = "PlayerState")
    float PlayerCurrentHealth;

    UPROPERTY(BlueprintReadWrite, SaveGame, Category = "PlayerState")
    float PlayerMaxHealth;

    UPROPERTY(BlueprintReadWrite, SaveGame, Category = "PlayerState")
    float PlayerCurrentRage;

    UPROPERTY(BlueprintReadWrite, SaveGame, Category = "PlayerState")
    float PlayerMaxRage;

    UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Level")
    FName LastLevelName;

 
    UMyGameSaveData()
    {
        LastLevelName = FName("None");
        PlayerCurrentHealth = 100.f;
        PlayerMaxHealth = 100.f;
        PlayerCurrentRage = 0.f;
        PlayerMaxRage = 100.f;
       
    }
};