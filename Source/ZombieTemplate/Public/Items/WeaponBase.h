// WeaponBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/InspectableItem.h"
#include "Particles/ParticleSystem.h"
#include "Items/ProjectileBase.h"
#include <Components/SpotLightComponent.h>
#include "WeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoEmptyStateChanged, bool, bIsEmpty);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, bool, Equip);

//FWeaponRecoilSpread
USTRUCT(BlueprintType)
struct FWeaponRecoilSpread
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
    FVector2D RecoilYaw = FVector2D(-0.5f, 0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
    FVector2D RecoilPitch = FVector2D(-0.7f, 0.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
    bool bUseRecoil = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread")
    float HipBaseSpread = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread")
    float HipMovementSpread = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread")
    float ADSBaseSpread = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spread")
    float ADSMovementSpread = 0.005f;

    // 配件乘数也放在这里（它们本来就是用来调整散布和后坐力的）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments")
    float CompensatorSpreadMultiplier = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments")
    float CompensatorRecoilMultiplier = 0.7f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments")
    float SilencerSpreadMultiplier = 1.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments")
    float SilencerRecoilMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments")
    float LaserHipSpreadMultiplier = 0.3f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments")
    float LaserADSSpreadMultiplier = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments")
    float ScopeADSSpreadMultiplier = 0.5f;
};

//FWeaponVisualFX 
USTRUCT(BlueprintType)
struct FWeaponVisualFX
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* FireSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundAttenuation* FireSoundAttenuation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundConcurrency* FireSoundConcurrency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* NoImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UParticleSystem* MuzzleFlashParticle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector MuzzleScale = FVector(0.5f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector MuzzleFlashOffset_Default = FVector(45.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector MuzzleFlashOffset_Compensator = FVector(55.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector MuzzleFlashOffset_Silencer = FVector(65.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UParticleSystem* MuzzleSmokeParticle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector MuzzleSmokeScale = FVector(0.25f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector MuzzleSmokeOffset_Default = FVector(5.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector MuzzleSmokeOffset_Silencer = FVector(20.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector MuzzleSmokeOffset_Scope = FVector(10.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float FireRange = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* CaseEjectImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_GameTraceChannel1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
    TSubclassOf<AActor> BulletHoleDecalClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* EmptyMagSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    int32 MuzzleSmokePoolSize = 3;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    float SmokeDelay = 0.1f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    float ShotCounterResetTime = 2.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    int32 ShotsToStartSmoke = 5;
};

USTRUCT(BlueprintType)
struct FWeaponDamage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float Damage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    TMap<FName, float> BoneDamageMultipliers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    UCurveFloat* DamageFalloffCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float MinimumDamage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachments")
    float SilencerDamageMultiplier = 0.8f;
};

USTRUCT(BlueprintType)
struct FWeaponAttachmentConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compensator")
    USoundBase* CompensatorFireSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Compensator")
    TSubclassOf<AActor> CompensatorDropClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Silencer")
    USoundBase* SilencerFireSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Silencer")
    TSubclassOf<AActor> SilencerDropClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scope")
    FVector ArmsLocCorrection = FVector(0.0f, 0.0f, -1.484848f);
};

USTRUCT(BlueprintType)
struct FWeaponAnimMontageSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    UAnimMontage* FireMontage_Hip_Hands;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    UAnimMontage* FireMontage_ADS_Hands;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    UAnimMontage* FireMontage_Weapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reload")
    UAnimMontage* ReloadMontage_Hands;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reload")
    UAnimMontage* ReloadMontage_Weapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    UAnimMontage* SwitchLightMontage_ADS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    UAnimMontage* SwitchLightMontage_Hip;
};

USTRUCT(BlueprintType)
struct FWeaponAmmoAndUIConfig
{
    GENERATED_BODY()

    // ---- 弹药 ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
    int32 MaxAmmo = 15; 

    // ---- UI ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TArray<UTexture2D*> WeaponIconTextures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    float CompensatorIconScale = 1.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    float SilencerIconScale = 1.5f;
};


UCLASS()
class ZOMBIETEMPLATE_API AWeaponBase : public AInspectableItem
{
    GENERATED_BODY()

public:
    AWeaponBase();
    void OnConstruction(const FTransform& Transform);

    UPROPERTY(BlueprintAssignable, Category = "Interact")
    FOnInteract OnInteract;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;



#pragma region Components & Base State
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
    USkeletalMeshComponent* Weapon_SKMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Components")
    UStaticMeshComponent* CompensatorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Components")
    UStaticMeshComponent* ScopeSlotCoverMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Components")
    UStaticMeshComponent* SilencerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Components")
    UStaticMeshComponent* ScopeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Components")
    UParticleSystemComponent* CaseEjectPS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Components")
    UStaticMeshComponent* FlashlightMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Components")
    UStaticMeshComponent* LaserBodyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Components")
    UPointLightComponent* LaserDot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Components")
    USpotLightComponent* FlashlightBeam;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName AttachSocketName = "ik_RHand_Gun_Sckt";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    FName MuzzleSocketName = "Muzzle";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Components")
    FName ScopeSocketName = "HoloSight_Socket";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Components")
    FName ScopeSlotCoverSocketName = "HoloSight_Socket";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Components")
    FName FlashlightSocketName = "FashLight_Socket";

    UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    bool bIsEquipped = false;
#pragma endregion

#pragma region Interaction
public:
    virtual void OnInteract_Implementation(AActor* Interactor) override;
    bool IsEquipped() const { return bIsEquipped; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Equip(AActor* NewOwner);

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Drop();
#pragma endregion

#pragma region Fire & Visual FX
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Visual")
    FWeaponVisualFX VisualFX;

    // 烟雾粒子池（固定3个组件）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Components")
    TArray<UParticleSystemComponent*> MuzzleSmokePool;

    // 烟雾延迟触发的定时器
    FTimerHandle SmokeDelayTimer;

    int32 ConsecutiveShots = 0;                // 连续射击次数
    float LastShotTime = 0.0f;                 // 上次射击时间（秒）


    // 从池中激活空闲烟雾
    void ActivatePooledSmoke();

    // 粒子播放完毕回调
    UFUNCTION()
    void OnPooledSmokeFinished(UParticleSystemComponent* PSC);


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Decal")
    TSubclassOf<AActor> BulletHoleDecalClass;////

    FVector GetCurrentMuzzleFlashOffset() const;
    FVector GetCurrentMuzzleSmokeOffset() const;

    void FireWeaponVisuals(const FVector& MuzzleLocation, const FRotator& AimRotation, bool bIsAiming, USkeletalMeshComponent* HandsMesh);
    void ProcessHit(const FHitResult& Hit);

private:
    float LastSmokeTime = 0.0f;   // internal cooldown timer
#pragma endregion

#pragma region Ballistics & Projectiles
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
    TSubclassOf<class AProjectileBase> BulletClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Projectile")
    bool bUseProjectile = true;

    void SpawnProjectileOrHit(const FVector& MuzzleLocation, const FVector& BulletDirection, bool bHit, const FHitResult& HitResult);

    UPROPERTY(EditAnywhere, Category = "Weapon|Pool")
    int32 ProjectilePoolSize = 10;
    UPROPERTY()
    TArray<AProjectileBase*> ProjectilePool;

    AProjectileBase* GetPooledProjectile();
    void InitializeProjectilePool();
#pragma endregion

#pragma region Recoil & Spread
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
    FWeaponRecoilSpread RecoilSpread;

    float GetCurrentHipSpread() const;
    float GetCurrentADSSpread() const;
    FVector2D GetCurrentRecoilYaw() const;
    FVector2D GetCurrentRecoilPitch() const;
#pragma endregion

#pragma region Attachment Management
public:
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Compensator")
    bool bCompensatorEquipped = false;
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Silencer")
    bool bSilencerEquipped = false;
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Scope")
    bool bScopeEquipped = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Attachments")
    FWeaponAttachmentConfig AttachmentConfig;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void EquipCompensator();
    void RemoveCompensator();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Silencer")
    void EquipSilencer();
    UFUNCTION(BlueprintCallable, Category = "Weapon|Silencer")
    void RemoveSilencer();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Scope")
    void EquipScope();
    UFUNCTION(BlueprintCallable, Category = "Weapon|Scope")
    void RemoveScope();
#pragma endregion

#pragma region Flashlight & Laser
public:
    // 手电筒光束开关状态（激光不参与开关，始终亮）
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Light")
    bool bFlashlightBeamOn = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Light")
    bool bLightAttachmentEquipped = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Light")
    float LaserDotDepthOffset = 2.0f;
    UPROPERTY(EditAnywhere, Category = "Weapon|Light")
    bool bDrawLaserDebug = true;


    // ---- 切换灯光音效 ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Light")
    USoundBase* SwitchLightSound;

    // 装备/卸下整个照明附件（含手电筒和激光）
    UFUNCTION(BlueprintCallable, Category = "Weapon|Light")
    void EquipLightAttachment();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Light")
    void RemoveLightAttachment();

    UFUNCTION(BlueprintCallable, Category = "Weapon|Light")
    void ToggleFlashlight(bool bIsAiming);

    USkeletalMeshComponent* CachedHandsMesh = nullptr;
    FTimerHandle LaserDotVisibilityTimer;
    void UpdateLaserTarget();
#pragma endregion

#pragma region Animation Montages
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Animation")
    FWeaponAnimMontageSet AnimMontageSet;


#pragma endregion

#pragma region Reload
public:

    /** 是否正在换弹 */
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Reload")
    bool bIsReloading = false;


    void StartReload(int32 AmmoToAdd);

    void StopReload();   // 强制中断换弹


    /** 换弹完成回调（由蒙太奇结束通知或手动调用） */
    UFUNCTION()
    void OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted);
#pragma endregion 

#pragma region Shell Ejection
public:
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void ActivateCaseEject();
#pragma endregion 

#pragma region WeaponIcon
public:

    // 获取图标和缩放的函数（非纯函数，通过引用输出）
    UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
    void GetWeaponIconAndScale(UTexture2D*& OutIcon, float& OutScale) const;
#pragma endregion 

#pragma region Ammo
    public:
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Config")
        FWeaponAmmoAndUIConfig AmmoAndUIConfig;

        /** 当前弹药数 */
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
        int32 CurrentAmmo = 15;   // 初始满弹，可在蓝图中修改

        /** 获取弹药信息（通过引用输出两个引脚） */
        UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
        void GetAmmoInfo(int32& OutCurrent, int32& OutMax) const;

        /** 消耗一发弹药（在开火成功后调用） */
        void ConsumeAmmo();

        /** 弹药空/非空状态变化事件（参数：bIsEmpty = true 表示空仓） */
        UPROPERTY(BlueprintAssignable, Category = "Weapon|Ammo")
        FOnAmmoEmptyStateChanged OnAmmoEmptyStateChanged;

        private:
            /** 换弹完成后待补充的弹药数（动画期间暂存） */
            int32 PendingReloadAmount = 0;


#pragma region Damage
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Damage")
    FWeaponDamage DamageConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Damage")
    bool bCanDismember = false;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon|Damage")
    float GetCurrentDamage() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon|Damage")
    bool CanDismember() const { return bCanDismember; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Damage")
    int32 CompensatorDismemberPower = 2;

    int32 DismemberPower = 1;

private:
    int32 OriginalDismemberPower = 1;

#pragma endregion


private:
    bool bLastIsEmpty = false;
    void CheckAmmoStateChange();

#pragma endregion 



private:
#pragma region Internal Helpers
    USoundBase* OriginalFireSound = nullptr;

    void SaveBaseAttributes();
    void RestoreBaseAttributes();

    void DisableWeaponPhysics();
    void EnableWeaponPhysics();
#pragma endregion
};