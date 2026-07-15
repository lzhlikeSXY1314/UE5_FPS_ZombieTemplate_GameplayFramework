// Fill out your copyright notice in the Description page of Project Settings.
#include "SaveSystem/ZombieSaveManager.h"
#include "SaveSystem/ZombieSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/ZombiePlayer.h"

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
    }

    // 保存当前关卡名称
    SaveData->LevelName = GetWorld()->GetMapName();
    
    // 将来扩展：填充 WeaponSaveData 数组...

}

void UZombieSaveManager::ApplySaveData(const UZombieSaveGame* SaveData, APlayerController* PlayerController)
{
    if (!SaveData || !PlayerController) return;

    AZombiePlayer* Player = Cast<AZombiePlayer>(PlayerController->GetPawn());
    if (Player)
    {
        Player->SetActorLocation(SaveData->PlayerLocation);
        Player->SetActorRotation(SaveData->PlayerRotation);
        Player->CurrentHealth = SaveData->PlayerHealth;
    }

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


