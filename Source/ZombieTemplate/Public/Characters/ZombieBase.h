// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/ZombieAnimationProvider.h"

#include <Components/TimelineComponent.h>

#include "ZombieBase.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, bool, Death);

// 命中方向枚举
UENUM(BlueprintType)
enum class EHitDirection : uint8
{
    Front UMETA(DisplayName = "Front"),
    Right UMETA(DisplayName = "Right"),
    Left  UMETA(DisplayName = "Left"),
    Back  UMETA(DisplayName = "Back")
};

// E_ChestStrikePosition 枚举
UENUM(BlueprintType)
enum class EChestStrikePosition : uint8
{
    Spine_Left   UMETA(DisplayName = "Spine_Left"),  
    Spine_Right  UMETA(DisplayName = "Spine_Right"), 
    Spine_Middle UMETA(DisplayName = "Spine_Middle"),
};


UCLASS()
class ZOMBIETEMPLATE_API AZombieBase : public ACharacter, public IZombieAnimationProvider
{
    GENERATED_BODY()

    // ========================================================================
    // 构造 / 生命周期
    // ========================================================================
public:
    AZombieBase();

    UPROPERTY(BlueprintAssignable, Category = "Death")
    FOnDeath OnDeath;

    USkeletalMeshComponent* TargetDismemberMesh = nullptr;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // ========================================================================
    // 动画接口实现
    // ========================================================================
protected:
    virtual UAnimSequenceBase* GetIdleAnimation_Implementation() const override;
    virtual UAnimSequenceBase* GetWalkAnimation_Implementation() const override;
 
    

    // ========================================================================
    // Region: Locomotion（移动动画）
    // ========================================================================
#pragma region Locomotion
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    UAnimSequenceBase* Animation_Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Locomotion")
    UAnimSequenceBase* Animation_Walk;
#pragma endregion Locomotion

    // ========================================================================
    // Region: Dismember（肢解系统）
    // ========================================================================
#pragma region Dismember
public:
    // 死亡时是否允许肢解（true = 死亡时仍然肢解，false = 死亡时不肢解）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    bool bDismemberOnDeath = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dismember")
    class UGoreComponent* DismemberGoreComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    int32 HeadBreakBullets = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    int32 RightArmBreakBullets = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    int32 LeftArmBreakBullets = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    int32 RightLegBreakBullets = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    int32 LeftLegBreakBullets = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    FName HeadBoneName = "head";

    // 可以在蓝图中选择的碎片 Actor 类
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    TSubclassOf<AActor> HeadFragmentClass;

    UFUNCTION(BlueprintCallable, Category = "Dismember")
    void DismemberLimb(int32 DismemberPower, bool bCanDismember);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dismember")
    void SpawnHeadFragment();

    UFUNCTION(BlueprintCallable, Category = "Dismember")
    USkeletalMeshComponent* FindMeshByTag(FName Tag);

    

#pragma endregion Dismember

    // ========================================================================
    // Region: BoneNames（骨骼名称配置）
    // ========================================================================
#pragma region BoneNames
public:
    // ---- 胸部源数组 ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneNames|Chest")
    TArray<FName> HitUpperChestNameArr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneNames|Chest")
    TArray<FName> HitMidChestNameArr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneNames|Chest")
    TArray<FName> HitLowerChestNameArr;

    // ---- 四肢源数组 ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneNames|Limbs")
    TArray<FName> HitFullRightHandNameArr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneNames|Limbs")
    TArray<FName> HitFullLeftHandNameArr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneNames|Limbs")
    TArray<FName> HitFullRightFootNameArr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoneNames|Limbs")
    TArray<FName> HitFullLeftFootNameArr;

    // ---- 合并后的输出数组 ----
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BoneNames|Chest")
    TArray<FName> AllChestName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BoneNames|Limbs")
    TArray<FName> AllLimbsName;

    UFUNCTION(BlueprintCallable, Category = "BoneNames")
    void InitializeBoneNames();
#pragma endregion BoneNames

    // ========================================================================
    // Region: BloodDecal（血迹贴花）
    // ========================================================================
#pragma region BloodDecal
public:
    UFUNCTION(BlueprintCallable, Category = "Hit/Damage")
    void SpawnBloodDecal();

    void InitializeDecalComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit/Damage")
    FVector2D DecalSize = FVector2D(5.0f, 10.0f);

private:
    class USkinnedDecalSampler* SkinnedDecalSampler = nullptr;
#pragma endregion BloodDecal

    // ========================================================================
    // Region: HitDirection（命中方向计算）
    // ========================================================================
#pragma region HitDirection
public:
    UFUNCTION(BlueprintCallable, Category = "Hit/Damage")
    void CalculateHitDirection();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit/Damage")
    double MiddleAngleThreshold = 45.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit/Damage")
    double BackAngleThreshold = 135.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit/Damage")
    float ImpulseMagnitude = 1000.0f;
    
private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit/Damage", meta = (AllowPrivateAccess = "true"))
    EHitDirection HitDirection = EHitDirection::Front;
#pragma endregion HitDirection

#pragma region ChestStrike
public:
    UFUNCTION(BlueprintCallable, Category = "Hit/Damage")
    void CalculateChestStrikePosition();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit/Damage")
    double SplitArea = 6.0f;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit/Damage", meta = (AllowPrivateAccess = "true"))
    EChestStrikePosition E_ChestStrikePosition = EChestStrikePosition::Spine_Left;
#pragma endregion ChestStrike


    // ========================================================================
    // Region: Combat（战斗系统）
    // ========================================================================
#pragma region Combat
public:
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CritSuccess() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void HandleDeathFunc(FVector InShotFromDirection);

    UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    double CritChance = 0.0;

    // 尸体存在时间（秒），超时自动销毁。可在蓝图中为不同丧尸设置不同值
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Death")
    float CorpseLifeSpan = -1.0f;  //-1不销毁

    /** 最大生命值 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Health")
    float MaxHealth = 100.0f;

    /** 当前生命值 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Health")
    bool bDebugShowHealth = false;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsDeath = false;
#pragma endregion Combat

#pragma region PointDamage
protected:
    virtual float InternalTakePointDamage(float Damage, struct FPointDamageEvent const& PointDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

private:
    FVector HitLocation;
    FVector HitNormal;
    FName HitBoneName;
    FVector ShotFromDirection;
#pragma endregion PointDamage

#pragma region MontageCollection
public:
    // 根据命中部位和方向返回对应的蒙太奇数组
    UFUNCTION(BlueprintCallable, Category = "Hit/Damage")
    void MontageCollectionByHitPart(bool& OutIsValid, bool& OutIsHitHead);

    // 根据方向选择头部受击蒙太奇
    UFUNCTION(BlueprintCallable, Category = "Hit/Damage")
    void FindDirectionHitHeadReaction(bool& OutIsValid);

    // 辅助函数：从四肢蒙太奇数组中选择一个（对应蓝图宏 FindHitLimbsMontage）
    bool FindHitLimbsMontage(const TArray<UAnimMontage*>& MT_RightHand,
        const TArray<UAnimMontage*>& MT_LeftHand,
        const TArray<UAnimMontage*>& MT_RightFoot,
        const TArray<UAnimMontage*>& MT_LeftFoot);

    // 辅助函数：从胸部蒙太奇数组中选择一个（对应蓝图宏 FindHitChestMontage）
    bool FindHitChestMontage(const TArray<UAnimMontage*>& UpperChestMontage,
        const TArray<UAnimMontage*>& MidChestMontage,
        const TArray<UAnimMontage*>& LowerChestMontage);

    UFUNCTION(BlueprintCallable, Category = "Hit/Damage")
    void CountBoneHitsAndGetDizzey(int32 Plus);

    // 眩晕阈值（命中多少次后触发眩晕）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit/Damage")
    int32 IntoHeadDizzeyCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit/Damage")
    int32 HitHeadPlus = 1;

    // 眩晕蒙太奇（触发眩晕时播放）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit/Damage")
    UAnimMontage* HitInStunned = nullptr;

    // 头部受击蒙太奇数组（按方向分类）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Head")
    TArray<UAnimMontage*> HitHeadMontage_Front;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Head")
    TArray<UAnimMontage*> HitHeadMontage_Left;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Head")
    TArray<UAnimMontage*> HitHeadMontage_Right;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Head")
    TArray<UAnimMontage*> HitHeadMontage_Back;



    // ---- 四肢蒙太奇数组（按方向分类） ----
    // Front 方向
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullRightHandMontage_Front;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullLeftHandMontage_Front;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullRightFootMontage_Front;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullLeftFootMontage_Front;

    // Back 方向
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullRightHandMontage_Back;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullLeftHandMontage_Back;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullRightFootMontage_Back;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullLeftFootMontage_Back;

    // Side 方向（左右手共用）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullRightHandMontage_Side;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullLeftHandMontage_Side;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullRightFootMontage_Side;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Limbs")
    TArray<UAnimMontage*> HitFullLeftFootMontage_Side;

    // ---- 胸部蒙太奇数组 ----
    // Spine_Left (NewEnumerator0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Front_L;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Front_L;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Front_L;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Side_L;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Side_L;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Side_L;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Back_L;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Back_L;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Back_L;

    // Spine_Right (NewEnumerator3)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Front_R;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Front_R;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Front_R;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Side_R;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Side_R;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Side_R;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Back_R;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Back_R;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Back_R;

    // Spine_Middle (NewEnumerator4)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Front_M;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Front_M;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Front_M;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitUpperChestMontage_Back_M;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitMidChestMontage_Back_M;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage|Chest")
    TArray<UAnimMontage*> HitLowerChestMontage_Back_M;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "0.5"))
    float MontageBlendOutTime = 0.25f;

private:

    // 当前选中的受击蒙太奇（只读，蓝图不可见）
    UPROPERTY(VisibleAnywhere, Category = "Montage|Head", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* CurrentHitReactionMontage = nullptr;

    // 命中计数（累计）
    int32 HitCountIntoDizzey = 0;

    bool InHitState = false;

#pragma endregion MontageCollection

#pragma region Speed
public:
    UFUNCTION(BlueprintCallable, Category = "Set Up", meta = (DisplayName = "ChangeSpeed"))
    void ChangeSpeed(float InSpeed);


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Set Up", meta = (DisplayName = "Speed"))
    float HitStateSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Set Up", meta = (DisplayName = "Speed"))
    float SprintSpeed = 220.0f;

    // 冲刺攻击最大距离（厘米）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|SprintAttack")
    float SprintMaxRange = 500.0f;

    // 冲刺攻击最小距离（厘米）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|SprintAttack")
    float SprintMinRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Set Up", meta = (DisplayName = "Speed"))
    float WalkSpeed = 57.0f;

#pragma endregion

#pragma region Chase
public:
    // 是否启用追击
    UPROPERTY(BlueprintReadWrite, Category = "AI")
    bool bChasePlayer = false;

    // 追击更新间隔（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float ChaseUpdateInterval = 0.1f;

    // 到达玩家的判定半径
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float ChaseAcceptanceRadius = 120.0f;

    // 开启追击
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartChase();

    // 停止追击
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopChase();

protected:
    // 定时器句柄
    FTimerHandle ChaseTimerHandle;

    // 追击循环回调
    void ChaseTick();

#pragma endregion

#pragma region Attack
public:
    // 攻击范围（厘米）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AttackRange = 50.0f;

    // 攻击蒙太奇数组（随机选择）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TArray<UAnimMontage*> AttackMontages;

    // 是否正在攻击
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    bool bIsAttacking = false;

    // 冲刺攻击蒙太奇数组
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|SprintAttack")
    TArray<UAnimMontage*> SprintAttackMontages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|SprintAttack")
    float SprintCooldown = 12.0f;

    float LastSprintTime = -SprintCooldown;

    bool bIsSprintAttacking = false;

    virtual bool IsSprintAttacking_Implementation() const override { return bIsSprintAttacking; }

    virtual bool SetIsSprintAttacking_Implementation(bool bNewValue) override;


    
 
    virtual void EnableAttackCollisionDetection_Implementation() override;
    UFUNCTION()
    void OnAnyMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attack")
    FName AttackSocketLeft = "hand_l";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attack")
    FName AttackSocketRight = "hand_r";

    // 攻击球形检测半径
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attack")
    float AttackTraceRadius = 50.0f;

    // 执行攻击伤害检测（由动画通知或接口调用）
    UFUNCTION(BlueprintCallable, Category = "AI|Attack")
    void PerformAttackDamage();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attack")
    float AttackDamage = 20.0f;



private:
    // 上次攻击命中的 Pawn（防止同一次攻击多次命中）
    UPROPERTY()
    APawn* AlreadyAttackPawn = nullptr;

    // 内部辅助函数：检测并伤害目标
    void CheckAttackHit(FName SocketName);


#pragma endregion

#pragma region PassiveAvoidence
public: 
    virtual void ApplyCrowdPush_Implementation(float InElbowPushStrength, float InForwardStrength) override;


    FVector CalculateDirection(AActor* Target) const;

    void TraceCheck(float Rotate, float InElbowPushStrength,float InForwardStrength);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    float TraceCheckDistance = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    TEnumAsByte<ETraceTypeQuery> AvoidanceTraceChannel = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel3);


    UFUNCTION(BlueprintCallable)
    void PassiveAvoidanceCheck(float InElbowPushStrength, float InForwardStrength);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    TArray<float> AvoidanceCheckAngles = { 0.0f, 30.0f, 60.0f, 90.0f, -30.0f, -60.0f, -90.0f };

    /** 是否启用速度滑步效果（对应原组件 Speed Up 功能） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance")
    bool bEnableSpeedUp = false;

    bool bAvoidance = false;

    /** 执行一次基于速度的世界偏移（应在 Tick 中调用） */
    void SpeedUp(float InForwardStrength);

    /** 播放后退蒙太奇（对应蓝图 "Retreat to make space" 事件） */
    UFUNCTION(BlueprintCallable, Category = "Avoidance")
    void PlayRetreatToMakeSpace();

    void OnRetreatBlendOut(UAnimMontage* Montage, bool bInterrupted);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance|Montage")
    UAnimMontage* RetreatMontage;

    /** 后退动画结束后的攻击冷却时间（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Avoidance|Montage")
    float RetreatCooldown = 1.5f;

    float LastRetreatTime = -1.5f;   // 初始值保证一开始可攻击
private:

#pragma endregion



};