// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZombieData.generated.h"

// 1. 生命与死亡
USTRUCT(BlueprintType)
struct FZombieCombatConfig
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float MaxHealth = 100.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float CorpseLifeSpan = -1.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    double CritChance = 0.0;
};

// 2. 移动速度
USTRUCT(BlueprintType)
struct FZombieMovementConfig
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float WalkSpeed = 57.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float SprintSpeed = 220.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float HitStateSpeed = 0.0f;
};

// 3. 追逐配置
USTRUCT(BlueprintType)
struct FZombieChaseConfig
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chase")
    float ChaseUpdateInterval = 0.1f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chase")
    float ChaseAcceptanceRadius = 60.0f;
};

// 4. 攻击配置
USTRUCT(BlueprintType)
struct FZombieAttackConfig
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    float AttackRange = 50.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    float AttackTraceRadius = 50.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    float AttackDamage = 20.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    FName AttackSocketLeft = "hand_l";
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    FName AttackSocketRight = "hand_r";
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    TArray<UAnimMontage*> AttackMontages;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    TArray<UAnimMontage*> LeftAttackMontages;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    TArray<UAnimMontage*> RightAttackMontages;
};

// 5. 冲刺攻击配置
USTRUCT(BlueprintType)
struct FZombieSprintAttackConfig
{
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SprintAttack")
    bool bEnableSprintAttack = true;

    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SprintAttack")
    float SprintMinRange = 300.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SprintAttack")
    float SprintMaxRange = 500.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SprintAttack")
    float SprintCooldown = 12.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SprintAttack")
    TArray<UAnimMontage*> SprintAttackMontages;
};

// 6. 回避配置
USTRUCT(BlueprintType)
struct FZombieAvoidanceConfig
{
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Avoidance")
    bool bEnableRetreat = true;

    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Avoidance")
    float TraceCheckDistance = 40.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Avoidance")
    TEnumAsByte<ETraceTypeQuery> AvoidanceTraceChannel = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel3);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Avoidance")
    TArray<float> AvoidanceCheckAngles = { 0.0f, 30.0f, 60.0f, 90.0f, -30.0f, -60.0f, -90.0f };
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Avoidance")
    bool bEnableSpeedUp = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Avoidance")
    UAnimMontage* RetreatMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Avoidance")
    float RetreatCooldown = 1.5f;
};

// 7. 受击反应基础配置（不含蒙太奇）
USTRUCT(BlueprintType)
struct FZombieHitReactionBaseConfig
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReaction")
    double MiddleAngleThreshold = 45.0;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReaction")
    double BackAngleThreshold = 135.0;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReaction")
    float ImpulseMagnitude = 1000.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReaction")
    double SplitArea = 6.0f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReaction")
    int32 IntoHeadDizzeyCount = 3;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReaction")
    int32 HitHeadPlus = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReaction")
    UAnimMontage* HitInStunned = nullptr;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReaction")
    float MontageBlendOutTime = 0.25f;
};

// 8. 受击蒙太奇（按方向分类）—— 保持你原有的命名方式
USTRUCT(BlueprintType)
struct FZombieHitMontages
{
    GENERATED_BODY()

    // 头部
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Head")
    TArray<UAnimMontage*> HitHeadMontage_Front;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Head")
    TArray<UAnimMontage*> HitHeadMontage_Left;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Head")
    TArray<UAnimMontage*> HitHeadMontage_Right;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Head")
    TArray<UAnimMontage*> HitHeadMontage_Back;

    // 四肢 - Front
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullRightHandMontage_Front;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullLeftHandMontage_Front;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullRightFootMontage_Front;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullLeftFootMontage_Front;

    // 四肢 - Back
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullRightHandMontage_Back;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullLeftHandMontage_Back;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullRightFootMontage_Back;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullLeftFootMontage_Back;

    // 四肢 - Side
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullRightHandMontage_Side;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullLeftHandMontage_Side;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullRightFootMontage_Side;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<UAnimMontage*> HitFullLeftFootMontage_Side;

    // 胸部 - 保留你原来的命名模式
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Front_L;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Front_L;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Front_L;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Side_L;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Side_L;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Side_L;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Back_L;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Back_L;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Back_L;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Front_R;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Front_R;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Front_R;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Side_R;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Side_R;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Side_R;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Back_R;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Back_R;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Back_R;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Front_M;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Front_M;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Front_M;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Back_M;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Back_M;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Back_M;
};


// 9. 骨骼名称
USTRUCT(BlueprintType)
struct FZombieBoneNames
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<FName> HitUpperChestNameArr;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<FName> HitMidChestNameArr;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chest")
    TArray<FName> HitLowerChestNameArr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<FName> HitFullRightHandNameArr;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<FName> HitFullLeftHandNameArr;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<FName> HitFullRightFootNameArr;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Limbs")
    TArray<FName> HitFullLeftFootNameArr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Head")
    FName HeadBoneName = "head";
};

// 10. 肢解配置
USTRUCT(BlueprintType)
struct FZombieDismemberConfig
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dismember")
    bool bDismemberOnDeath = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dismember")
    int32 HeadBreakBullets = 2;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dismember")
    int32 RightArmBreakBullets = 3;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dismember")
    int32 LeftArmBreakBullets = 3;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dismember")
    int32 RightLegBreakBullets = 3;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dismember")
    int32 LeftLegBreakBullets = 3;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dismember")
    TSubclassOf<AActor> HeadFragmentClass;
};

// 11. 血迹贴花
USTRUCT(BlueprintType)
struct FZombieDecalConfig
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal")
    FVector2D DecalSize = FVector2D(5.0f, 10.0f);
};

// 12. 基础动画
USTRUCT(BlueprintType)
struct FZombieAnimationConfig
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimSequenceBase* Animation_Idle;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimSequenceBase* Animation_Walk;
};


UCLASS()
class ZOMBIETEMPLATE_API UZombieData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    FZombieCombatConfig CombatConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    FZombieMovementConfig MovementConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chase")
    FZombieChaseConfig ChaseConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
    FZombieAttackConfig AttackConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SprintAttack")
    FZombieSprintAttackConfig SprintAttackConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Avoidance")
    FZombieAvoidanceConfig AvoidanceConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReaction")
    FZombieHitReactionBaseConfig HitReactionBase;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitReaction")
    FZombieHitMontages HitMontages;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Bones")
    FZombieBoneNames BoneNames;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dismember")
    FZombieDismemberConfig DismemberConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decal")
    FZombieDecalConfig DecalConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    FZombieAnimationConfig AnimationConfig;
};
