// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ZombieSaveGame.generated.h"

/** 玩家武器状态（将来扩展） */
USTRUCT(BlueprintType)
struct FWeaponSaveData
{
    GENERATED_BODY(SaveGame)

    UPROPERTY()
    int32 CurrentAmmo = 0;

    UPROPERTY(SaveGame)
    bool bIsEquipped = false;
};




UCLASS()
class ZOMBIETEMPLATE_API UZombieSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
    /** 玩家位置 */
    UPROPERTY(SaveGame)
    FVector PlayerLocation;

    /** 玩家旋转 */
    UPROPERTY(SaveGame)
    FRotator PlayerRotation;

    /** 玩家血量（虽然丧尸有血量，但玩家也需要保存） */
    UPROPERTY(SaveGame)
    float PlayerHealth = 100.0f;

    /** 当前关卡名称（用于加载对应关卡） */
    UPROPERTY(SaveGame)
    FString LevelName;

    /** 已装备武器的保存数据（将来扩展多个武器） */
    UPROPERTY(SaveGame)
    TArray<FWeaponSaveData> Weapons;

    /** 存档保存时的日期时间 */
    UPROPERTY(SaveGame)
    FDateTime SaveTimestamp;

    /** 自定义显示名称（例如玩家输入的名字） */
    UPROPERTY(SaveGame)
    FString SaveDisplayName;

    /** 清空所有数据（用于新游戏） */
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void Clear();
};
