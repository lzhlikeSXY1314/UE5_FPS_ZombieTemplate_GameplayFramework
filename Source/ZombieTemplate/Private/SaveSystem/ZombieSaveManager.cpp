// Fill out your copyright notice in the Description page of Project Settings.
#include "SaveSystem/ZombieSaveManager.h"
#include "SaveSystem/ZombieSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/ZombiePlayer.h"
#include "Items/WeaponBase.h"
#include <Characters/ZombieBase.h>

const FString UZombieSaveManager::SaveSlotPrefix = TEXT("ZombieSaveSlot_");

void UZombieSaveManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("ZombieSaveManager Initialized"));
}

void UZombieSaveManager::Deinitialize()
{
    Super::Deinitialize();
}

UZombieSaveManager* UZombieSaveManager::GetSaveManager(const UObject* WorldContext)
{
    if (!WorldContext) return nullptr;
    UGameInstance* GameInstance = WorldContext->GetWorld()->GetGameInstance();
    if (!GameInstance) return nullptr;
    return GameInstance->GetSubsystem<UZombieSaveManager>();
}

bool UZombieSaveManager::SaveGame(const FString& SlotName)
{
    UZombieSaveGame* SaveData = Cast<UZombieSaveGame>(UGameplayStatics::CreateSaveGameObject(UZombieSaveGame::StaticClass()));
    if (!SaveData) return false;

    // 获取玩家控制器（这里假设单人游戏，取索引0）
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return false;

    PopulateSaveData(SaveData, PC);
    
    SaveData->SaveTimestamp = FDateTime::Now();
    SaveData->SaveDisplayName = SlotName;


    return UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotPrefix + SlotName, 0);
}

UZombieSaveGame* UZombieSaveManager::LoadGame(const FString& SlotName)
{
    const FString FullSlot = SaveSlotPrefix + SlotName;
    if (!UGameplayStatics::DoesSaveGameExist(FullSlot, 0))
    {
        UE_LOG(LogTemp, Warning, TEXT("Save slot '%s' does not exist"), *SlotName);
        return nullptr;
    }
    UZombieSaveGame* SaveData = Cast<UZombieSaveGame>(UGameplayStatics::LoadGameFromSlot(FullSlot, 0));
    return SaveData;
}

bool UZombieSaveManager::DeleteSave(const FString& SlotName)
{
    const FString FullSlot = SaveSlotPrefix + SlotName;
    if (UGameplayStatics::DoesSaveGameExist(FullSlot, 0))
    {
        return UGameplayStatics::DeleteGameInSlot(FullSlot, 0);
    }
    return false;
}

bool UZombieSaveManager::DoesSaveExist(const FString& SlotName) const
{
    return UGameplayStatics::DoesSaveGameExist(SaveSlotPrefix + SlotName, 0);
}

TArray<FString> UZombieSaveManager::GetAllSaveSlots() const
{
    TArray<FString> Slots;
    // 简单枚举 0~9 检查是否存在（可根据需求改进）
    for (int32 i = 0; i < 10; ++i)
    {
        FString Slot = FString::FromInt(i);
        if (DoesSaveExist(Slot))
        {
            Slots.Add(Slot);
        }
    }
    return Slots;
}

void UZombieSaveManager::PopulateSaveData(UZombieSaveGame* SaveData, APlayerController* PlayerController)
{
    if (!SaveData || !PlayerController) return;
    // 获取玩家角色
    AZombiePlayer* Player = Cast<AZombiePlayer>(PlayerController->GetPawn());
    if (Player)
    {
        SaveData->PlayerLocation = Player->GetActorLocation();
        SaveData->PlayerRotation = Player->GetActorRotation();
        SaveData->PlayerHealth = Player->CurrentHealth; // 假设 AZombiePlayer 中有 CurrentHealth

        // 记录装备武器的 ID
        if (Player->GetCurrentWeapon() && Player->GetCurrentWeapon()->Implements<USaveableActor>())
        {
            SaveData->EquippedWeaponID = ISaveableActor::Execute_GetUniqueSaveID(Player->GetCurrentWeapon());
        }
        else
        {
            SaveData->EquippedWeaponID = NAME_None;
        }
    }

    // 2. 遍历所有实现了 ISaveableActor 的 Actor，收集其状态
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveableActor::StaticClass(), AllActors);
    SaveData->ActorSaveDataList.Empty();
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->Implements<USaveableActor>())
        {
            SaveData->ActorSaveDataList.Add(ISaveableActor::Execute_GetSaveData(Actor));
        }
    }

    // 保存当前关卡名称
    SaveData->LevelName = GetWorld()->GetMapName();

}

void UZombieSaveManager::ApplySaveData(const UZombieSaveGame* SaveData, APlayerController* PlayerController)
{
    if (!SaveData || !PlayerController) return;
    AZombiePlayer* Player = Cast<AZombiePlayer>(PlayerController->GetPawn());

    // ===== 0. 获取当前所有可保存 Actor =====
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveableActor::StaticClass(), AllActors);

    // 构建存档数据快速查找表
    TMap<FName, FActorSaveData> SavedMap;
    for (const FActorSaveData& Data : SaveData->ActorSaveDataList)
        SavedMap.Add(Data.ActorID, Data);

    // ===== 1. 先找出装备武器实例（如果存在），防止在清理时被误删 =====
    AWeaponBase* EquippedWeapon = nullptr;
    if (SaveData->EquippedWeaponID != NAME_None)
    {
        for (AActor* Actor : AllActors)
        {
            if (Actor->Implements<USaveableActor>() &&
                ISaveableActor::Execute_GetUniqueSaveID(Actor) == SaveData->EquippedWeaponID)
            {
                EquippedWeapon = Cast<AWeaponBase>(Actor);
                break;
            }
        }
    }

    // ===== 2. 清理不在存档中的多余 Actor（回档前新生成的掉落物等） =====
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !Actor->Implements<USaveableActor>()) continue;
        if (Actor == EquippedWeapon) continue;   // 绝不要删除装备武器

        FName ID = ISaveableActor::Execute_GetUniqueSaveID(Actor);
        if (!SavedMap.Contains(ID))
        {
            Actor->Destroy();
        }
    }

    // 重新收集一次（因为上面销毁了一些 Actor）
    AllActors.Empty();
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveableActor::StaticClass(), AllActors);

    // 再次确认装备武器是否还活着（如果之前不存在，这里仍是 nullptr）
    if (EquippedWeapon && !AllActors.Contains(EquippedWeapon))
    {
        EquippedWeapon = nullptr; // 它意外被销毁了
    }

    // ===== 3. 重置所有“未装备”的 Actor 到默认状态 =====
    for (AActor* Actor : AllActors)
    {
        if (Actor == EquippedWeapon) continue;
        if (Actor->Implements<USaveableActor>())
            ISaveableActor::Execute_ResetToDefault(Actor);
    }

    // ===== 4. 恢复现存 Actor 的自定义状态（弹药、配件等） =====
    for (AActor* Actor : AllActors)
    {
        if (Actor == EquippedWeapon) continue;
        FName ID = ISaveableActor::Execute_GetUniqueSaveID(Actor);
        FActorSaveData* Found = SavedMap.Find(ID);
        if (Found)
            ISaveableActor::Execute_RestoreState(Actor, *Found);
    }

    // ===== 5. 重新生成已消失的 Actor（包括可能缺失的装备武器） =====
    TSet<FName> ExistingIDs;
    for (AActor* Actor : AllActors)
    {
        if (Actor->Implements<USaveableActor>())
            ExistingIDs.Add(ISaveableActor::Execute_GetUniqueSaveID(Actor));
    }

    for (const auto& Pair : SavedMap)
    {
        if (ExistingIDs.Contains(Pair.Key)) continue;

        UClass* ActorClass = LoadClass<AActor>(nullptr, *Pair.Value.ActorClassPath);
        if (!ActorClass) continue;

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, Pair.Value.WorldTransform, SpawnParams);
        if (!NewActor || !NewActor->Implements<USaveableActor>()) continue;

        // 强制设置新 Actor 的 ID 为存档中的 ID
        if (AInspectableItem* Inspectable = Cast<AInspectableItem>(NewActor))
        {
            Inspectable->SaveActorID = Pair.Key;
        }
        ISaveableActor::Execute_RestoreState(NewActor, Pair.Value);
        ExistingIDs.Add(Pair.Key);

        // 如果新生成的是装备武器，记录下来
        if (Pair.Key == SaveData->EquippedWeaponID)
        {
            EquippedWeapon = Cast<AWeaponBase>(NewActor);
        }
    }

    // ===== 6. 处理玩家装备的武器 =====
    FActorSaveData* EquippedWeaponData = SaveData->EquippedWeaponID != NAME_None ? SavedMap.Find(SaveData->EquippedWeaponID) : nullptr;

    if (EquippedWeapon && EquippedWeaponData)
    {
        EquippedWeapon->SoftReset();
        EquippedWeapon->RestoreState_Implementation(*EquippedWeaponData);

        if (Player)
        {
            if (Player->GetCurrentWeapon() != EquippedWeapon)
            {
                Player->EquipWeaponDirect(EquippedWeapon);
            }
            else
            {
                EquippedWeapon->RefreshAttachmentVisuals();
            }
        }
    }
    else if (Player)
    {
        // 存档中没有装备武器
        if (Player->GetCurrentWeapon())
        {
            Player->GetCurrentWeapon()->Drop();
            Player->ClearCurrentWeapon();
        }
    }

    // ===== 7. 恢复玩家自身状态 =====
    if (Player)
    {
        Player->SetActorLocation(SaveData->PlayerLocation);
        Player->SetActorRotation(SaveData->PlayerRotation);
        Player->CurrentHealth = SaveData->PlayerHealth;
    }

    // 关卡校验
    FString CurrentLevel = GetWorld()->GetMapName();
    if (SaveData->LevelName != CurrentLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("Save data is from a different level (%s vs %s). Position may be invalid."), *SaveData->LevelName, *CurrentLevel);
    }
}

TArray<FString> UZombieSaveManager::GetAllSlotNames()
{
    TArray<FString> Names;
    for (int32 i = 0; i < 10; ++i)
    {
        Names.Add(FString::Printf(TEXT("Slot%d"), i));
    }
    return Names;
}

UZombieSaveGame* UZombieSaveManager::GetSlotInfo(const FString& SlotName) //纯粹用于 UI 显示
{
    const FString FullSlot = SaveSlotPrefix + SlotName;
    if (!UGameplayStatics::DoesSaveGameExist(FullSlot, 0))
        return nullptr;

    return Cast<UZombieSaveGame>(UGameplayStatics::LoadGameFromSlot(FullSlot, 0));
}


