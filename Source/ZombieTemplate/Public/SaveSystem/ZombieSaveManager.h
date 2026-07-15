// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ZombieSaveGame.h"
#include "ZombieSaveManager.generated.h"

class UZombieSaveGame;

UCLASS()
class ZOMBIETEMPLATE_API UZombieSaveManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	/** 获取保存管理器实例 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SaveSystem")
	static UZombieSaveManager* GetSaveManager(const UObject* WorldContext);

	/** 保存游戏到指定槽位 */
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	bool SaveGame(const FString& SlotName);

	/** 从指定槽位加载游戏 */
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	UZombieSaveGame* LoadGame(const FString& SlotName);

	/** 删除指定槽位的存档 */
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	bool DeleteSave(const FString& SlotName);

	/** 检查槽位是否存在 */
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	bool DoesSaveExist(const FString& SlotName) const;

	/** 获取所有存档槽位名称（简单遍历 Slot0..N，仅演示） */
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	TArray<FString> GetAllSaveSlots() const;

	/** 填充存档数据（从当前游戏世界获取状态） */
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void PopulateSaveData(UZombieSaveGame* SaveData, class APlayerController* PlayerController);

	/** 应用存档数据到游戏世界 */
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void ApplySaveData(const UZombieSaveGame* SaveData, class APlayerController* PlayerController);

	/** 获取所有槽位名称（Slot0 ~ Slot9） */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SaveSystem")
	static TArray<FString> GetAllSlotNames();

	/** 获取指定槽位的存档信息（不应用到游戏），若不存在返回 nullptr */
	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	UZombieSaveGame* GetSlotInfo(const FString& SlotName);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	static const FString SaveSlotPrefix;



};
