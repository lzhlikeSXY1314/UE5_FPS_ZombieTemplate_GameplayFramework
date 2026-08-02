// WeaponBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/InspectableItem.h"
#include "Particles/ParticleSystem.h"
#include "Items/ProjectileBase.h"
#include <Components/SpotLightComponent.h>
#include <Datas/WeaponData.h>
#include "SaveSystem/SaveableActor.h" 
#include "WeaponBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmmoEmptyStateChanged, bool, bIsEmpty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, bool, Equip);

UCLASS()
class ZOMBIETEMPLATE_API AWeaponBase : public AInspectableItem
{
    GENERATED_BODY()

public:
    AWeaponBase();
    void OnConstruction(const FTransform& Transform);
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void DiscardItemInInventory_Implementation(int32 Quantity) override;
  


    // ---------- 数据资产 ----------
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    TObjectPtr<UWeaponData> WeaponData;

    // ---------- 委托 ----------
    UPROPERTY(BlueprintAssignable, Category = "Interact")
    FOnInteract OnInteract;


#pragma region SaveActorInterface
public:
    // 接口实现
    virtual FName GetUniqueSaveID_Implementation() const override;
    virtual FActorSaveData GetSaveData_Implementation() const override;
    virtual void RestoreState_Implementation(const FActorSaveData& Data) override;
    virtual void ResetToDefault_Implementation() override;

    void RefreshAttachmentVisuals();
    void SoftReset();
private:
    FTransform OriginalWorldTransform;   // 记录关卡设计时的位置

#pragma endregion 


    // ========================================================================
    //  Region: 组件与 Socket
    // ========================================================================
#pragma region Components & Sockets
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName AttachBackSocketName = "Backpack_Sckt";


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|VFX")
    FName MuzzleSocketName = "Muzzle";
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Components")
    FName ScopeSocketName = "HoloSight_Socket";
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Components")
    FName ScopeSlotCoverSocketName = "HoloSight_Socket";
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Components")
    FName FlashlightSocketName = "FashLight_Socket";
#pragma endregion

    // ========================================================================
    //  Region: 交互与装备状态
    // ========================================================================
#pragma region Interaction & Equip State
public:
    bool CanDirEquipped = true;

    virtual void OnInteract_Implementation(AActor* Interactor) override;

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    bool IsEquipped() const { return InventoryItemPayload.IsEquipped; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Equip(AActor* NewOwner);

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void UnEquip(AActor* NewOwner);

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Drop();
#pragma endregion

    // ========================================================================
    //  Region: 弹药与换弹
    // ========================================================================
#pragma region Ammo & Reload
public:
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Reload")
    bool bIsReloading = false;

    UPROPERTY(BlueprintAssignable, Category = "Weapon|Ammo")
    FOnAmmoEmptyStateChanged OnAmmoEmptyStateChanged;

    UFUNCTION(BlueprintCallable, Category = "Weapon|Ammo")
    void GetAmmoInfo(int32& OutCurrent, int32& OutMax) const;

    void ConsumeAmmo();
    void StartReload();
    void StopReload();

    UFUNCTION()
    void OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
    
    bool bLastIsEmpty = false;
    void CheckAmmoStateChange();
#pragma endregion

    // ========================================================================
    //  Region: 伤害与断肢
    // ========================================================================
#pragma region Damage & Dismember
public:
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Damage")
    bool bCanDismember = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Damage")
    int32 CompensatorDismemberPower = 2;

    int32 DismemberPower = 1;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon|Damage")
    float GetCurrentDamage() const;
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Weapon|Damage")
    bool CanDismember() const { return bCanDismember; }

private:
    int32 OriginalDismemberPower = 1;
#pragma endregion

    // ========================================================================
    //  Region: 配件装备标志
    // ========================================================================
#pragma region Attachment State
public:
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Compensator")
    bool bCompensatorEquipped = false;
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Silencer")
    bool bSilencerEquipped = false;
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Scope")
    bool bScopeEquipped = false;

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

    // ========================================================================
    //  Region: 手电筒与激光
    // ========================================================================
#pragma region Flashlight & Laser
public:
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Light")
    bool bFlashlightBeamOn = false;
    UPROPERTY(BlueprintReadOnly, Category = "Weapon|Light")
    bool bLightAttachmentEquipped = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Light")
    float LaserDotDepthOffset = 2.0f;
    UPROPERTY(EditAnywhere, Category = "Weapon|Light")
    bool bDrawLaserDebug = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Light")
    USoundBase* SwitchLightSound;

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

    // ========================================================================
    //  Region: 射击、视觉特效与弹道
    // ========================================================================
#pragma region Fire & Visual FX
public:
    // 烟雾池
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Components")
    TArray<UParticleSystemComponent*> MuzzleSmokePool;

    FTimerHandle SmokeDelayTimer;
    int32 ConsecutiveShots = 0;
    float LastShotTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Decal")
    TSubclassOf<AActor> BulletHoleDecalClass;

    void ActivatePooledSmoke();
    UFUNCTION()
    void OnPooledSmokeFinished(UParticleSystemComponent* PSC);

    FVector GetCurrentMuzzleFlashOffset() const;
    FVector GetCurrentMuzzleSmokeOffset() const;
    void FireWeaponVisuals(const FVector& MuzzleLocation, const FRotator& AimRotation, bool bIsAiming, USkeletalMeshComponent* HandsMesh);
    void ProcessHit(const FHitResult& Hit);
    void ClearMuzzleSmokePool();
private:
    float LastSmokeTime = 0.0f;

    // ---- 弹道 ----
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

    // ========================================================================
    //  Region: 散布/后坐力读取（从 DataAsset 获取）
    // ========================================================================
#pragma region Recoil & Spread
public:
    float GetCurrentHipSpread() const;
    float GetCurrentADSSpread() const;
    FVector2D GetCurrentRecoilYaw() const;
    FVector2D GetCurrentRecoilPitch() const;
#pragma endregion

    // ========================================================================
    //  Region: UI 图标
    // ========================================================================
#pragma region Weapon Icon
public:
    UFUNCTION(BlueprintCallable, Category = "Weapon|UI")
    void GetWeaponIconAndScale(UTexture2D*& OutIcon, float& OutScale) const;
#pragma endregion

    // ========================================================================
    //  Region: 弹壳抛出
    // ========================================================================
#pragma region Shell Ejection
public:
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void ActivateCaseEject();
#pragma endregion

    // ========================================================================
    //  Region: 内部辅助
    // ========================================================================
#pragma region Internal Helpers
private:
    USoundBase* OriginalFireSound = nullptr;

    void SaveBaseAttributes();
    void RestoreBaseAttributes();
    void DisableWeaponPhysics();
    void EnableWeaponPhysics();
  
#pragma endregion
};