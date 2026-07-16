// ZombieBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/ZombieAnimationProvider.h"
#include <Components/TimelineComponent.h>
#include <Datas/ZombieData.h>
#include "SaveSystem/SaveableActor.h"
#include "ZombieBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, bool, Death);

UENUM(BlueprintType)
enum class EHitDirection : uint8
{
    Front UMETA(DisplayName = "Front"),
    Right UMETA(DisplayName = "Right"),
    Left  UMETA(DisplayName = "Left"),
    Back  UMETA(DisplayName = "Back")
};

UENUM(BlueprintType)
enum class EChestStrikePosition : uint8
{
    Spine_Left   UMETA(DisplayName = "Spine_Left"),
    Spine_Right  UMETA(DisplayName = "Spine_Right"),
    Spine_Middle UMETA(DisplayName = "Spine_Middle"),
};

UCLASS()
class ZOMBIETEMPLATE_API AZombieBase : public ACharacter, public IZombieAnimationProvider, public ISaveableActor
{
    GENERATED_BODY()

public:
    AZombieBase();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    TObjectPtr<UZombieData> ZombieData;

    UPROPERTY(BlueprintAssignable, Category = "Death")
    FOnDeath OnDeath;

    USkeletalMeshComponent* TargetDismemberMesh = nullptr;

    //----------存档接口----------//
    virtual FName GetUniqueSaveID_Implementation() const override;
    virtual FActorSaveData GetSaveData_Implementation() const override;
    virtual void RestoreState_Implementation(const FActorSaveData& Data) override;
    virtual void ResetToDefault_Implementation() override;


protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual UAnimSequenceBase* GetIdleAnimation_Implementation() const override;
    virtual UAnimSequenceBase* GetWalkAnimation_Implementation() const override;

    // ========================================================================
    // Region: Dismember（肢解系统）
    // ========================================================================
#pragma region Dismember
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dismember")
    class UGoreComponent* DismemberGoreComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    int32 HeadBreakBullets = 4;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    int32 RightArmBreakBullets = 3;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    int32 LeftArmBreakBullets = 3;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    int32 RightLegBreakBullets = 3;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dismember")
    int32 LeftLegBreakBullets = 3;

    UFUNCTION(BlueprintCallable, Category = "Dismember")
    void DismemberLimb(int32 DismemberPower, bool bCanDismember);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Dismember")
    void SpawnHeadFragment();

    bool HasBreakHead = false;

    UFUNCTION(BlueprintCallable, Category = "Dismember")
    USkeletalMeshComponent* FindMeshByTag(FName Tag);
#pragma endregion Dismember

    // ========================================================================
    // Region: BoneNames（骨骼名称配置）
    // ========================================================================
#pragma region BoneNames
public:
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

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hit/Damage", meta = (AllowPrivateAccess = "true"))
    EHitDirection HitDirection = EHitDirection::Front;
#pragma endregion HitDirection

    // ========================================================================
    // Region: ChestStrike
    // ========================================================================
#pragma region ChestStrike
public:
    UFUNCTION(BlueprintCallable, Category = "Hit/Damage")
    void CalculateChestStrikePosition();

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Health")
    bool bDebugShowHealth = false;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
    bool bIsDeath = false;
#pragma endregion Combat

    // ========================================================================
    // Region: PointDamage
    // ========================================================================
#pragma region PointDamage
protected:
    virtual float InternalTakePointDamage(float Damage, struct FPointDamageEvent const& PointDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

private:
    FVector HitLocation;
    FVector HitNormal;
    FName HitBoneName;
    FVector ShotFromDirection;
#pragma endregion PointDamage

    // ========================================================================
    // Region: MontageCollection
    // ========================================================================
#pragma region MontageCollection
public:
    UFUNCTION(BlueprintCallable, Category = "Hit/Damage")
    void MontageCollectionByHitPart(bool& OutIsValid, bool& OutIsHitHead);

    UFUNCTION(BlueprintCallable, Category = "Hit/Damage")
    void FindDirectionHitHeadReaction(bool& OutIsValid);

    bool FindHitLimbsMontage(const TArray<UAnimMontage*>& MT_RightHand,
        const TArray<UAnimMontage*>& MT_LeftHand,
        const TArray<UAnimMontage*>& MT_RightFoot,
        const TArray<UAnimMontage*>& MT_LeftFoot);

    bool FindHitChestMontage(const TArray<UAnimMontage*>& UpperChestMontage,
        const TArray<UAnimMontage*>& MidChestMontage,
        const TArray<UAnimMontage*>& LowerChestMontage);

    UFUNCTION(BlueprintCallable, Category = "Hit/Damage")
    void CountBoneHitsAndGetDizzey(int32 Plus);

private:
    UPROPERTY(VisibleAnywhere, Category = "Montage|Head", meta = (AllowPrivateAccess = "true"))
    UAnimMontage* CurrentHitReactionMontage = nullptr;

    int32 HitCountIntoDizzey = 0;
    bool InHitState = false;
#pragma endregion MontageCollection

    // ========================================================================
    // Region: Speed
    // ========================================================================
#pragma region Speed
public:
    UFUNCTION(BlueprintCallable, Category = "Set Up", meta = (DisplayName = "ChangeSpeed"))
    void ChangeSpeed(float InSpeed);
#pragma endregion Speed

    // ========================================================================
    // Region: Chase
    // ========================================================================
#pragma region Chase
public:
    UPROPERTY(BlueprintReadWrite, Category = "AI")
    bool bChasePlayer = false;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartChase();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopChase();

protected:
    FTimerHandle ChaseTimerHandle;
    void ChaseTick();
#pragma endregion Chase

    // ========================================================================
    // Region: Attack
    // ========================================================================
#pragma region Attack
public:
    UPROPERTY(BlueprintReadOnly, Category = "AI")
    bool bIsAttacking = false;

    float LastSprintTime = -1.5f;
    bool bIsSprintAttacking = false;

    virtual bool IsSprintAttacking_Implementation() const override { return bIsSprintAttacking; }
    virtual bool SetIsSprintAttacking_Implementation(bool bNewValue) override;

    virtual void EnableAttackCollisionDetection_Implementation() override;

    UFUNCTION()
    void OnAnyMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION(BlueprintCallable, Category = "AI|Attack")
    void PerformAttackDamage();

private:
    UPROPERTY()
    APawn* AlreadyAttackPawn = nullptr;

    void CheckAttackHit(FName SocketName);
#pragma endregion Attack

    // ========================================================================
    // Region: PassiveAvoidence
    // ========================================================================
#pragma region PassiveAvoidence
public:
    virtual void ApplyCrowdPush_Implementation(float InElbowPushStrength, float InForwardStrength) override;

    FVector CalculateDirection(AActor* Target) const;
    void TraceCheck(float Rotate, float InElbowPushStrength, float InForwardStrength);

    UFUNCTION(BlueprintCallable)
    void PassiveAvoidanceCheck(float InElbowPushStrength, float InForwardStrength);

    bool bAvoidance = false;
    void SpeedUp(float InForwardStrength);

    UFUNCTION(BlueprintCallable, Category = "Avoidance")
    void PlayRetreatToMakeSpace();

    void OnRetreatBlendOut(UAnimMontage* Montage, bool bInterrupted);

    float LastRetreatTime = -1.5f;
#pragma endregion PassiveAvoidence
};