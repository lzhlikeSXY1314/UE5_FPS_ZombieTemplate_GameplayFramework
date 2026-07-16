#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveableActor.generated.h"

// 保存数据的数据结构
USTRUCT(BlueprintType)
struct FActorSaveData
{
    GENERATED_BODY()

    UPROPERTY()
    FName ActorID;                // 唯一标识符（建议用 Actor 的 Name）

    UPROPERTY()
    FString ActorClassPath;       // 类路径，用于重新创建

    UPROPERTY()
    FTransform WorldTransform;    // 世界空间位置/旋转/缩放

    UPROPERTY()
    bool bEquipped = false;       // 是否被装备（附着在玩家身上）

    UPROPERTY()
    FName EquipSocket;            // 装备时挂载的Socket名

    UPROPERTY()
    FTransform RelativeTransform; // 装备时相对于父对象的偏移

    UPROPERTY()
    int32 CurrentAmmo = 0;        // 弹药数（武器专用，其他Actor可忽略）

    // 武器配件状态
    UPROPERTY()
    bool bCompensatorEquipped = false;
    UPROPERTY()
    bool bSilencerEquipped = false;
    UPROPERTY()
    bool bScopeEquipped = false;
    UPROPERTY()
    bool bLightAttachmentEquipped = false;
    UPROPERTY()
    bool bFlashlightBeamOn = false;
};

UINTERFACE(MinimalAPI, Blueprintable)
class USaveableActor : public UInterface
{
    GENERATED_BODY()
};

class ZOMBIETEMPLATE_API ISaveableActor
{
    GENERATED_BODY()

public:
    /** 返回此 Actor 在场景中的唯一标识符 */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FName GetUniqueSaveID() const;

    /** 获取当前状态，打包成 FActorSaveData */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    FActorSaveData GetSaveData() const;

    /** 从给定的数据中恢复状态（弹药、配件等，不包含位置和装备信息） */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void RestoreState(const FActorSaveData& Data);

    /** 重置到关卡设计的默认状态（位置、装备状态、配件、弹药等） */
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void ResetToDefault();
};