// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ZombiePlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include <EnhancedInputSubsystems.h>
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include <Components/PlayerInteractionComponent.h>
#include <Interface/Interactable.h>
#include "Items/WeaponBase.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include <Kismet/GameplayStatics.h>
#include "Components/AudioComponent.h"
#include "Camera/PlayerCameraManager.h"

#include "Kismet/KismetSystemLibrary.h" // 必须加这个头文件
#include "InventorySystem/Components/InventoryHUDComponent.h"





// Sets default values
AZombiePlayer::AZombiePlayer()
{


    // Create and attach Camera
    FPS_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPS_Camera"));
    FPS_Camera->SetupAttachment(RootComponent); //修改GetMesh()
    FPS_Camera->SetRelativeLocation(FVector(0.0f, -0.0f, 65.5f));
    FPS_Camera->SetFieldOfView(70.0f);
    FPS_Camera->bUsePawnControlRotation = true;

    FPSSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSSkeletalMesh"));
    FPSSkeletalMesh->SetupAttachment(FPS_Camera);
    FPSSkeletalMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -165.5f));
    FPSSkeletalMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
    FPSSkeletalMesh->SetOnlyOwnerSee(true);
    FPSSkeletalMesh->SetCastShadow(false);
    FPSSkeletalMesh->ComponentTags.AddUnique(FName("FPSMesh"));




    // Character movement settings
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;


    FOVTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("FOVTimeline"));
    FOVTimelineCallback.BindUFunction(this, FName("UpdateFOV"));

    // 创建一个音频组件用于 ADS 音效，避免 PlaySoundAtLocation 产生重叠
    ADSAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ADSAudioComponent"));
    ADSAudioComponent->bAutoActivate = false;
    ADSAudioComponent->SetVolumeMultiplier(1.0f);

    //库存组件
    InventoryComponent = CreateDefaultSubobject<UInventoryHUDComponent>(TEXT("InventoryComponent"));

}

// Called when the game starts or when spawned
void AZombiePlayer::BeginPlay()
{
	Super::BeginPlay();
            if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
            {
                MovementComp->MaxWalkSpeed = NormalWalkSpeed;
            }

            if (APlayerController* PC = Cast<APlayerController>(Controller))
            {
                if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                    ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
                {
                    if (DefaultMappingContext)
                    {
                        Subsystem->AddMappingContext(DefaultMappingContext, 0);
                    }
                }
            }

            // 只有本地控制的角色才显示准星（多人游戏或 AI 不显示）
            if (IsLocallyControlled() && CrosshairWidgetClass)
            {
                CrosshairWidget = CreateWidget<UUserWidget>(GetWorld(), CrosshairWidgetClass);
                if (CrosshairWidget)
                {
                    CrosshairWidget->AddToViewport();
                }
            }

            // 设置 FOV 时间轴曲线（如果已经指定）
            if (FOVCurve)
            {
                FOVTimeline->AddInterpFloat(FOVCurve, FOVTimelineCallback);
                // 设置时间轴长度与曲线的最后一帧一致（通常曲线最后一帧的时间即为总时长）
                float MinTime, MaxTime;
                FOVCurve->GetTimeRange(MinTime, MaxTime);
                FOVTimeline->SetTimelineLength(MaxTime);
            }

            if (FPSSkeletalMesh)
            {
                DefaultArmsRelativeLocation = FPSSkeletalMesh->GetRelativeLocation();
            }

            AllOwnerWeapons.Empty();
}

// Called every frame
void AZombiePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    UpdateCameraShake();

}

// Called to bind functionality to input
void AZombiePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AZombiePlayer::Move);
        EnhancedInput->BindAction(IA_Move, ETriggerEvent::Completed, this, &AZombiePlayer::MoveEnd);
        EnhancedInput->BindAction(IA_Move, ETriggerEvent::Canceled, this, &AZombiePlayer::MoveEnd);

		EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AZombiePlayer::Look);
        EnhancedInput->BindAction(IA_Look, ETriggerEvent::Completed, this, &AZombiePlayer::LookEnd);
        EnhancedInput->BindAction(IA_Look, ETriggerEvent::Canceled, this, &AZombiePlayer::LookEnd);

        // 绑定跳跃动作
        EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Started, this, &AZombiePlayer::JumpStart);
        EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Completed, this, &AZombiePlayer::JumpStop);
    
        // 绑定交互动作
        EnhancedInput->BindAction(IA_Interact, ETriggerEvent::Started, this, &AZombiePlayer::Interact);

        EnhancedInput->BindAction(IA_DropWeapon, ETriggerEvent::Started, this, &AZombiePlayer::DropWeapon);

        EnhancedInput->BindAction(IA_Aim, ETriggerEvent::Started, this, &AZombiePlayer::StartAiming);
        EnhancedInput->BindAction(IA_Aim, ETriggerEvent::Completed, this, &AZombiePlayer::StopAiming);

        EnhancedInput->BindAction(IA_Fire, ETriggerEvent::Started, this, &AZombiePlayer::PerformFireTrace);

        // 手电筒绑定

        EnhancedInput->BindAction(IA_ToggleFlashlight, ETriggerEvent::Started, this, &AZombiePlayer::ToggleFlashlight);
        EnhancedInput->BindAction(IA_Reload, ETriggerEvent::Started, this, &AZombiePlayer::Reload);

        EnhancedInput->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AZombiePlayer::StartSprinting);
        EnhancedInput->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AZombiePlayer::StopSprinting);


        //打开库存绑定
        EnhancedInput->BindAction(IA_OpenInventory, ETriggerEvent::Triggered, this, &AZombiePlayer::OpenInventory);
    }
}

void AZombiePlayer::DropWeaponInInventory()
{
    if (!CurrentWeapon) return;

    const FString WeaponItemName = CurrentWeapon->InventoryItemPayload.ItemName;
    RemoveWeaponRefByName(CurrentWeapon->InventoryItemPayload.ItemName);
    CurrentWeapon->Drop();
    CurrentWeapon = nullptr;
    CurrentAnimState = EPlayerAnimState::NoWeapon_AnimState;
    StopAiming();
}

void AZombiePlayer::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    MoveInput = MovementVector;
    if (Controller && MovementVector.SizeSquared() > 0.f)
    {
        const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
        const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        AddMovementInput(Forward, MovementVector.Y);
        AddMovementInput(Right, MovementVector.X);


    }
}

void AZombiePlayer::MoveEnd()
{
    MoveInput = FVector2D::ZeroVector;
}

void AZombiePlayer::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxis = Value.Get<FVector2D>();
    LookInput = LookAxis;
    AddControllerYawInput(LookAxis.X);
    AddControllerPitchInput(LookAxis.Y);
}

void AZombiePlayer::LookEnd()
{
    LookInput = FVector2D::ZeroVector;
}

void AZombiePlayer::JumpStart()
{
    Jump();
    StopCurrentCameraShake();  // 停止移动晃动
    PlayCameraShake(JumpCameraShakeClass, JumpShakeScale);
}

void AZombiePlayer::JumpStop()
{
    StopJumping();
}

void AZombiePlayer::PerformFireTrace()
{
    if (!CurrentWeapon || CurrentWeapon->bIsReloading) return;



    // 空仓检查：如果弹药为0，播放空仓音效并退出（不消耗后备，也不开火）
    if (CurrentWeapon->InventoryItemPayload.AmmoAmount <= 0)
    {
        if (CurrentWeapon->WeaponData->VisualFX.EmptyMagSound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), CurrentWeapon->WeaponData->VisualFX.EmptyMagSound, GetActorLocation());
        }
        Reload();
        return;
    }


    // ---------- 获取枪口位置 ----------
    FVector MuzzleLoc = FPS_Camera->GetComponentLocation();
    if (CurrentWeapon->Weapon_SKMesh)
    {
        FVector SocketLoc = CurrentWeapon->Weapon_SKMesh->GetSocketLocation(CurrentWeapon->MuzzleSocketName);
        if (!SocketLoc.IsZero())
            MuzzleLoc = SocketLoc;
    }

    // ---------- 计算瞄准方向与散布（角度制，使用 VRandCone） ----------
    FRotator AimRot = GetControlRotation();
    FVector AimDir = AimRot.Vector();

    // 获取受附件影响的最终散布
    float Speed = FMath::Min(GetVelocity().Size(), 700.0f);
    float BaseSpreadAngle = bIsAiming ? CurrentWeapon->GetCurrentADSSpread() : CurrentWeapon->GetCurrentHipSpread();
    float MovementFactor = bIsAiming ? CurrentWeapon->WeaponData->RecoilSpread.ADSMovementSpread : CurrentWeapon->WeaponData->RecoilSpread.HipMovementSpread;
    float ActualSpreadAngle = BaseSpreadAngle + (Speed / 100.0f) * BaseSpreadAngle * MovementFactor;

    // 生成在锥角 ActualSpreadAngle 内随机方向（均匀分布）
    FVector SpreadDir = FMath::VRandCone(AimDir, FMath::DegreesToRadians(ActualSpreadAngle));

    // 射线终点
    FVector TraceEnd = MuzzleLoc + SpreadDir * CurrentWeapon->WeaponData->VisualFX.FireRange;


    // ---------- 射线检测（带开关） ----------
    FHitResult Hit;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);
    ActorsToIgnore.Add(CurrentWeapon);

    // 用 bool 控制是否显示调试射线
    EDrawDebugTrace::Type DebugType = bDrawShootTrace
        ? EDrawDebugTrace::ForDuration
        : EDrawDebugTrace::None;

    bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(),MuzzleLoc, TraceEnd, UEngineTypes::ConvertToTraceType(ECC_Visibility), false,ActorsToIgnore,DebugType,Hit,true,FLinearColor::Red,FLinearColor::Green,10.0f);


    // ---------- 计算子弹飞行方向 ----------
    FVector BulletDirection = SpreadDir;

    // ---------- 播放武器开火视觉效果 ----------
    CurrentWeapon->FireWeaponVisuals(MuzzleLoc, AimRot, bIsAiming, FPSSkeletalMesh);

    // 生成投射物或即时命中处理
    CurrentWeapon->SpawnProjectileOrHit(MuzzleLoc, BulletDirection, bHit, Hit);

    // 应用即时射线伤害
    if (bHit && Hit.GetActor())
    {
        float FinalDamage = CalculateDamage(Hit.Distance, Hit.BoneName);
        APlayerController* PC = Cast<APlayerController>(GetController());
        UGameplayStatics::ApplyPointDamage(
            Hit.GetActor(),
            FinalDamage,
            AimDir,                     // 伤害来源方向
            Hit,
            PC,
            CurrentWeapon,              // 伤害造成者（武器）
            UDamageType::StaticClass()
        );
    }


    // ---------- 后坐力 ----------
    if (CurrentWeapon->WeaponData->RecoilSpread.bUseRecoil)
    {
        AddControllerYawInput(FMath::RandRange(CurrentWeapon->WeaponData->RecoilSpread.RecoilYaw.X, CurrentWeapon->WeaponData->RecoilSpread.RecoilYaw.Y));
        AddControllerPitchInput(FMath::RandRange(CurrentWeapon->WeaponData->RecoilSpread.RecoilPitch.X, CurrentWeapon->WeaponData->RecoilSpread.RecoilPitch.Y));
    }

    // 开火晃动
    StopCurrentCameraShake();
    PlayCameraShake(FireCameraShakeClass, FireShakeScale);

    // 消耗弹药
    CurrentWeapon->ConsumeAmmo();
    OnWeaponFired.Broadcast();
}

bool AZombiePlayer::CanAim(AActor* Target) const
{
    // 空手状态不能瞄准
    if (CurrentAnimState == EPlayerAnimState::NoWeapon_AnimState)
        return false;

    // 角色在空中（跳跃/下落）不能瞄准
    if (GetCharacterMovement()->IsFalling())
        return false;


    return true;
}

void AZombiePlayer::StartSprinting()
{ 
    // 如果已经死亡或无法移动，可在此加判断
    if (!GetCharacterMovement()) return;

    // 瞄准时强制退出冲刺（确保互斥）
    if (bIsAiming) return;

    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void AZombiePlayer::StopSprinting()
{
    bIsSprinting = false;

    // 恢复为正常行走速度（因为冲刺会打断瞄准，所以不必考虑ADS速度）
    GetCharacterMovement()->MaxWalkSpeed = NormalWalkSpeed;
}

void AZombiePlayer::RemoveWeaponRefByName(FString WeaponName)
{
    for (int32 i = AllOwnerWeapons.Num() - 1; i >= 0; --i)
    {
        AWeaponBase* Weapon = AllOwnerWeapons[i];
        if (!Weapon) continue;

        if (Weapon->InventoryItemPayload.ItemName.Equals(WeaponName, ESearchCase::IgnoreCase))
        {
            AllOwnerWeapons.RemoveAt(i);
            return ; 
        }
    }
    return;
}

AWeaponBase* AZombiePlayer::FindWeaponRefByName(FString WeaponName)
{
    for (int32 i = AllOwnerWeapons.Num() - 1; i >= 0; --i)
    {
        AWeaponBase* Weapon = AllOwnerWeapons[i];
        if (!Weapon) continue;

        if (Weapon->InventoryItemPayload.ItemName.Equals(WeaponName, ESearchCase::IgnoreCase))
        {
            return Weapon;
        }
    }
    return nullptr;
}

void AZombiePlayer::UpdateFOV(float Value)
{
    if (FPS_Camera)
    {
        FPS_Camera->SetFieldOfView(Value);
    }

    UpdateArmsFromFOV(Value);
}

void AZombiePlayer::ClearCurrentWeapon()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->Drop();
    }
    CurrentWeapon = nullptr;
    CurrentAnimState = EPlayerAnimState::NoWeapon_AnimState;
}

void AZombiePlayer::EquipWeaponDirect(AWeaponBase* Weapon)
{
    // 无效或已经是同一把武器则跳过
    if (!Weapon || Weapon == CurrentWeapon) return;

    // 先安全卸载当前武器（会恢复旧武器的世界位置，但不会生成掉落物，因为Drop会按正常流程走）
    if (CurrentWeapon)
    {
        CurrentWeapon->Drop();
        CurrentWeapon = nullptr;
    }

    // 调用武器自身的装备逻辑（Attach到指定Socket，设置bIsEquipped等）
    Weapon->Equip(this);

    // 确认装备成功，更新玩家状态（但不要播放拾取蒙太奇）
    if (Weapon->IsEquipped())
    {
        CurrentWeapon = Weapon;
        CurrentAnimState = EPlayerAnimState::CG_Handgun_AnimState;
        // 注意：这里故意省略 PlayPickUpMontage()，避免加载存档时播放动画
    }
}



void AZombiePlayer::UpdateArmsFromFOV(float FOVValue)
{
    if (!FPSSkeletalMesh || !CurrentWeapon || !CurrentWeapon->ScopeMesh || !CurrentWeapon->bScopeEquipped) return;

    // 计算 FOV 变化进度（0.0 = 腰射，1.0 = 完全ADS）
    float FOVRange = NormalFOV - ADSFOV;
    float Alpha = FMath::Clamp((NormalFOV - FOVValue) / FOVRange, 0.0f, 1.0f);

    // 只有在瞄准且瞄准镜可见时才应用偏移
    FVector TargetOffset = FVector::ZeroVector;
    if (bIsAiming && CurrentWeapon->ScopeMesh->IsVisible())
    {
        TargetOffset = CurrentWeapon->WeaponData->AttachmentConfig.ArmsLocCorrection;
    }

    // 根据 Alpha 混合手臂位置（Alpha 从 0 → 1，手臂从默认位置移动至偏移位置）
    FVector DesiredLocation = DefaultArmsRelativeLocation + TargetOffset * Alpha;
    FPSSkeletalMesh->SetRelativeLocation(DesiredLocation);
}

float AZombiePlayer::CalculateDamage(float Distance, FName BoneName) const
{
    if (!CurrentWeapon) return 0.0f;
    // 1. 基础伤害 (已包含附件乘数)
    float BaseDamage = CurrentWeapon->GetCurrentDamage();

    // 2. 距离衰减
    float FalloffMult = 1.0f;
    if (CurrentWeapon->WeaponData->DamageConfig.DamageFalloffCurve)
    {
        FalloffMult = CurrentWeapon->WeaponData->DamageConfig.DamageFalloffCurve->GetFloatValue(Distance);
    }
    float AfterFalloff = BaseDamage * FalloffMult;

    // 3. 最低伤害
    if (AfterFalloff < CurrentWeapon->WeaponData->DamageConfig.MinimumDamage)
        AfterFalloff = CurrentWeapon->WeaponData->DamageConfig.MinimumDamage;

    // 4. 部位系数
    float BoneMult = 1.0f;
    if (CurrentWeapon->WeaponData->DamageConfig.BoneDamageMultipliers.Contains(BoneName))
    {
        BoneMult = CurrentWeapon->WeaponData->DamageConfig.BoneDamageMultipliers[BoneName];
    }

    return AfterFalloff * BoneMult;

}


void AZombiePlayer::RequestToggleWeapon(const FString& WeaponName, bool IsEquipped)
{
    if (!FindWeaponRefByName(WeaponName)) return;

    PendingToggleWeaponName = WeaponName;
    bPendingIsEquipped = IsEquipped;

    if (CurrentWeapon)  CurrentWeapon->StopReload();

    // 播放蒙太奇
    if (EquipMontage && FPSSkeletalMesh && FPSSkeletalMesh->GetAnimInstance() && !bPendingIsEquipped && CurrentWeapon)
    {
        UAnimInstance* AnimInstance = FPSSkeletalMesh->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->StopAllMontages(0.0);
            AnimInstance->Montage_Play(EquipMontage);
            PerformToggleWeapon(WeaponName);
        }
    }
    else
    {
        // 如果没有配置蒙太奇，直接切换
        PerformToggleWeapon(WeaponName);
    }

}


void AZombiePlayer::PerformToggleWeapon(const FString& WeaponName)
{
    AWeaponBase* EquippedWeapon = FindWeaponRefByName(WeaponName);
    if (!EquippedWeapon) return;

    // 卸下当前武器
    if (CurrentWeapon)
    {
   
        CurrentWeapon->UnEquip(this);
    }

    if (bPendingIsEquipped)
    {
        CurrentWeapon = nullptr;
        CurrentAnimState = EPlayerAnimState::NoWeapon_AnimState;
        return;
    }

    // 装备新武器
    EquippedWeapon->CanDirEquipped = true;
    EquippedWeapon->Equip(this);
    CurrentAnimState = EPlayerAnimState::CG_Handgun_AnimState;
    CurrentWeapon = EquippedWeapon;
}

void AZombiePlayer::ConsumeAmmoFromInventory()
{
    if (!CurrentWeapon || !InventoryComponent) return;

    const int32 CurrentAmmo = CurrentWeapon->InventoryItemPayload.AmmoAmount;
    int32 NeededAmmo = CurrentWeapon->WeaponData->AmmoAndUIConfig.MaxAmmo - CurrentAmmo;

    if (NeededAmmo <= 0) return;

    const int32 TotalAvailableAmmo = GetWeaponAmmoFromInventory();
    NeededAmmo = FMath::Min(NeededAmmo, TotalAvailableAmmo);

    const FString* AmmoNamePtr = FindWeaponAmmo.Find(CurrentWeapon->InventoryItemPayload.ItemName);
    if (!AmmoNamePtr) return;

    // 从库存扣弹药
    InventoryComponent->RemoveItemAmountFromInventory(NeededAmmo, *AmmoNamePtr);

    // 填满武器弹药
    CurrentWeapon->InventoryItemPayload.AmmoAmount = CurrentAmmo + NeededAmmo;
}

int32 AZombiePlayer::GetWeaponAmmoFromInventory()
{
    if (!InventoryComponent) return 0;
    if (!CurrentWeapon) return 0;
    const FString* AmmoNamePtr = FindWeaponAmmo.Find(CurrentWeapon->InventoryItemPayload.ItemName);
    if(!AmmoNamePtr)  return 0;
    FString AmmoName = *AmmoNamePtr;
    return InventoryComponent->FindAllItemAmountByName(AmmoName);
}

void AZombiePlayer::UpdateAllWeaponAmmoWidget()
{
    for(AWeaponBase* Weapon : AllOwnerWeapons)
    {
        if (!Weapon || !InventoryComponent) continue;
        InventoryComponent->UpdateAllWeaponWidgetAmmoByName(Weapon->InventoryItemPayload.ItemName, Weapon->InventoryItemPayload.AmmoAmount);
    }
}

void AZombiePlayer::PlayPickUpMontage()
{
    // 播放第一人称拾取蒙太奇
    if (PickupWeaponMontage && FPSSkeletalMesh)
    {
        // 通过 GetAnimInstance() 函数获取动画实例
        UAnimInstance* FPSAnimInstance = FPSSkeletalMesh->GetAnimInstance();
        if (FPSAnimInstance)
        {
            // 动画实例调用 Montage_Play 播放蒙太奇
            FPSAnimInstance->Montage_Play(PickupWeaponMontage);
        }
    }
}

void AZombiePlayer::Interact()
{
    UPlayerInteractionComponent* InteractionComp = FindComponentByClass<UPlayerInteractionComponent>();
    if (!InteractionComp) return;

    AActor* Target = InteractionComp->GetCurrentBestTarget();
    if (!Target) return;

    if (AWeaponBase* NewWeapon = Cast<AWeaponBase>(Target))
    {
        if (NewWeapon->Implements<UInteractable>())
        {
            PlayPickUpMontage();
            if (CurrentWeapon)
            {
                NewWeapon->CanDirEquipped = false;
                if (CurrentWeapon->bIsReloading)
                {
                    CurrentWeapon->StopReload();
                    StopAiming();
                }
            }
            AllOwnerWeapons.Add(NewWeapon);
            IInteractable::Execute_OnInteract(NewWeapon, this);
        }
        if (CurrentWeapon)  return;
        CurrentWeapon = NewWeapon; 

        CurrentAnimState = EPlayerAnimState::CG_Handgun_AnimState;
    }
    else
    {
        // 非武器物品
        if (Target->Implements<UInteractable>())
        {
            if (CurrentWeapon)
            {
                if (CurrentWeapon->bIsReloading)
                {
                    CurrentWeapon->StopReload();
                    StopAiming();
                }
            }

            PlayPickUpMontage();
            IInteractable::Execute_OnInteract(Target, this);
        }
    }
    InteractionComp->ClearBestTarget();
}

void AZombiePlayer::DropWeapon()
{
    if (!CurrentWeapon) return;

    const FString WeaponItemName = CurrentWeapon->InventoryItemPayload.ItemName;
    RemoveWeaponRefByName(WeaponItemName);
    CurrentWeapon->Drop();
    CurrentWeapon = nullptr;
    CurrentAnimState = EPlayerAnimState::NoWeapon_AnimState;

    if (InventoryComponent)
    {
        InventoryComponent->RemoveItemAmountFromInventory(1, WeaponItemName);
    }

    StopAiming();
}

void AZombiePlayer::PlayADSSound(bool bStart)
{
    USoundBase* SoundToPlay = bStart ? OnADSSound : OffADSSound;
    if (!SoundToPlay) return;

    if (ADSAudioComponent)
    {
        if (ADSAudioComponent->IsPlaying())
            ADSAudioComponent->Stop();
        ADSAudioComponent->SetSound(SoundToPlay);
        ADSAudioComponent->Play();
    }
}

void AZombiePlayer::SetAimingState(bool bAiming)
{
    // 如果状态未变化则不处理
    if (bIsAiming == bAiming) return;
    bIsAiming = bAiming;

    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (bAiming)
    {
        if (MovementComp)
        {
            MovementComp->MaxWalkSpeed = ADSWalkSpeed;
        }
        if (FOVTimeline)
        {
            FOVTimeline->Play();
        }
        PlayADSSound(true);
    }
    else
    {
        if (MovementComp)
        {
            MovementComp->MaxWalkSpeed = NormalWalkSpeed;
        }
        if (FOVTimeline)
        {
            FOVTimeline->Reverse();
        }
        PlayADSSound(false);
    }
}

void AZombiePlayer::StartAiming()
{
    // 条件检查：必须满足 CanAim
    if (!CanAim(nullptr)) return;

    // 避免重复进入
    if (bIsAiming) return;

    SetAimingState(true);
}

void AZombiePlayer::StopAiming()
{
    if (!bIsAiming) return;

    SetAimingState(false);
}

void AZombiePlayer::ToggleFlashlight()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->ToggleFlashlight(bIsAiming);
    }
}

void AZombiePlayer::Reload()
{
    if (!CurrentWeapon) return;
    CurrentWeapon->StartReload();
}

float AZombiePlayer::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage > 0.0f)
    {
        CurrentHealth -= ActualDamage;

        if (GEngine && bDebugShowHealth)
        {
            FString Msg = FString::Printf(TEXT("Player Health: %.1f / %.1f"), CurrentHealth, MaxHealth);
            GEngine->AddOnScreenDebugMessage(1, 3.0f, FColor::Cyan, Msg);
        }

        if (CurrentHealth <= 0.0f)
        {
            CurrentHealth = 0.0f;
            StopCurrentCameraShake();
            OnPlayerDeath.Broadcast();
        }
    }

    return ActualDamage;
}

void AZombiePlayer::UpdateCameraShake()
{
    if (!GetCharacterMovement()) return;

    FVector Velocity = GetVelocity();
    Velocity.Z = 0.0f;
    float Speed = Velocity.Size();
    bool bOnGround = GetCharacterMovement()->IsMovingOnGround();

    if (!bOnGround || Speed < ShakeMinSpeed)
    {
        if (CurrentShakeState != 0)
        {
            StopCurrentCameraShake();
            CurrentShakeState = 0;
        }
        return;
    }

    int32 TargetState = bIsSprinting ? 2 : 1;

    if (TargetState != CurrentShakeState)
    {
        StopCurrentCameraShake();

        TSubclassOf<UCameraShakeBase> ShakeClass = (TargetState == 2) ? RunCameraShakeClass : WalkCameraShakeClass;
        float Scale = (TargetState == 2) ? RunShakeScale : WalkShakeScale;

        if (ShakeClass)
        {
            // 使用 CameraManager 启动持续晃动，并保存实例指针
            if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
            {
                ActiveCameraShakeInstance = CameraManager->StartCameraShake(ShakeClass, Scale, ECameraShakePlaySpace::CameraLocal, FRotator::ZeroRotator);
                // 打印调试信息（可选）
                if (GEngine && ShakeClass)
                {
                    FString ShakeName = ShakeClass->GetName();   // 直接获得类名字符串
                    GEngine->AddOnScreenDebugMessage(
                        1,                    // Key，相同键会刷新覆盖
                        3.0f,                 // 显示时间（秒）
                        FColor::Cyan,
                        FString::Printf(TEXT("Playing Shake: %s"), *ShakeName)
                    );
                }
            }
        }

        CurrentShakeState = TargetState;
    }
}

void AZombiePlayer::PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale)
{
    if (!ShakeClass || Scale <= 0.0f) return;
    if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
    {
        CameraManager->StartCameraShake(ShakeClass, Scale, ECameraShakePlaySpace::CameraLocal, FRotator::ZeroRotator);
    }
}

void AZombiePlayer::StopCurrentCameraShake()
{
    if (APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
    {
        if (ActiveCameraShakeInstance.IsValid())
        {
            CameraManager->StopCameraShake(ActiveCameraShakeInstance.Get(), true);
            ActiveCameraShakeInstance.Reset();
        }
    }
    CurrentShakeState = 0;   // 重置状态，确保下次移动时重新创建
}

void AZombiePlayer::OpenInventory()
{
    if (InventoryComponent)
    {
        UpdateAllWeaponAmmoWidget();
        InventoryComponent->OpenInventory();
    }
}


