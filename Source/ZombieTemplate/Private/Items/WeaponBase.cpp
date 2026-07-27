// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/WeaponBase.h"
#include "GameFramework/Character.h"
#include <Kismet/GameplayStatics.h>
#include "Particles/ParticleSystemComponent.h"
#include <Characters/ZombiePlayer.h>



// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 创建武器骨骼网格体，附着到根组件（Root）
	Weapon_SKMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon_SKMesh"));
	Weapon_SKMesh->SetupAttachment(RootComponent);
	Weapon_SKMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Weapon_SKMesh->SetVisibility(false);
    Weapon_SKMesh->SetCastShadow(false);

    CompensatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CompensatorMesh"));
    CompensatorMesh->SetupAttachment(Weapon_SKMesh, MuzzleSocketName); // 或自定义 Socket
    CompensatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CompensatorMesh->SetVisibility(false);
    CompensatorMesh->SetCastShadow(false);

    // 瞄准镜槽盖组件
    ScopeSlotCoverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScopeSlotCoverMesh"));
    ScopeSlotCoverMesh->SetupAttachment(Weapon_SKMesh, ScopeSlotCoverSocketName);
    ScopeSlotCoverMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ScopeSlotCoverMesh->SetVisibility(true);   // 默认显示（无瞄准镜状态）
    ScopeSlotCoverMesh->SetCastShadow(false);

    // 消音器组件
    SilencerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SilencerMesh"));
    SilencerMesh->SetupAttachment(Weapon_SKMesh, MuzzleSocketName);
    SilencerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SilencerMesh->SetVisibility(false);   // 默认隐藏
    SilencerMesh->SetCastShadow(false);

    // 瞄准镜组件
    ScopeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScopeMesh"));
    ScopeMesh->SetupAttachment(Weapon_SKMesh, ScopeSocketName);
    ScopeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ScopeMesh->SetVisibility(false);   // 默认隐藏
    ScopeMesh->SetCastShadow(false);

    CaseEjectPS = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("CaseEjectPS"));
    CaseEjectPS->SetupAttachment(Weapon_SKMesh, NAME_None);  // 或使用专用 Socket
    CaseEjectPS->SetRelativeLocation(FVector(2.655801f, 0.000002f, 7.487558f));
    CaseEjectPS->bAutoActivate = false;
    CaseEjectPS->SetVisibility(true);   // 但不会自动播放
    CaseEjectPS->SetCastShadow(false);

    // ---------- 手电筒外壳 ----------
    FlashlightMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlashlightMesh"));
    FlashlightMesh->SetupAttachment(Weapon_SKMesh, FlashlightSocketName);
    FlashlightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FlashlightMesh->SetVisibility(false);
    // 忽略所有碰撞通道（已在自定义碰撞预设中隐含，也可显式设置）
    FlashlightMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    FlashlightMesh->SetCastShadow(false);

    // ---------- 激光指示器模型（附着在手电筒外壳上） ----------
    LaserBodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserBodyMesh"));
    LaserBodyMesh->SetupAttachment(FlashlightMesh);
    LaserBodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LaserBodyMesh->SetVisibility(false);
    LaserBodyMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    // 设置相对缩放 (X=2, Y=0.2, Z=0.2)
    LaserBodyMesh->SetRelativeScale3D(FVector(2.0f, 0.2f, 0.2f));
    LaserBodyMesh->SetCastShadow(false);

    // ---------- 激光点光源（附着在手电筒外壳上） ----------
    LaserDot = CreateDefaultSubobject<UPointLightComponent>(TEXT("LaserDot"));
    LaserDot->SetupAttachment(FlashlightMesh);
    LaserDot->SetVisibility(false);
    LaserDot->SetIntensity(15000.0f);
    LaserDot->SetLightColor(FColor::Red, true);
    LaserDot->SetAttenuationRadius(2.0f);
    LaserDot->SetCastShadows(false);   // 可选，小点光源没必要阴影


    // ---------- 聚光灯（手电筒光柱，附着在手电筒外壳上） ----------
    FlashlightBeam = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightBeam"));
    FlashlightBeam->SetupAttachment(FlashlightMesh);
    FlashlightBeam->SetVisibility(false);
    FlashlightBeam->SetIntensity(50000.0f);
    FlashlightBeam->SetAttenuationRadius(50000.0f);
    FlashlightBeam->SetOuterConeAngle(25.0f);

    ClearMuzzleSmokePool();

}

void AWeaponBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bLightAttachmentEquipped)   // 激光始终更新
    {
        UpdateLaserTarget();
    }
}

FName AWeaponBase::GetUniqueSaveID_Implementation() const
{
    return GetFName();
}

FActorSaveData AWeaponBase::GetSaveData_Implementation() const
{
    FActorSaveData Data;
    Data.ActorID = GetFName();
    Data.ActorClassPath = GetClass()->GetPathName();
    Data.CurrentAmmo = CurrentAmmo;
    // 配件状态
    Data.bCompensatorEquipped = bCompensatorEquipped;
    Data.bSilencerEquipped = bSilencerEquipped;
    Data.bScopeEquipped = bScopeEquipped;
    Data.bLightAttachmentEquipped = bLightAttachmentEquipped;
    Data.bFlashlightBeamOn = bFlashlightBeamOn;
    if (bIsEquipped && GetAttachParentActor())
    {
        Data.bEquipped = true;
        Data.EquipSocket = AttachSocketName;
        Data.RelativeTransform = GetActorTransform().GetRelativeTransform(
            GetAttachParentActor()->GetActorTransform()
        );
    }
    else
    {
        Data.bEquipped = false;
        Data.WorldTransform = GetActorTransform();
    }
    return Data;


}

void AWeaponBase::RestoreState_Implementation(const FActorSaveData& Data)
{
    // 弹药
    CurrentAmmo = Data.CurrentAmmo;

    bCompensatorEquipped = Data.bCompensatorEquipped;
    bSilencerEquipped = Data.bSilencerEquipped;
    bScopeEquipped = Data.bScopeEquipped;
    bLightAttachmentEquipped = Data.bLightAttachmentEquipped;
    bFlashlightBeamOn = Data.bFlashlightBeamOn;

    bCanDismember = Data.bCompensatorEquipped;
 

    if (!Data.bEquipped)  SetActorTransform(Data.WorldTransform);

    RefreshAttachmentVisuals();
}

void AWeaponBase::ResetToDefault_Implementation()
{
    if (GetAttachParentActor()) DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    SetActorTransform(OriginalWorldTransform);
    bIsEquipped = false;
    SetOwner(nullptr);          
    CachedHandsMesh = nullptr;  

    if (Weapon_SKMesh) Weapon_SKMesh->SetVisibility(false);
    if (VisualMesh) VisualMesh->SetVisibility(true);
    // 恢复掉落物理和视觉（VisualMesh）
    EnableWeaponPhysics();

    bCompensatorEquipped = false;
    bSilencerEquipped = false;
    bScopeEquipped = false;
    bLightAttachmentEquipped = false;
    bFlashlightBeamOn = false;

    // 关闭光源（防止漂在空中）
    if (LaserDot) LaserDot->SetVisibility(false);         // ← 加上
    if (FlashlightBeam) FlashlightBeam->SetVisibility(false); // ← 加上
    // 清除定时器
    if (GetWorldTimerManager().IsTimerActive(SmokeDelayTimer))
        GetWorldTimerManager().ClearTimer(SmokeDelayTimer);   // ← 加上  Drop函数参考


    bCanDismember = false;
    DismemberPower = 1;

    if (WeaponData) CurrentAmmo = WeaponData->AmmoAndUIConfig.MaxAmmo;

    RefreshAttachmentVisuals();

}

void AWeaponBase::RefreshAttachmentVisuals()
{
    if (CompensatorMesh)
    {
        CompensatorMesh->SetVisibility(bCompensatorEquipped);
        DismemberPower = CompensatorDismemberPower;
        
    }
    if (SilencerMesh)
        SilencerMesh->SetVisibility(bSilencerEquipped);
    if (ScopeMesh)
        ScopeMesh->SetVisibility(bScopeEquipped);
    if (ScopeSlotCoverMesh)
        ScopeSlotCoverMesh->SetVisibility(!bScopeEquipped); // 无镜时盖子显示

    if (FlashlightMesh)
        FlashlightMesh->SetVisibility(bLightAttachmentEquipped);
    if (LaserBodyMesh)
        LaserBodyMesh->SetVisibility(bLightAttachmentEquipped);
    if (LaserDot)
        LaserDot->SetVisibility(bLightAttachmentEquipped);
    if (FlashlightBeam)
        FlashlightBeam->SetVisibility(bLightAttachmentEquipped && bFlashlightBeamOn);
}

void AWeaponBase::SoftReset()
{ 
    // 重置配件标志（不生成掉落物，只改标志）
    bCompensatorEquipped = false;
    bSilencerEquipped = false;
    bScopeEquipped = false;
    bLightAttachmentEquipped = false;
    bFlashlightBeamOn = false;

    bCanDismember = false;
    DismemberPower = 1;

    if (WeaponData)
        CurrentAmmo = WeaponData->AmmoAndUIConfig.MaxAmmo;

    RefreshAttachmentVisuals();
}

void AWeaponBase::GetWeaponIconAndScale(UTexture2D*& OutIcon, float& OutScale) const
{
    // ========== 计算图标纹理（位掩码 + 回退） ==========
    auto TryGetIcon = [this](int32 Flags) -> UTexture2D*
        {
            if (WeaponData->AmmoAndUIConfig.WeaponIconTextures.IsValidIndex(Flags))
            {
                return WeaponData->AmmoAndUIConfig.WeaponIconTextures[Flags];
            }
            return nullptr;
        };

    int32 Flags = 0;
    if (bLightAttachmentEquipped)   Flags |= 1;   // 手电筒/激光
    if (bSilencerEquipped)          Flags |= 2;   // 消音器
    if (bCompensatorEquipped)       Flags |= 4;   // 补偿器
    if (bScopeEquipped)             Flags |= 8;   // 瞄准镜

    // 尝试完整组合
    OutIcon = TryGetIcon(Flags);

    // 回退：消音器与补偿器互斥，若都装备则优先保留消音器，去掉补偿器
    if (!OutIcon && (Flags & 4))
    {
        OutIcon = TryGetIcon(Flags & ~4);
    }
    // 如果还不行，则保留补偿器，去掉消音器
    if (!OutIcon && (Flags & 2))
    {
        OutIcon = TryGetIcon(Flags & ~2);
    }
    // 最终兜底：无任何附件
    if (!OutIcon)
    {
        OutIcon = TryGetIcon(0);
    }

    // ========== 计算缩放 ==========
    if (bCompensatorEquipped)
    {
        OutScale = WeaponData->AmmoAndUIConfig.CompensatorIconScale;
    }
    else if (bSilencerEquipped)
    {
        OutScale = WeaponData->AmmoAndUIConfig.SilencerIconScale;
    }
    else
    {
        OutScale = 1.0f;
    }

}

void AWeaponBase::OnConstruction(const FTransform& Transform)
{

    Super::OnConstruction(Transform);

    
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
    InitializeProjectilePool(); // 初始化对象池
    SaveBaseAttributes();
    ClearMuzzleSmokePool();

    // 动态创建烟雾粒子池（不能在 BeginPlay 中用 CreateDefaultSubobject）
    if (WeaponData && Weapon_SKMesh && MuzzleSmokePool.Num() == 0) // 防止重复创建
    {
        for (int32 i = 0; i < WeaponData->VisualFX.MuzzleSmokePoolSize; ++i)
        {
            FString CompName = FString::Printf(TEXT("MuzzleSmoke_%d"), i);
            UParticleSystemComponent* SmokeComp = NewObject<UParticleSystemComponent>(this, *CompName);
            SmokeComp->SetupAttachment(Weapon_SKMesh, MuzzleSocketName);
            SmokeComp->bAutoActivate = false;
            SmokeComp->SetRelativeLocation(FVector::ZeroVector);
            SmokeComp->SetRelativeScale3D(WeaponData->VisualFX.MuzzleSmokeScale);
            SmokeComp->RegisterComponent();          // 必须注册
            SmokeComp->SetVisibility(true);
            MuzzleSmokePool.Add(SmokeComp);
        }
    }

    OriginalDismemberPower = DismemberPower;

    OriginalWorldTransform = GetActorTransform();

}

void AWeaponBase::OnInteract_Implementation(AActor* Interactor)
{
    Super::OnInteract_Implementation(Interactor);
	Equip(Interactor);

}

void AWeaponBase::Equip(AActor* NewOwner)
{
    if (!NewOwner || bIsEquipped) return;

    DisableWeaponPhysics();

    if (Weapon_SKMesh)
    {
        Weapon_SKMesh->SetVisibility(true);
    }

    // 确定要附着的骨骼网格体
    USkeletalMeshComponent* AttachTargetMesh = nullptr;

    if (ACharacter* Character = Cast<ACharacter>(NewOwner))
    {
        // 优先查找带有 "FPSMesh" 标签的骨骼网格体（第一人称手臂）
        TArray<USkeletalMeshComponent*> SkelComps;
        Character->GetComponents<USkeletalMeshComponent>(SkelComps);
        for (USkeletalMeshComponent* Comp : SkelComps)
        {
            if (Comp->ComponentHasTag(FName("FPSMesh")))
            {
                AttachTargetMesh = Comp;
                CachedHandsMesh = AttachTargetMesh;
                break;
            }
        }

        // 如果没找到，回退到默认的第三人称 Mesh
        if (!AttachTargetMesh)
        {
            AttachTargetMesh = Character->GetMesh();
        }
    }

    if (AttachTargetMesh)
    {
        AttachToComponent(AttachTargetMesh,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            AttachSocketName);
    }
    else
    {
        // 找不到合适网格体，恢复掉落状态（丢弃）
        EnableWeaponPhysics();
        if (Weapon_SKMesh) Weapon_SKMesh->SetVisibility(false);
        return; // 直接返回，不标记为已装备
    }

    SetOwner(NewOwner);
    bIsEquipped = true;
    OnInteract.Broadcast(true);
    if (LaserDot)
    {
        GetWorld()->GetTimerManager().SetTimer(LaserDotVisibilityTimer,
            [this]()
            {
                // 仅在装备状态仍有效且附件仍装备时显示
                if (bIsEquipped && bLightAttachmentEquipped && LaserDot)
                {
                    LaserDot->SetVisibility(true);
                }
            },
            0.8f,
            false
        );
    }

}

void AWeaponBase::EquipCompensator()
{
    if (!bIsEquipped) return;
    
    if (bSilencerEquipped) RemoveSilencer(); // 互斥

    if (CompensatorMesh) CompensatorMesh->SetVisibility(true);
    if (WeaponData->AttachmentConfig.CompensatorFireSound) WeaponData->VisualFX.FireSound = WeaponData->AttachmentConfig.CompensatorFireSound;
    bCompensatorEquipped = true;
    bCanDismember = true;   // ← 补偿器开启断肢

    OriginalDismemberPower = DismemberPower;
    DismemberPower = CompensatorDismemberPower;
}

void AWeaponBase::RemoveCompensator()
{
    if (!bCompensatorEquipped) return;

    // 生成前清除残留的同 ID 掉落物
    FName DropID = FName(*(GetName() + TEXT("_CompensatorDrop")));
    TArray<AActor*> Existing;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveableActor::StaticClass(), Existing);
    for (AActor* Act : Existing)
    {
        if (Act && Act->Implements<USaveableActor>() &&
            ISaveableActor::Execute_GetUniqueSaveID(Act) == DropID)
        {
            Act->Destroy();
        }
    }

    // 生成新掉落物
    if (WeaponData->AttachmentConfig.CompensatorDropClass && Weapon_SKMesh)
    {
        FVector SpawnLoc = Weapon_SKMesh->GetSocketLocation(MuzzleSocketName);
        FRotator SpawnRot = GetActorRotation();
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* DropActor = GetWorld()->SpawnActor<AActor>(WeaponData->AttachmentConfig.CompensatorDropClass, SpawnLoc, SpawnRot, SpawnParams);
        if (AInspectableItem* Inspectable = Cast<AInspectableItem>(DropActor))
        {
            Inspectable->SaveActorID = DropID;
        }
    }

    if (CompensatorMesh) CompensatorMesh->SetVisibility(false);
    RestoreBaseAttributes();
    bCompensatorEquipped = false;
    bCanDismember = false;
    DismemberPower = OriginalDismemberPower;
}

void AWeaponBase::EquipSilencer()
{
    
    if (!bIsEquipped) return;
    if (bCompensatorEquipped) RemoveCompensator();

    if (SilencerMesh) SilencerMesh->SetVisibility(true);
    if (WeaponData->AttachmentConfig.SilencerFireSound) WeaponData->VisualFX.FireSound = WeaponData->AttachmentConfig.SilencerFireSound;
    bSilencerEquipped = true;
}

void AWeaponBase::RemoveSilencer()
{
    if (!bSilencerEquipped) return;

    // 生成前清除残留的同 ID 掉落物
    FName DropID = FName(*(GetName() + TEXT("_SilencerDrop")));
    TArray<AActor*> Existing;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveableActor::StaticClass(), Existing);
    for (AActor* Act : Existing)
    {
        if (Act && Act->Implements<USaveableActor>() &&
            ISaveableActor::Execute_GetUniqueSaveID(Act) == DropID)
        {
            Act->Destroy();
        }
    }

    if (WeaponData->AttachmentConfig.SilencerDropClass && Weapon_SKMesh)
    {
        FVector SpawnLoc = Weapon_SKMesh->GetSocketLocation(MuzzleSocketName);
        FRotator SpawnRot = GetActorRotation();
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* DropActor = GetWorld()->SpawnActor<AActor>(WeaponData->AttachmentConfig.SilencerDropClass, SpawnLoc, SpawnRot, SpawnParams);

        if (AInspectableItem* Inspectable = Cast<AInspectableItem>(DropActor))
        {
            Inspectable->SaveActorID = DropID;
        }
    }

    if (SilencerMesh) SilencerMesh->SetVisibility(false);
    RestoreBaseAttributes();
    bSilencerEquipped = false;
}

void AWeaponBase::EquipScope()
{
    
    if (!bIsEquipped || !ScopeMesh) return;
    ScopeMesh->SetVisibility(true);
    if (ScopeSlotCoverMesh) ScopeSlotCoverMesh->SetVisibility(false);
    bScopeEquipped = true;
}

void AWeaponBase::RemoveScope()
{
    if (!bScopeEquipped) return;

    // 生成前清除残留的同 ID 掉落物
    FName DropID = FName(*(GetName() + TEXT("_ScopeDrop")));
    TArray<AActor*> Existing;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveableActor::StaticClass(), Existing);
    for (AActor* Act : Existing)
    {
        if (Act && Act->Implements<USaveableActor>() &&
            ISaveableActor::Execute_GetUniqueSaveID(Act) == DropID)
        {
            Act->Destroy();
        }
    }

    if (WeaponData->AttachmentConfig.ScopeDropClass && Weapon_SKMesh)
    {
        FVector SpawnLoc = Weapon_SKMesh->GetSocketLocation(ScopeSocketName);
        FRotator SpawnRot = GetActorRotation();
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* DropActor = GetWorld()->SpawnActor<AActor>(
            WeaponData->AttachmentConfig.ScopeDropClass,
            SpawnLoc, SpawnRot, SpawnParams
        );

        if (AInspectableItem* Inspectable = Cast<AInspectableItem>(DropActor))
        {
            Inspectable->SaveActorID = DropID;
        }
    }

    if (ScopeMesh) ScopeMesh->SetVisibility(false);
    if (ScopeSlotCoverMesh) ScopeSlotCoverMesh->SetVisibility(true);
    bScopeEquipped = false;
}


void AWeaponBase::StartReload(int32 AmmoToAdd)
{
    // 检查状态
    if (!bIsEquipped || bIsReloading || AmmoToAdd <= 0 || CurrentAmmo >= WeaponData->AmmoAndUIConfig.MaxAmmo) return;

    bIsReloading = true;
    PendingReloadAmount = AmmoToAdd;   // 暂存数量，动画结束时使用

    // 播放手部换弹蒙太奇
    if (WeaponData->AnimMontageSet.ReloadMontage_Hands && CachedHandsMesh)
    {
        UAnimInstance* HandsAnimInst = CachedHandsMesh->GetAnimInstance();
        if (HandsAnimInst)
        {
            HandsAnimInst->Montage_Play(WeaponData->AnimMontageSet.ReloadMontage_Hands);
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &AWeaponBase::OnReloadMontageEnded);
            HandsAnimInst->Montage_SetEndDelegate(EndDelegate, WeaponData->AnimMontageSet.ReloadMontage_Hands);
        }
    }

    // 播放武器自身换弹蒙太奇
    if (WeaponData->AnimMontageSet.ReloadMontage_Weapon && Weapon_SKMesh)
    {
        UAnimInstance* WeaponAnimInst = Weapon_SKMesh->GetAnimInstance();
        if (WeaponAnimInst)
        {
            WeaponAnimInst->Montage_Play(WeaponData->AnimMontageSet.ReloadMontage_Weapon);
        }
    }
}

void AWeaponBase::StopReload()
{
    if (!bIsReloading) return;
    bIsReloading = false;
    // 停止手部蒙太奇
    if (CachedHandsMesh && WeaponData->AnimMontageSet.ReloadMontage_Hands)
    {
        UAnimInstance* HandsAnimInst = CachedHandsMesh->GetAnimInstance();
        if (HandsAnimInst && HandsAnimInst->Montage_IsPlaying(WeaponData->AnimMontageSet.ReloadMontage_Hands))
        {
            HandsAnimInst->Montage_Stop(0.1f, WeaponData->AnimMontageSet.ReloadMontage_Hands);
        }
    }
    // 停止武器蒙太奇
    if (Weapon_SKMesh && WeaponData->AnimMontageSet.ReloadMontage_Weapon)
    {
        UAnimInstance* WeaponAnimInst = Weapon_SKMesh->GetAnimInstance();
        if (WeaponAnimInst && WeaponAnimInst->Montage_IsPlaying(WeaponData->AnimMontageSet.ReloadMontage_Weapon))
        {
            WeaponAnimInst->Montage_Stop(0.1f, WeaponData->AnimMontageSet.ReloadMontage_Weapon);
        }
    }
}

void AWeaponBase::OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    bIsReloading = false;


    // 只有正常结束且有待补充弹药时才处理
    if (!bInterrupted && PendingReloadAmount > 0)
    {
        // 通知拥有者（玩家）消耗后备弹药，并获得实际可补充的数量
        AZombiePlayer* Player = Cast<AZombiePlayer>(GetOwner());
        int32 ActualAdded = 0;
        if (Player)
        {
            // 玩家返回实际消耗的后备弹药数（可能少于请求值，如果后备不足）
            ActualAdded = Player->ConsumeReserveAmmo(PendingReloadAmount);
        }
        else
        {
            // 如果拥有者不是玩家（例如AI），则直接使用待补充量（不消耗后备）
            ActualAdded = PendingReloadAmount;
        }

        // 填充武器弹药，确保不超过最大容量
        CurrentAmmo = FMath::Min(CurrentAmmo + ActualAdded, WeaponData->AmmoAndUIConfig.MaxAmmo);
        CheckAmmoStateChange();
    }

    PendingReloadAmount = 0;
}


void AWeaponBase::ActivateCaseEject()
{
    if (CaseEjectPS)
    {
        CaseEjectPS->Activate(true);
    }
}

void AWeaponBase::EquipLightAttachment()
{
    if (!bIsEquipped || bLightAttachmentEquipped) return;
   
    // 显示外壳和激光模型（激光一直亮）
    if (FlashlightMesh) FlashlightMesh->SetVisibility(true);
    if (LaserBodyMesh) LaserBodyMesh->SetVisibility(true);

    // 手电筒光束初始关闭
    if (FlashlightBeam) FlashlightBeam->SetVisibility(false);
    bFlashlightBeamOn = false;
    bLightAttachmentEquipped = true;

    if (LaserDot)
    {
        GetWorld()->GetTimerManager().SetTimer(LaserDotVisibilityTimer,
            [this]()
            {
                // 仅在装备状态仍有效且附件仍装备时显示
                if (bIsEquipped && bLightAttachmentEquipped && LaserDot)
                {
                    LaserDot->SetVisibility(true);
                }
            },
            0.8f,
            false
        );
    }
}

void AWeaponBase::RemoveLightAttachment()
{
    if (!bLightAttachmentEquipped) return;

    // 如果手电筒还开着，先关闭
    if (FlashlightBeam && bFlashlightBeamOn)
    {
        FlashlightBeam->SetVisibility(false);
        bFlashlightBeamOn = false;
    }

    // 隐藏所有模型
    if (FlashlightMesh) FlashlightMesh->SetVisibility(false);
    if (LaserBodyMesh) LaserBodyMesh->SetVisibility(false);
    if (LaserDot) LaserDot->SetVisibility(false);

    bLightAttachmentEquipped = false;
}

void AWeaponBase::ToggleFlashlight(bool bIsAiming)
{
    if (bIsReloading) return;
    // 必须已装备照明附件
    if (!bIsEquipped || !bLightAttachmentEquipped) return;

    bFlashlightBeamOn = !bFlashlightBeamOn;
    if (FlashlightBeam) FlashlightBeam->SetVisibility(bFlashlightBeamOn);

    // 播放蒙太奇和音效
    UAnimMontage* Montage = bIsAiming ? WeaponData->AnimMontageSet.SwitchLightMontage_ADS : WeaponData->AnimMontageSet.SwitchLightMontage_Hip;
    if (Montage && CachedHandsMesh)
    {
        UAnimInstance* AnimInst = CachedHandsMesh->GetAnimInstance();
        if (AnimInst) AnimInst->Montage_Play(Montage);
    }
    if (SwitchLightSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), SwitchLightSound, GetActorLocation());
    }
}

void AWeaponBase::UpdateLaserTarget()
{
    if (!bIsEquipped || !LaserDot) return;

    // 1. 获取持有者（玩家）的摄像机位置和方向
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return;

    AController* OwnerController = OwnerActor->GetInstigatorController();
    if (!OwnerController) return;

    FVector CameraLoc;
    FRotator CameraRot;
    OwnerController->GetPlayerViewPoint(CameraLoc, CameraRot);
    FVector AimDir = CameraRot.Vector();

    // 2. 从摄像机发射一条射线（精确无散布），得到准星瞄准点
    FVector TraceStart = CameraLoc;
    FVector TraceEnd = TraceStart + AimDir * WeaponData->VisualFX.FireRange;

    FHitResult Hit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(OwnerActor);

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
    FVector ImpactPoint = bHit ? Hit.ImpactPoint : TraceEnd;

    // 3. 计算激光点光源最终位置（沿法线偏移避免陷入墙面）
    FVector OffsetDirection = bHit ? Hit.ImpactNormal : (-AimDir);
    FVector FinalLaserPoint = ImpactPoint + OffsetDirection * LaserDotDepthOffset;

    // 4. 将激光点光源移动到该位置（每帧设置，临时脱离父级）
    if (LaserDot)
    {
        LaserDot->SetWorldLocation(FinalLaserPoint);
    }

    // 5. 绘制从激光发射口到目标点的连线（如果有 LaserBodyMesh）
    if (bDrawLaserDebug && LaserBodyMesh)
    {
        FVector LaserOrigin = LaserBodyMesh->GetComponentLocation();
        DrawDebugLine(GetWorld(), LaserOrigin, FinalLaserPoint, FColor::Red, false, -1.0f, 0, 1.0f);
        DrawDebugPoint(GetWorld(), FinalLaserPoint, 5.0f, FColor::Red, false, -1.0f);
    }

}

void AWeaponBase::GetAmmoInfo(int32& OutCurrent, int32& OutMax) const
{
    OutCurrent = CurrentAmmo;
    OutMax = WeaponData->AmmoAndUIConfig.MaxAmmo;
}

void AWeaponBase::ConsumeAmmo()
{
    if (CurrentAmmo > 0)
    {
        CurrentAmmo--;
        CheckAmmoStateChange();
    }
}

float AWeaponBase::GetCurrentDamage() const
{
    float FinalDamage = WeaponData->DamageConfig.Damage;
    if (bSilencerEquipped)   FinalDamage *= WeaponData->DamageConfig.SilencerDamageMultiplier;
    // 补偿器不直接影响伤害，只负责断肢，可在此扩展
    return FinalDamage;
}

void AWeaponBase::CheckAmmoStateChange()
{
    bool bIsEmpty = (CurrentAmmo <= 0);
    if (bIsEmpty != bLastIsEmpty)
    {
        OnAmmoEmptyStateChanged.Broadcast(bIsEmpty);
        bLastIsEmpty = bIsEmpty;
    }
}


void AWeaponBase::SaveBaseAttributes()
{
    OriginalFireSound = WeaponData->VisualFX.FireSound;
}

void AWeaponBase::RestoreBaseAttributes()
{
    WeaponData->VisualFX.FireSound = OriginalFireSound;
}

void AWeaponBase::Drop()
{
    if (!bIsEquipped) return;
    if (bIsReloading) StopReload();
    // 从拥有者分离
    DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

    // 恢复掉落物理和视觉（VisualMesh）
    EnableWeaponPhysics();

    // 隐藏武器专用的骨骼网格体
    if (Weapon_SKMesh)
    {
        Weapon_SKMesh->SetVisibility(false);
    }

    SetOwner(nullptr);
    bIsEquipped = false;
    CachedHandsMesh = nullptr;

    if (bLightAttachmentEquipped)
    {
        if (LaserDot) LaserDot->SetVisibility(false);
    }

    if (GetWorldTimerManager().IsTimerActive(SmokeDelayTimer))
    {
        GetWorldTimerManager().ClearTimer(SmokeDelayTimer);
    }
}


void AWeaponBase::ActivatePooledSmoke()
{
    if (!bIsEquipped || !WeaponData->VisualFX.MuzzleSmokeParticle) return;

    for (UParticleSystemComponent* SmokeComp : MuzzleSmokePool)
    {
        if (SmokeComp && !SmokeComp->IsActive())
        {
            // 更新偏移（根据附件动态变化）
            SmokeComp->SetRelativeLocation(GetCurrentMuzzleSmokeOffset());
            // 如果模板未设置，使用武器指定的粒子资产
            if (SmokeComp->Template == nullptr)
            {
                SmokeComp->SetTemplate(WeaponData->VisualFX.MuzzleSmokeParticle);
            }
            SmokeComp->Activate(true);
            SmokeComp->OnSystemFinished.RemoveDynamic(this, &AWeaponBase::OnPooledSmokeFinished);
            SmokeComp->OnSystemFinished.AddDynamic(this, &AWeaponBase::OnPooledSmokeFinished);



            return;
        }
    }
}

void AWeaponBase::OnPooledSmokeFinished(UParticleSystemComponent* PSC)
{
    if (PSC)
    {
        PSC->Deactivate();  
    }
}

FVector AWeaponBase::GetCurrentMuzzleFlashOffset() const
{
    if (bSilencerEquipped)      return WeaponData->VisualFX.MuzzleFlashOffset_Silencer;
    if (bCompensatorEquipped)   return WeaponData->VisualFX.MuzzleFlashOffset_Compensator;
    return WeaponData->VisualFX.MuzzleFlashOffset_Default;
}

FVector AWeaponBase::GetCurrentMuzzleSmokeOffset() const
{
    if (bSilencerEquipped)      return WeaponData->VisualFX.MuzzleSmokeOffset_Silencer;
    if (bScopeEquipped)         return WeaponData->VisualFX.MuzzleSmokeOffset_Scope;
    return WeaponData->VisualFX.MuzzleSmokeOffset_Default;
}

void AWeaponBase::FireWeaponVisuals(const FVector& MuzzleLocation, const FRotator& AimRotation, bool bIsAiming, USkeletalMeshComponent* HandsMesh)
{
    if (!Weapon_SKMesh) return;

    // 1. 手臂动画
    UAnimMontage* HandsMontage = bIsAiming ? WeaponData->AnimMontageSet.FireMontage_ADS_Hands : WeaponData->AnimMontageSet.FireMontage_Hip_Hands;
    if (HandsMontage && HandsMesh)
    {
        if (UAnimInstance* AnimInstance = HandsMesh->GetAnimInstance())
            AnimInstance->Montage_Play(HandsMontage);
    }

    // 2. 武器自身动画
    if (WeaponData->AnimMontageSet.FireMontage_Weapon)
    {
        if (UAnimInstance* AnimInstance = Weapon_SKMesh->GetAnimInstance())
            AnimInstance->Montage_Play(WeaponData->AnimMontageSet.FireMontage_Weapon);
    }

    // 3. 枪口火焰
    if (WeaponData->VisualFX.MuzzleFlashParticle)
    {
        FVector ActualFlashOffset = GetCurrentMuzzleFlashOffset();
        UGameplayStatics::SpawnEmitterAttached(
            WeaponData->VisualFX.MuzzleFlashParticle,
            Weapon_SKMesh,
            MuzzleSocketName,
            ActualFlashOffset,          // 使用动态偏移
            FRotator::ZeroRotator,
            WeaponData->VisualFX.MuzzleScale,
            EAttachLocation::SnapToTarget,
            true
        );
    }


// 4. 枪口烟雾（延迟触发 + 对象池）
    if (WeaponData->VisualFX.MuzzleSmokeParticle)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();

        // 冷却重置：超过 3 秒未射击，计数清零
        if (CurrentTime - LastShotTime > WeaponData->VisualFX.ShotCounterResetTime)
        {
            ConsecutiveShots = 0;
        }

        LastShotTime = CurrentTime;
        ConsecutiveShots++;

        if (ConsecutiveShots >= WeaponData->VisualFX.ShotsToStartSmoke)
        {
            ActivatePooledSmoke();  // 内部会根据计数控制烟雾规模
        }
    }
   

    // 5. 开火音效
    if (WeaponData->VisualFX.FireSound)
    {
        USceneComponent* AttachComp = Weapon_SKMesh ? Weapon_SKMesh : RootComponent;
        UGameplayStatics::SpawnSoundAttached(
            WeaponData->VisualFX.FireSound, AttachComp, NAME_None, FVector::ZeroVector,
            EAttachLocation::KeepRelativeOffset, false, 1.0f, 1.0f, 0.0f,
            WeaponData->VisualFX.FireSoundAttenuation, WeaponData->VisualFX.FireSoundConcurrency, false);
    }

    // 6弹壳抛出
    ActivateCaseEject();
}

void AWeaponBase::SpawnProjectileOrHit(const FVector& MuzzleLocation, const FVector& BulletDirection, bool bHit, const FHitResult& HitResult)
{
    if (bUseProjectile && BulletClass)
    {
        AProjectileBase* Proj = GetPooledProjectile();
        if (Proj)
        {
            Proj->ActivateFromPool(MuzzleLocation, BulletDirection, 9000.0f); 
            Proj->AddIgnoreActor(GetOwner());
            Proj->AddIgnoreActor(this);
        }

        ProcessHit(HitResult);

    }
    else  
    {
        if (bHit)
        {
            ProcessHit(HitResult);
        }
    }
}

void AWeaponBase::ProcessHit(const FHitResult& Hit)
{
    if (!Hit.bBlockingHit) return;

    // 击中音效
    if (WeaponData->VisualFX.ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponData->VisualFX.ImpactSound, Hit.ImpactPoint);
    }

    // 弹孔贴花
    if (WeaponData->VisualFX.BulletHoleDecalClass)
    {
        UPrimitiveComponent* HitComp = Hit.Component.Get();
        if (HitComp && (HitComp->IsA<UStaticMeshComponent>()))
        {
            FRotator DecalRotation = Hit.ImpactNormal.Rotation();
            GetWorld()->SpawnActor<AActor>(WeaponData->VisualFX.BulletHoleDecalClass, Hit.ImpactPoint, DecalRotation);
        }
    }

    // 物理冲量等效果可后续添加
}

void AWeaponBase::ClearMuzzleSmokePool()
{
    for (UParticleSystemComponent* SmokeComp : MuzzleSmokePool)
    {
        if (SmokeComp)
        {
            // 1. 停止所有粒子效果
            SmokeComp->Deactivate();
            SmokeComp->OnSystemFinished.RemoveAll(this);  // 移除所有委托绑定

            // 2. 从父组件分离
            SmokeComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);

            // 3. 从世界移除并销毁组件（推荐方式）
            SmokeComp->DestroyComponent();
        }
    }

    // 4. 清空数组
    MuzzleSmokePool.Empty();
}

AProjectileBase* AWeaponBase::GetPooledProjectile()
{
    for (AProjectileBase* Proj : ProjectilePool)
    {
        if (Proj && !Proj->IsActive())
            return Proj;
    }
  
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,               // Key: -1 表示每次都会刷新并覆盖
            2.0f,             // 显示时长（秒）
            FColor::Red,      // 颜色
            TEXT("Projectile pool exhausted! Consider increasing pool size.")
        );
    }

    return nullptr;
}

void AWeaponBase::InitializeProjectilePool()
{
    if (!BulletClass) return;
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 i = 0; i < ProjectilePoolSize; ++i)
    {
        AProjectileBase* Proj = GetWorld()->SpawnActor<AProjectileBase>(
            BulletClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (Proj)
        {
            Proj->DeactivateToPool();   // 确保初始停用
            ProjectilePool.Add(Proj);
        }
    }
}

float AWeaponBase::GetCurrentHipSpread() const
{
    float Spread = WeaponData->RecoilSpread.HipBaseSpread;
    if (bCompensatorEquipped) Spread *= WeaponData->RecoilSpread.CompensatorSpreadMultiplier;
    if (bSilencerEquipped) Spread *= WeaponData->RecoilSpread.SilencerSpreadMultiplier;
    if (!bScopeEquipped) // 激光只在腰射时生效（开镜时通常关闭）
        Spread *= WeaponData->RecoilSpread.LaserHipSpreadMultiplier;
    // 瞄准镜本身不直接影响腰射，可以忽略
    return Spread;
}

float AWeaponBase::GetCurrentADSSpread() const
{
    float Spread = WeaponData->RecoilSpread.ADSBaseSpread;
    if (bCompensatorEquipped) Spread *= WeaponData->RecoilSpread.CompensatorSpreadMultiplier;
    if (bSilencerEquipped) Spread *= WeaponData->RecoilSpread.SilencerSpreadMultiplier;
    if (bScopeEquipped) Spread *= WeaponData->RecoilSpread.ScopeADSSpreadMultiplier;
    if (bScopeEquipped) Spread *= WeaponData->RecoilSpread.LaserADSSpreadMultiplier; // 一般开镜时激光关闭，但保留乘数
    return Spread;
}

FVector2D AWeaponBase::GetCurrentRecoilYaw() const
{
    FVector2D Yaw = WeaponData->RecoilSpread.RecoilYaw;
    if (bCompensatorEquipped) Yaw *= WeaponData->RecoilSpread.CompensatorRecoilMultiplier;
    if (bSilencerEquipped) Yaw *= WeaponData->RecoilSpread.SilencerRecoilMultiplier;
    return Yaw;
}

FVector2D AWeaponBase::GetCurrentRecoilPitch() const
{
    FVector2D Pitch = WeaponData->RecoilSpread.RecoilPitch;
    if (bCompensatorEquipped) Pitch *= WeaponData->RecoilSpread.CompensatorRecoilMultiplier;
    if (bSilencerEquipped) Pitch *= WeaponData->RecoilSpread.SilencerRecoilMultiplier;
    return Pitch;
}

void AWeaponBase::DisableWeaponPhysics()
{
    if (VisualMesh)
    {
        // 必须先关物理，再附加
        VisualMesh->SetSimulatePhysics(false);
        VisualMesh->SetEnableGravity(false);
        VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        VisualMesh->SetVisibility(false);
    }

    if (InteractionWidget)
    {
        InteractionWidget->SetVisibility(false);
    }

}

void AWeaponBase::EnableWeaponPhysics()
{
	if (VisualMesh)
    {
        VisualMesh->SetVisibility(true);
        VisualMesh->SetSimulatePhysics(true);      // 开启物理模拟（可以和其他物体碰撞）
        VisualMesh->SetEnableGravity(true);     
        VisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

        // 恢复碰撞响应（与基类 AInspectableItem 的默认设置一致）
        VisualMesh->SetCollisionResponseToAllChannels(ECR_Block);
        VisualMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        VisualMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
        VisualMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }

    // 显示检视控件（掉落物需要拾取提示）
    if (InteractionWidget)
    {
        InteractionWidget->SetVisibility(true);
    }
}

void AWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    ClearMuzzleSmokePool();
}
