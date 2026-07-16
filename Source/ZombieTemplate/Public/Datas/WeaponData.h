// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponData.generated.h"



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
    TSubclassOf<AActor> ScopeDropClass;

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
class ZOMBIETEMPLATE_API UWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    FWeaponRecoilSpread RecoilSpread;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
    FWeaponVisualFX VisualFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
    FWeaponDamage DamageConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attachments")
    FWeaponAttachmentConfig AttachmentConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    FWeaponAnimMontageSet AnimMontageSet;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo/UI")
    FWeaponAmmoAndUIConfig AmmoAndUIConfig;



};
