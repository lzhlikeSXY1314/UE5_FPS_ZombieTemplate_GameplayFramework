// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ZombieBase.h"
#include "../../../../Plugins/DismembermentSystem/Source/DismembermentSystem/Public/Components/GoreComponent.h"
#include "Engine/DamageEvents.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "SkinnedDecalSampler.h"
#include "Kismet/KismetMathLibrary.h" 
#include <Kismet/KismetArrayLibrary.h>
#include <Items/WeaponBase.h>
#include "AIController.h"
#include <Kismet/GameplayStatics.h>
#include "EngineUtils.h"

#include "Engine/OverlapResult.h"

#include <Characters/ZombiePlayer.h>


// Sets default values
AZombieBase::AZombieBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    DismemberGoreComponent = CreateDefaultSubobject<UGoreComponent>(TEXT("DismemberGoreComponent"));
  
}

void AZombieBase::InitializeBoneNames()
{
    // ---- 处理胸部骨骼 ----
    AllChestName.Empty();  // 清空

    // 依次追加
    AllChestName.Append(ZombieData->BoneNames.HitUpperChestNameArr);
    AllChestName.Append(ZombieData->BoneNames.HitMidChestNameArr);
    AllChestName.Append(ZombieData->BoneNames.HitLowerChestNameArr);

    // ---- 处理四肢骨骼 ----
    AllLimbsName.Empty();

    AllLimbsName.Append(ZombieData->BoneNames.HitFullRightHandNameArr);
    AllLimbsName.Append(ZombieData->BoneNames.HitFullLeftHandNameArr);
    AllLimbsName.Append(ZombieData->BoneNames.HitFullRightFootNameArr);
    AllLimbsName.Append(ZombieData->BoneNames.HitFullLeftFootNameArr);
}

// Called when the game starts or when spawned
void AZombieBase::BeginPlay()
{
	Super::BeginPlay();

    check(ZombieData);

    if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
    {
        AnimInst->OnMontageEnded.AddDynamic(this, &AZombieBase::OnAnyMontageEnded);
    }

    // 优先用标签查找，否则回退到默认 Mesh
    TargetDismemberMesh = FindMeshByTag("DismemberMesh");
    if (!TargetDismemberMesh) TargetDismemberMesh = GetMesh();

    // 查找 GoreComponent
    UGoreComponent* GoreComp = FindComponentByClass<UGoreComponent>();
    if (GoreComp && TargetDismemberMesh)
    {
        // 设置肢解目标
        GoreComp->SetSkeletalMeshComponentToDismember(TargetDismemberMesh);
    }
    InitializeBoneNames();

    SkinnedDecalSampler = FindComponentByClass<USkinnedDecalSampler>();
    SkinnedDecalSampler->SetMeshComponent(TargetDismemberMesh);

    LastSprintTime = -ZombieData->SprintAttackConfig.SprintCooldown;

    if (ZombieData)
    {
        HeadBreakBullets = ZombieData->DismemberConfig.HeadBreakBullets;
        RightArmBreakBullets = ZombieData->DismemberConfig.RightArmBreakBullets;
        LeftArmBreakBullets = ZombieData->DismemberConfig.LeftArmBreakBullets;
        RightLegBreakBullets = ZombieData->DismemberConfig.RightLegBreakBullets;
        LeftLegBreakBullets = ZombieData->DismemberConfig.LeftLegBreakBullets;
    }
}

// Called every frame
void AZombieBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AZombieBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

UAnimSequenceBase* AZombieBase::GetIdleAnimation_Implementation() const
{
    if (!ZombieData) return nullptr;
    return ZombieData->AnimationConfig.Animation_Idle;  // 你的 UPROPERTY
}

UAnimSequenceBase* AZombieBase::GetWalkAnimation_Implementation() const
{
    if (!ZombieData) return nullptr;
	return ZombieData->AnimationConfig.Animation_Walk;
}


void AZombieBase::SpawnBloodDecal()
{
    if (!SkinnedDecalSampler)
        return;

    // 从 DecalSize 的 X 和 Y 中随机生成一个贴花大小（蓝图逻辑）
    float MinSize = ZombieData->DecalConfig.DecalSize.X;
    float MaxSize = ZombieData->DecalConfig.DecalSize.Y;
    float Size = FMath::FRandRange(MinSize, MaxSize);

    // 将法线向量转换为四元数（作为旋转）
    FQuat Rotation = HitNormal.Rotation().Quaternion();

    // 调用 SpawnDecal
    SkinnedDecalSampler->SpawnDecal(
        HitLocation,           // Location
        Rotation,              // Rotation (Quat)
        HitBoneName,           // BoneName
        Size,                  // Size
        0,                     // SubUV (默认为0)
        -1                     // Index (默认为-1)
    );
}

void AZombieBase::InitializeDecalComponent()
{
    // 1. 获取角色的主 Mesh（对应蓝图的 self.Mesh）
    if (!GetOwner()) return;
    USkeletalMeshComponent* OwnerMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    if (!OwnerMesh) return;

    // 2. 获取 Mesh 的直接子组件（对应 GetChildrenComponents(false)）
    TArray<USceneComponent*> SkMChildren;
    OwnerMesh->GetChildrenComponents(false, SkMChildren);

    // 3. 遍历子组件
    for (USceneComponent* Child : SkMChildren)
    {
        // 4. 尝试转换为骨骼网格组件（对应 DynamicCast）
        USkeletalMeshComponent* SkelMeshChild = Cast<USkeletalMeshComponent>(Child);
        if (!SkelMeshChild) continue;

        // 5. 检查是否含有 "DecalTarget" 标签
        if (!SkelMeshChild->ComponentHasTag(FName("DecalTarget"))) continue;

        // 6. 注册到 SkinnedDecalSampler
        if (SkinnedDecalSampler)
        {
            SkinnedDecalSampler->SetMeshComponent(SkelMeshChild, false); // false = 不是主网格的子级？
        }
        break;
    }
}

void AZombieBase::CalculateHitDirection()
{
    // 1. 获取射击方向在水平面上的投影（忽略 Z）
    FVector ShotDir = ShotFromDirection;
    ShotDir.Z = 0.0f;
    if (ShotDir.IsNearlyZero())
    {
        HitDirection = EHitDirection::Front;
        return;
    }

    // 2. 获取角色前方向在水平面上的投影
    FRotator ActorRot = GetActorRotation();
    FVector Forward = UKismetMathLibrary::GetForwardVector(ActorRot);
    Forward.Z = 0.0f;
    Forward.Normalize();

    // 3. 计算二维向量
    FVector2D VecA(ShotDir.X, ShotDir.Y);
    FVector2D VecB(Forward.X, Forward.Y);

    // 4. 检查是否零向量（保险）
    if (VecA.IsNearlyZero() || VecB.IsNearlyZero())
    {
        HitDirection = EHitDirection::Front;
        return;
    }

    // 5. 计算点积、叉积、长度
    double Dot = FVector2D::DotProduct(VecA, VecB);
    double Cross = FVector2D::CrossProduct(VecA, VecB);
    double MagA = VecA.Size();
    double MagB = VecB.Size();

    // 6. 计算夹角
    double CosTheta = Dot / (MagA * MagB);
    CosTheta = FMath::Clamp(CosTheta, -1.0, 1.0);
    double AngleRad = FMath::Acos(CosTheta);
    double AngleDeg = FMath::RadiansToDegrees(AngleRad);

    // 7. 根据角度和叉积判断方向
    if (AngleDeg <= ZombieData->HitReactionBase.MiddleAngleThreshold)
    {
        HitDirection = EHitDirection::Front;
    }
    else if (AngleDeg > ZombieData->HitReactionBase.BackAngleThreshold)
    {
        HitDirection = EHitDirection::Back;
    }
    else if (Cross > 0.0)   // 叉积 > 0 表示右侧
    {
        HitDirection = EHitDirection::Right;
    }
    else
    {
        HitDirection = EHitDirection::Left;
    }

}

void AZombieBase::CalculateChestStrikePosition()
{
    // 1. 获取 Mesh 组件
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        E_ChestStrikePosition = EChestStrikePosition::Spine_Left;  // 默认
        return;
    }

    // 2. 获取 spine_02 骨骼的世界变换
    FTransform SpineTransform = MeshComp->GetSocketTransform("spine_02", RTS_World);

    // 3. 将 HitLocation 转换到骨骼局部空间
    FVector LocalPos = UKismetMathLibrary::InverseTransformLocation(SpineTransform, HitLocation);

    // 4. 取 Z 坐标（上下方向）
    double LocalZ = LocalPos.Z;

    // 5. 与 SplitArea 比较
    if (LocalZ < -ZombieData->HitReactionBase.SplitArea)
    {
        E_ChestStrikePosition = EChestStrikePosition::Spine_Left;   // 对应 NewEnumerator0
    }
    else if (LocalZ > ZombieData->HitReactionBase.SplitArea)
    {
        E_ChestStrikePosition = EChestStrikePosition::Spine_Right;  // 对应 NewEnumerator3
    }
    else
    {
        E_ChestStrikePosition = EChestStrikePosition::Spine_Middle; // 对应 NewEnumerator4
    }


}

USkeletalMeshComponent* AZombieBase::FindMeshByTag(FName Tag)
{
    TArray<USkeletalMeshComponent*> Components;
    GetComponents<USkeletalMeshComponent>(Components);
    for (USkeletalMeshComponent* Comp : Components)
    {
        if (Comp->ComponentTags.Contains(Tag))
            return Comp;
    }
    return nullptr;
}

void AZombieBase::DismemberLimb(int32 DismemberPower, bool bCanDismember)
{
    // 死亡后处理
    if (bIsDeath)
    {
        UGoreComponent* GoreComp = FindComponentByClass<UGoreComponent>();
        if (GoreComp)
        {
            FRotator SpawnRot = HitNormal.Rotation();
            GoreComp->SpawnBloodPoolFromBleedingHits(0.6f, 0.9f, HitLocation, SpawnRot, 500.0f, FVector2D(0.5f, 2.0f));
            GoreComp->SpawnPenetratingBloodPool(false, HitLocation, ShotFromDirection, FVector2D(0.5f, 1.5f), 200.0f, 1.0f);
        }
        // 死亡后是否还能断肢由 bDismemberOnDeath 和 DismemberPower 控制
        if (!ZombieData->DismemberConfig.bDismemberOnDeath || DismemberPower <= 1) return;
    }

    FName Bone = HitBoneName;
    bool bIsHead = (Bone == ZombieData->BoneNames.HeadBoneName);
    bool bShouldDismember = false;

     // 扣减计数器
    if (bIsHead)
    {
        HeadBreakBullets = FMath::Max(HeadBreakBullets - DismemberPower, 0);
        if (bCanDismember && HeadBreakBullets <= 0)
        {
            bShouldDismember = true;
            SpawnHeadFragment();
        }
    }
    else
    {
        auto ProcessLimb = [&](int32& LimbCounter, const TArray<FName>& LimbArray) -> bool
            {
                if (LimbArray.Contains(Bone))
                {
                    LimbCounter = FMath::Max(LimbCounter - DismemberPower, 0);
                    return (bCanDismember && LimbCounter <= 0);
                }
                return false;
            };

        if (ProcessLimb(RightArmBreakBullets, ZombieData->BoneNames.HitFullRightHandNameArr) ||
            ProcessLimb(LeftArmBreakBullets, ZombieData->BoneNames.HitFullLeftHandNameArr) ||
            ProcessLimb(RightLegBreakBullets, ZombieData->BoneNames.HitFullRightFootNameArr) ||
            ProcessLimb(LeftLegBreakBullets, ZombieData->BoneNames.HitFullLeftFootNameArr))
        {
            bShouldDismember = true;
        }
    }

    // ---- 执行断肢或暴击/血池 ----
    if (bShouldDismember)
    {
        // 成功触发断肢
        float ImpulseMag = bIsHead ? FMath::FRandRange(500.0f, 1000.0f) : 0.0f;
        FVector Impulse = ShotFromDirection * ImpulseMag;

        if (UGoreComponent* GoreComp = FindComponentByClass<UGoreComponent>())
        {
            GoreComp->DismemberLimb(Bone, Impulse);
        }

        // 只有头部或腿部断肢才立即死亡；手臂断肢不死亡
        if (bIsHead || ZombieData->BoneNames.HitFullRightFootNameArr.Contains(Bone) || ZombieData->BoneNames.HitFullLeftFootNameArr.Contains(Bone))
        {
            HandleDeathFunc(ShotFromDirection);
        }
        // 手臂断肢后不调用死亡，丧尸仍存活（可继续战斗，后续可替换无臂动画）
    }
    else
    {
        // 未达到断肢条件，检查暴击
        if (CritSuccess())
        {
            // 暴击无视附件限制，直接肢解
            float ImpulseMag = bIsHead ? FMath::FRandRange(500.0f, 1000.0f) : 0.0f;
            FVector Impulse = ShotFromDirection * ImpulseMag;

            if (UGoreComponent* GoreComp = FindComponentByClass<UGoreComponent>())
            {
                GoreComp->DismemberLimb(Bone, Impulse);
            }

            // 暴击同样只有头/腿致死
            if (bIsHead || ZombieData->BoneNames.HitFullRightFootNameArr.Contains(Bone) || ZombieData->BoneNames.HitFullLeftFootNameArr.Contains(Bone))
            {
                HandleDeathFunc(ShotFromDirection);
            }
        }
        else
        {
            // 非断肢非暴击：生成血池
            if (UGoreComponent* GoreComp = FindComponentByClass<UGoreComponent>())
            {
                FRotator SpawnRot = HitNormal.Rotation();
                GoreComp->SpawnBloodPoolFromBleedingHits(0.6f, 0.9f, HitLocation, SpawnRot, 500.0f, FVector2D(0.5f, 2.0f));
                GoreComp->SpawnPenetratingBloodPool(false, HitLocation, ShotFromDirection, FVector2D(0.5f, 1.5f), 200.0f, 1.0f);
            }
        }
    }

}

void AZombieBase::SpawnHeadFragment_Implementation()
{
    if (!ZombieData->DismemberConfig.HeadFragmentClass) return;

    // 1. 获取骨骼网格组件
    USkeletalMeshComponent* TargetMesh = FindMeshByTag("DismemberMesh");
    if (!TargetMesh) return;

    // 2. 获取 head 骨骼的世界变换
    FTransform HeadTransform = TargetMesh->GetBoneTransform(FName("head"), ERelativeTransformSpace::RTS_World);
    FVector SpawnLocation = HeadTransform.GetLocation() + FVector(0, 0, 10); // Z轴抬高 10
    FRotator SpawnRotation = HeadTransform.GetRotation().Rotator();

    // 3. 设置缩放 (0.4, 0.4, 0.4)
    FVector SpawnScale = FVector(0.4f);

    // 4. 生成头碎片 Actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    // 如果你希望将生成的头碎片归属到当前角色，可以设置 Owner = this;

    AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
        ZombieData->DismemberConfig.HeadFragmentClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (!SpawnedActor) return;
    SpawnedActor->SetActorScale3D(SpawnScale);

    // 如果该 Actor 有 EnableImpluse 函数，通过反射调用
    UFunction* Func = SpawnedActor->FindFunction(FName("EnableImpluse"));
    if (Func)
    {
        struct { FVector Impulse; } Params;
        Params.Impulse = FVector(0, 0, 15);
        SpawnedActor->ProcessEvent(Func, &Params);
    }

    SpawnedActor->SetLifeSpan(0.5f);
    SpawnedActor->SetActorTickEnabled(false);
}

float AZombieBase::InternalTakePointDamage(float Damage, FPointDamageEvent const& PointDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // 获取点伤害详细信息
    const FHitResult& Hit = PointDamageEvent.HitInfo;
    HitLocation = Hit.ImpactPoint;
    HitNormal = Hit.ImpactNormal;
    HitBoneName = Hit.BoneName;
    ShotFromDirection = PointDamageEvent.ShotDirection;  // 射击方向
    
    int32 WeaponDismemberPower = 0;
    bool bCanDismember = false;

    if (AWeaponBase* Weapon = Cast<AWeaponBase>(DamageCauser))
    {
        WeaponDismemberPower = Weapon->DismemberPower;
        bCanDismember = Weapon->bCanDismember;
    }


    // 获取武器的断肢权限
    bool bAllowDismember = false;
    if (AWeaponBase* Weapon = Cast<AWeaponBase>(DamageCauser))
    {
        bAllowDismember = Weapon->bCanDismember;
    }

    // 调用断肢逻辑
    DismemberLimb(WeaponDismemberPower, bCanDismember);

    if (bIsDeath)
    {
        SpawnBloodDecal();
        
        FVector Impulse = ShotFromDirection.GetSafeNormal() * ZombieData->HitReactionBase.ImpulseMagnitude;
        TWeakObjectPtr<UMeshComponent> HitComponent = Cast<UMeshComponent>(Hit.Component);
        if (HitComponent.IsValid())
        {
            HitComponent->AddImpulse(Impulse, HitBoneName, false);
        }

        return 0.0f;
    }

    //========== 血量处理（普通武器无法断肢，只能靠伤害致死） ==========
    CurrentHealth -= Damage;

    // 打印当前血量
    if (GEngine && bDebugShowHealth)
    {
        FString Msg = FString::Printf(TEXT("Health: %.1f / %.1f"), CurrentHealth, ZombieData->CombatConfig.MaxHealth);
        GEngine->AddOnScreenDebugMessage(1, 3.0f, FColor::Cyan, Msg);   // Key=1 保证同一条消息覆盖刷新
    }

    if (CurrentHealth <= 0.0f)
    {
        CurrentHealth = 0.0f;
        HandleDeathFunc(ShotFromDirection);
        return 0.0f;
    }

    CalculateHitDirection();
    CalculateChestStrikePosition();

    bool OutIsValid = false;
    bool OutIsHitHead = false;
    MontageCollectionByHitPart(OutIsValid, OutIsHitHead);

    if (!OutIsValid)
    {
        CurrentHitReactionMontage = nullptr;
        return 0.0f;
    }

    if (OutIsHitHead)
    {
        CountBoneHitsAndGetDizzey(ZombieData->HitReactionBase.HitHeadPlus);
    }

    SpawnBloodDecal();

    // ---- 播放受击蒙太奇 ----
    if (CurrentHitReactionMontage)
    {
        UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
        if (AnimInst)
        {
            AnimInst->StopAllMontages(ZombieData->HitReactionBase.MontageBlendOutTime);

            // 播放蒙太奇（PlayRate = 1.0, StartingPosition = 0.0）
            AnimInst->Montage_Play(CurrentHitReactionMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
            FOnMontageBlendingOutStarted BlendOutDelegate;
            BlendOutDelegate.BindUObject(this, &AZombieBase::OnRetreatBlendOut);
            AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, ZombieData->AvoidanceConfig.RetreatMontage);
        }

        InHitState = true;
        ChangeSpeed(ZombieData->MovementConfig.HitStateSpeed);
    }

    // 调用父类以保持引擎默认处理（如应用伤害）
    return Super::InternalTakePointDamage(Damage, PointDamageEvent, EventInstigator, DamageCauser);
}

void AZombieBase::MontageCollectionByHitPart(bool& OutIsValid, bool& OutIsHitHead)
{
    OutIsValid = false;
    OutIsHitHead = false;
    FName Bone = HitBoneName;

    // ---- 判断命中部位 ----
    bool bIsChest = AllChestName.Contains(Bone);
    bool bIsLimbs = AllLimbsName.Contains(Bone);
    bool bIsHead = (Bone == ZombieData->BoneNames.HeadBoneName);

    // ---- 头部 ----

    if (bIsHead)
    {
        OutIsHitHead = true;
        FindDirectionHitHeadReaction(OutIsValid);
        return;
    }
    
    // ---- 四肢 ----
    if (bIsLimbs)
    {
        const TArray<UAnimMontage*>* RightHandArr = nullptr;
        const TArray<UAnimMontage*>* LeftHandArr = nullptr;
        const TArray<UAnimMontage*>* RightFootArr = nullptr;
        const TArray<UAnimMontage*>* LeftFootArr = nullptr;

        switch (HitDirection)
        {
        case EHitDirection::Front:
            RightHandArr = &ZombieData->HitMontages.HitFullRightHandMontage_Front;
            LeftHandArr = &ZombieData->HitMontages.HitFullLeftHandMontage_Front;
            RightFootArr = &ZombieData->HitMontages.HitFullRightFootMontage_Front;
            LeftFootArr = &ZombieData->HitMontages.HitFullLeftFootMontage_Front;
            break;
        case EHitDirection::Left:
            RightHandArr = &ZombieData->HitMontages.HitFullRightHandMontage_Side;
            LeftHandArr = &ZombieData->HitMontages.HitFullRightHandMontage_Side;
            RightFootArr = &ZombieData->HitMontages.HitFullRightFootMontage_Side;
            LeftFootArr = &ZombieData->HitMontages.HitFullRightFootMontage_Side;
            break;
        case EHitDirection::Right:
            RightHandArr = &ZombieData->HitMontages.HitFullLeftHandMontage_Side;
            LeftHandArr = &ZombieData->HitMontages.HitFullLeftHandMontage_Side;
            RightFootArr = &ZombieData->HitMontages.HitFullLeftFootMontage_Side;
            LeftFootArr = &ZombieData->HitMontages.HitFullLeftFootMontage_Side;
            break;
        case EHitDirection::Back:
            RightHandArr = &ZombieData->HitMontages.HitFullRightHandMontage_Back;
            LeftHandArr = &ZombieData->HitMontages.HitFullLeftHandMontage_Back;
            RightFootArr = &ZombieData->HitMontages.HitFullRightFootMontage_Back;
            LeftFootArr = &ZombieData->HitMontages.HitFullLeftFootMontage_Back;
            break;
        default:
            OutIsValid = false;
            OutIsHitHead = false;
            return;
        }

        bool bSuccess = FindHitLimbsMontage(*RightHandArr, *LeftHandArr, *RightFootArr, *LeftFootArr);
        OutIsValid = bSuccess;
        OutIsHitHead = false;
        return;
    }

    // ---- 胸部 ----
    if (bIsChest)
    {
        OutIsHitHead = false;
        bool bSuccess = false;

        switch (HitDirection)
        {
        case EHitDirection::Front:
            switch (E_ChestStrikePosition)
            {
            case EChestStrikePosition::Spine_Left:
                bSuccess = FindHitChestMontage(ZombieData->HitMontages.HitUpperChestMontage_Front_L,
                    ZombieData->HitMontages.HitMidChestMontage_Front_L,
                    ZombieData->HitMontages.HitLowerChestMontage_Front_L);
                break;
            case EChestStrikePosition::Spine_Right:
                bSuccess = FindHitChestMontage(ZombieData->HitMontages.HitUpperChestMontage_Front_R,
                    ZombieData->HitMontages.HitMidChestMontage_Front_R,
                    ZombieData->HitMontages.HitLowerChestMontage_Front_R);
                break;
            case EChestStrikePosition::Spine_Middle:
                bSuccess = FindHitChestMontage(ZombieData->HitMontages.HitUpperChestMontage_Front_M,
                    ZombieData->HitMontages.HitMidChestMontage_Front_M,
                    ZombieData->HitMontages.HitLowerChestMontage_Front_M);
                break;
            }
            break;
        case EHitDirection::Right:
            bSuccess = FindHitChestMontage(ZombieData->HitMontages.HitUpperChestMontage_Side_L,
                ZombieData->HitMontages.HitMidChestMontage_Side_L,
                ZombieData->HitMontages.HitLowerChestMontage_Side_L);
            break;
        case EHitDirection::Left:
            bSuccess = FindHitChestMontage(ZombieData->HitMontages.HitUpperChestMontage_Side_R,
                ZombieData->HitMontages.HitMidChestMontage_Side_R,
                ZombieData->HitMontages.HitLowerChestMontage_Side_R);
            break;
        case EHitDirection::Back:
            switch (E_ChestStrikePosition)
            {
            case EChestStrikePosition::Spine_Left:
                bSuccess = FindHitChestMontage(ZombieData->HitMontages.HitUpperChestMontage_Back_L,
                    ZombieData->HitMontages.HitMidChestMontage_Back_L,
                    ZombieData->HitMontages.HitLowerChestMontage_Back_L);
                break;
            case EChestStrikePosition::Spine_Right:
                bSuccess = FindHitChestMontage(ZombieData->HitMontages.HitUpperChestMontage_Back_R,
                    ZombieData->HitMontages.HitMidChestMontage_Back_R,
                    ZombieData->HitMontages.HitLowerChestMontage_Back_R);
                break;
            case EChestStrikePosition::Spine_Middle:
                bSuccess = FindHitChestMontage(ZombieData->HitMontages.HitUpperChestMontage_Back_M,
                    ZombieData->HitMontages.HitMidChestMontage_Back_M,
                    ZombieData->HitMontages.HitLowerChestMontage_Back_M);
                break;
            }
            break;
        default:
            bSuccess = false;
            break;
        }

        OutIsValid = bSuccess;
        OutIsHitHead = false;
        return;
    }

    // ---- 既不是胸部也不是四肢 ----
    OutIsValid = false;
    OutIsHitHead = false;
    return;
}

void AZombieBase::FindDirectionHitHeadReaction(bool& OutIsValid)
{
    OutIsValid = false;
    CurrentHitReactionMontage = nullptr;

    // 根据命中方向选择对应的蒙太奇数组
    TArray<UAnimMontage*>* SelectedArray = nullptr;

    switch (HitDirection)
    {
    case EHitDirection::Front:
        SelectedArray = &ZombieData->HitMontages.HitHeadMontage_Front;
        break;

    case EHitDirection::Right:
        SelectedArray = &ZombieData->HitMontages.HitHeadMontage_Left;
        break;

    case EHitDirection::Left:
        SelectedArray = &ZombieData->HitMontages.HitHeadMontage_Right;
        break;

    case EHitDirection::Back:
        SelectedArray = &ZombieData->HitMontages.HitHeadMontage_Back;
        break;

    default:  // None 或其他未定义值
        OutIsValid = false;
        return;
    }

    // 检查数组是否为空
    if (!SelectedArray || SelectedArray->IsEmpty())
    {
        OutIsValid = false;
        return;
    }

    // 从数组中随机选择一个蒙太奇
    int32 RandomIndex = FMath::RandRange(0, SelectedArray->Num() - 1);
    CurrentHitReactionMontage = (*SelectedArray)[RandomIndex];

    // 如果选中的蒙太奇有效，则返回 true
    OutIsValid = (CurrentHitReactionMontage != nullptr);
}

bool AZombieBase::FindHitLimbsMontage(const TArray<UAnimMontage*>& MT_RightHand, const TArray<UAnimMontage*>& MT_LeftHand, const TArray<UAnimMontage*>& MT_RightFoot, const TArray<UAnimMontage*>& MT_LeftFoot)
{
    // 右手
    if (ZombieData->BoneNames.HitFullRightHandNameArr.Contains(HitBoneName))
    {
        if (!MT_RightHand.IsEmpty())
        {
            int32 Index = FMath::RandRange(0, MT_RightHand.Num() - 1);
            CurrentHitReactionMontage = MT_RightHand[Index];
            return true;
        }
        return false;
    }
    // 左手
    if (ZombieData->BoneNames.HitFullLeftHandNameArr.Contains(HitBoneName))
    {
        if (!MT_LeftHand.IsEmpty())
        {
            int32 Index = FMath::RandRange(0, MT_LeftHand.Num() - 1);
            CurrentHitReactionMontage = MT_LeftHand[Index];
            return true;
        }
        return false;
    }
    // 右脚
    if (ZombieData->BoneNames.HitFullRightFootNameArr.Contains(HitBoneName))
    {
        if (!MT_RightFoot.IsEmpty())
        {
            int32 Index = FMath::RandRange(0, MT_RightFoot.Num() - 1);
            CurrentHitReactionMontage = MT_RightFoot[Index];
            return true;
        }
        return false;
    }
    // 左脚
    if (ZombieData->BoneNames.HitFullLeftFootNameArr.Contains(HitBoneName))
    {
        if (!MT_LeftFoot.IsEmpty())
        {
            int32 Index = FMath::RandRange(0, MT_LeftFoot.Num() - 1);
            CurrentHitReactionMontage = MT_LeftFoot[Index];
            return true;
        }
        return false;
    }
    return false;

}

bool AZombieBase::FindHitChestMontage(const TArray<UAnimMontage*>& UpperChestMontage, const TArray<UAnimMontage*>& MidChestMontage, const TArray<UAnimMontage*>& LowerChestMontage)
{
    // 检查是否属于上胸部骨骼组
    if (ZombieData->BoneNames.HitUpperChestNameArr.Contains(HitBoneName))
    {
        if (!UpperChestMontage.IsEmpty())
        {
            int32 Index = FMath::RandRange(0, UpperChestMontage.Num() - 1);
            CurrentHitReactionMontage = UpperChestMontage[Index];
            return true;
        }
        return false;
    }
    // 检查是否属于中胸部骨骼组
    if (ZombieData->BoneNames.HitMidChestNameArr.Contains(HitBoneName))
    {
        if (!MidChestMontage.IsEmpty())
        {
            int32 Index = FMath::RandRange(0, MidChestMontage.Num() - 1);
            CurrentHitReactionMontage = MidChestMontage[Index];
            return true;
        }
        return false;
    }
    // 检查是否属于下胸部骨骼组
    if (ZombieData->BoneNames.HitLowerChestNameArr.Contains(HitBoneName))
    {
        if (!LowerChestMontage.IsEmpty())
        {
            int32 Index = FMath::RandRange(0, LowerChestMontage.Num() - 1);
            CurrentHitReactionMontage = LowerChestMontage[Index];
            return true;
        }
        return false;
    }
    // 都不属于
    return false;
}

void AZombieBase::CountBoneHitsAndGetDizzey(int32 Plus)
{
    // 累加命中计数
    HitCountIntoDizzey += Plus;

    // 判断是否达到眩晕阈值
    if (HitCountIntoDizzey >= ZombieData->HitReactionBase.IntoHeadDizzeyCount)
    {
        // 设置眩晕蒙太奇
        CurrentHitReactionMontage = ZombieData->HitReactionBase.HitInStunned;

        // 重置计数
        HitCountIntoDizzey = 0;
    }
}


void AZombieBase::ChangeSpeed(float InSpeed)
{
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = InSpeed;
    }
}


void AZombieBase::StartChase()
{
    if (bIsDeath) return;

    bChasePlayer = true;

    // 确保拥有 AI 控制器
    AAIController* AIC = Cast<AAIController>(GetController());
    if (!AIC)
    {
        AIC = GetWorld()->SpawnActor<AAIController>(AAIController::StaticClass());
        AIC->Possess(this);
    }

    ChangeSpeed(ZombieData->MovementConfig.WalkSpeed);

    // 启动定时循环
    GetWorldTimerManager().SetTimer(ChaseTimerHandle, this, &AZombieBase::ChaseTick, ZombieData->ChaseConfig.ChaseUpdateInterval, true);
}

void AZombieBase::StopChase()
{
    bChasePlayer = false;
    GetWorldTimerManager().ClearTimer(ChaseTimerHandle);

    AAIController* AIC = Cast<AAIController>(GetController());
    if (AIC)
    {
        AIC->StopMovement();
        AIC->K2_ClearFocus();
    }
}

void AZombieBase::ChaseTick()
{ // 死亡则停止追击


    if (bIsDeath)
    {
        StopChase();
        return;
    }

    // 获取玩家 Pawn
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;

    // 获取 AI 控制器
    AAIController* AIC = Cast<AAIController>(GetController());
    if (!AIC) return;

    if (bIsSprintAttacking || bIsAttacking || InHitState) return;

    // 计算距离
    float Dist = FVector::Distance(GetActorLocation(), PlayerPawn->GetActorLocation());


    if (Dist <= ZombieData->AttackConfig.AttackRange && bIsSprintAttacking)
    {
        ChangeSpeed(ZombieData->MovementConfig.HitStateSpeed);
    }

    // ---- 冲刺攻击判定 ----
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (Dist >= ZombieData->SprintAttackConfig.SprintMinRange && Dist <= ZombieData->SprintAttackConfig.SprintMaxRange &&
        ZombieData->SprintAttackConfig.SprintAttackMontages.Num() > 0 &&
        CurrentTime - LastSprintTime >= ZombieData->SprintAttackConfig.SprintCooldown &&!bIsAttacking &&
        (CurrentTime - LastRetreatTime >= ZombieData->AvoidanceConfig.RetreatCooldown) && !InHitState && ZombieData->SprintAttackConfig.bEnableSprintAttack)
    {
        bIsSprintAttacking = true;
        ChangeSpeed(ZombieData->MovementConfig.SprintSpeed);   // 冲刺速度
        LastSprintTime = CurrentTime;

        int32 RandomIndex = FMath::RandRange(0, ZombieData->SprintAttackConfig.SprintAttackMontages.Num() - 1);
        UAnimMontage* SprintMontage = ZombieData->SprintAttackConfig.SprintAttackMontages[RandomIndex];
        if (SprintMontage)
        {
            UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
            if (AnimInst)
            {
                AnimInst->StopAllMontages(ZombieData->HitReactionBase.MontageBlendOutTime);
                GetCharacterMovement()->bOrientRotationToMovement = false;
                GetCharacterMovement()->bUseControllerDesiredRotation = true;
                AIC->K2_SetFocus(PlayerPawn);
                AnimInst->Montage_Play(SprintMontage);

            }
        }
        return;  // 跳过本次 Tick，避免同时进入普通攻击
    }
    

    //普通攻击
    if (Dist <= ZombieData->AttackConfig.AttackRange && !bIsSprintAttacking &&  CurrentTime - LastRetreatTime >= ZombieData->AvoidanceConfig.RetreatCooldown && !InHitState)
    {

        const bool bCanLeft = (LeftArmBreakBullets > 0);
        const bool bCanRight = (RightArmBreakBullets > 0);

        // 如果两只手臂都断了，不发动攻击
        if (!bCanLeft && !bCanRight)
        {
            return;
        }

        // 决定用哪只手
        const bool bUseLeft = (bCanLeft && bCanRight) ? FMath::RandBool() : bCanLeft;

        const TArray<UAnimMontage*>& AttackMontages = bUseLeft
            ? ZombieData->AttackConfig.LeftAttackMontages
            : ZombieData->AttackConfig.RightAttackMontages;

        // 进入攻击范围，尝试攻击
        if (AttackMontages.Num() > 0)
        {
           
            AIC->StopMovement();
            bIsAttacking = true;
            ChangeSpeed(ZombieData->MovementConfig.HitStateSpeed);
           

            int32 RandomIndex = FMath::RandRange(0, AttackMontages.Num() - 1);
            UAnimMontage* AttackMontage = AttackMontages[RandomIndex];

            if (AttackMontage)
            {
                if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
                {                  
                    GetCharacterMovement()->bOrientRotationToMovement = false;
                    GetCharacterMovement()->bUseControllerDesiredRotation = true;
                    AIC->K2_SetFocus(PlayerPawn);

                    AnimInst->Montage_Play(AttackMontage);
                }
            }
        }
        return; // 已在范围内，不移动
    }

    if (Dist > ZombieData->ChaseConfig.ChaseAcceptanceRadius)
    {
        
        AIC->K2_ClearFocus();
        ChangeSpeed(ZombieData->MovementConfig.WalkSpeed);
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->bUseControllerDesiredRotation = false;
        AIC->MoveToActor(PlayerPawn, ZombieData->ChaseConfig.ChaseAcceptanceRadius, false, true, true);
    }


}

void AZombieBase::EnableAttackCollisionDetection_Implementation()
{
    PerformAttackDamage();
}

void AZombieBase::OnAnyMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!Montage) return;
    // 攻击蒙太奇结束
    if (bIsAttacking && ZombieData->AttackConfig.AttackMontages.Contains(Montage))
    {
        bIsAttacking = false;
        ChangeSpeed(ZombieData->MovementConfig.WalkSpeed);
        PlayRetreatToMakeSpace();

    }
    // 冲刺攻击蒙太奇结束
    else if (bIsSprintAttacking && ZombieData->SprintAttackConfig.SprintAttackMontages.Contains(Montage))
    {
        bIsSprintAttacking = false;
        ChangeSpeed(ZombieData->MovementConfig.WalkSpeed);
        PlayRetreatToMakeSpace();
    }
    // 受击蒙太奇结束（如果你也想用这个统一回调）
    else if (InHitState && CurrentHitReactionMontage == Montage)
    {
       
        InHitState = false;
        ChangeSpeed(ZombieData->MovementConfig.WalkSpeed);  // 或 RunSpeed，根据你的设计
    }
   
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    AlreadyAttackPawn = nullptr;
}

void AZombieBase::PerformAttackDamage()
{
    if (bIsDeath) return;

    // 对左右手各检测一次
    CheckAttackHit(ZombieData->AttackConfig.AttackSocketLeft);
    CheckAttackHit(ZombieData->AttackConfig.AttackSocketRight);
}

void AZombieBase::CheckAttackHit(FName SocketName)
{
    USkeletalMeshComponent* TargetMesh = FindMeshByTag("DismemberMesh");
    if (!TargetMesh) return;

    FVector Start = TargetMesh->GetSocketLocation(SocketName);
    FVector End = Start;

    // 忽略自身及所有丧尸同类
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this);   // 忽略自身

    // 将场景中所有丧尸加入忽略列表（简单但有点暴力，也可改用碰撞通道） //自行修改！！！
    for (TActorIterator<AZombieBase> It(GetWorld()); It; ++It)
    {
        if (*It != this)
            IgnoredActors.Add(*It);
    }

    FHitResult Hit;
    bool bHit = UKismetSystemLibrary::SphereTraceSingle(
        GetWorld(),
        Start,
        End,
        ZombieData->AttackConfig.AttackTraceRadius,
        ETraceTypeQuery::TraceTypeQuery1,
        false,
        IgnoredActors,
        EDrawDebugTrace::None,   // 调试时可临时改为 ForOneFrame
        Hit,
        true
    );

    if (bHit && Hit.GetActor())
    {
        // 双重保险：确保不是丧尸（万一漏网）
        if (Hit.GetActor()->IsA(AZombieBase::StaticClass())) return;

        APawn* HitPawn = Cast<APawn>(Hit.GetActor());
        if (HitPawn && HitPawn != AlreadyAttackPawn)
        {
            AlreadyAttackPawn = HitPawn;
            UGameplayStatics::ApplyDamage(
                HitPawn,
                ZombieData->AttackConfig.AttackDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
        }
    }
}




bool AZombieBase::CritSuccess() const
{
    // 计算概率：CritChance / 100.0
    double Probability = ZombieData->CombatConfig.CritChance / 100.0;
    // 生成 0~1 随机浮点数
    double RandomValue = FMath::FRand();
    // 如果概率大于随机数，则暴击成功（注意蓝图用 Greater，即 Probability > RandomValue）
    return Probability > RandomValue;
}

void AZombieBase::HandleDeathFunc(FVector InShotFromDirection)
{
    if (bIsDeath) return;

    USkeletalMeshComponent* TargetMesh = FindMeshByTag("DismemberMesh");
    if (!TargetMesh) return;

    // 2. 启用物理模拟
    TargetMesh->SetSimulatePhysics(true);

    // 3. 施加冲量（使用 InShotFromDirection 乘以 20000）
    FVector Impulse = InShotFromDirection * 20000.0f;
    TargetMesh->AddImpulse(Impulse, NAME_None, false); // bVelChange = false

    // 4. 设置骨骼网格碰撞为 QueryAndPhysics
    TargetMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TargetMesh->SetCollisionResponseToAllChannels(ECR_Block);
    TargetMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore); //不知道啥BUG
    // 5. 设置胶囊体碰撞为 NoCollision
    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    if (CapsuleComp)
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 6. 停止动画并设置为单节点模式
    TargetMesh->Stop();
    GetMesh()->Stop();

    TargetMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    // 强制初始化动画实例（等同于蓝图中的 bForceInitAnimScriptInstance = true）
    // 注意：SetAnimationMode 有一个重载版本可接受 bForceInit，但我们手动调用
    TargetMesh->InitAnim(true); // 强制重新初始化
    GetMesh()->Stop();
    GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

    // 7. 设置 bIsDeath = true（如果您的变量叫 IsDeath，请按实际名称）
    bIsDeath = true;
    StopChase();
    // 8. 将 Mesh 组件从父级分离（保持世界变换）
    TargetMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

    //9. 停止控制器移动（修复名称冲突）
    AController* MyController = GetController();
    if (MyController)
    {
        MyController->StopMovement();
    }

    // 定义销毁组件的 Lambda
    auto DestroyComponentByClass = [this](TSubclassOf<UActorComponent> CompClass)
        {
            UActorComponent* Comp = FindComponentByClass(CompClass);
            if (Comp)
            {
                Comp->DestroyComponent();
            }
        };

    //10. 
    DestroyComponentByClass(UCharacterMovementComponent::StaticClass());
    //需要更改
    
    SetLifeSpan(ZombieData->CombatConfig.CorpseLifeSpan);

    // 11. 禁用 Actor 的 Tick
    SetActorTickEnabled(false);

    OnDeath.Broadcast(bIsDeath);
    //死亡肢解后会陷入地面！！需要改

}

bool AZombieBase::SetIsSprintAttacking_Implementation(bool bNewValue)
{
    bIsSprintAttacking = bNewValue;
    ChangeSpeed(InHitState);
    return bIsSprintAttacking;
}


void AZombieBase::ApplyCrowdPush_Implementation(float InElbowPushStrength, float InForwardStrength)
{
    if (bIsDeath) return;
    PassiveAvoidanceCheck(InElbowPushStrength,InForwardStrength);
}

FVector AZombieBase::CalculateDirection(AActor* Target) const
{
    if (!Target) return FVector::ZeroVector;

    // 1. 计算从角色指向目标的向量
    const FVector CharLoc = GetActorLocation();
    const FVector TargetLoc = Target->GetActorLocation();
    const FVector ToTarget = TargetLoc - CharLoc;                     // L_Vector_AB
   
    // 4. 返回归一化后的方向（安全起见处理零向量）
    // 2. 判断目标在角色的左侧还是右侧（利用右向量的点积）
    const FVector RightVec = GetActorRightVector();
    const float Dot = FVector::DotProduct(ToTarget, RightVec);
    const bool bIsRight = (Dot >= 0.0f);                              // L_IsRight

    // 3. 根据左右选择避开方向（垂直于 ToTarget 的侧面向量）
   //    - 目标在右侧(bIsRight=true) → 逆时针旋转90度 = (Y, -X)
   //    - 目标在左侧(bIsRight=false) → 顺时针旋转90度 = (-Y, X)
    FVector AddDirection;
    if (bIsRight)
    {
        AddDirection = FVector(ToTarget.Y, -ToTarget.X, 0.0f);
    }
    else
    {
        AddDirection = FVector(-ToTarget.Y, ToTarget.X, 0.0f);
    }

    return AddDirection.GetSafeNormal();
}

void AZombieBase::TraceCheck(float Rotate,float InElbowPushStrength,float InForwardStrength)
{
    // 计算射线长度
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    float CapsuleRadius = Capsule ? Capsule->GetScaledCapsuleRadius() : 34.0f;
    float TraceLength = CapsuleRadius + ZombieData->AvoidanceConfig.TraceCheckDistance;

    FVector Start = GetActorLocation();
    FRotator Rot(0.0f, Rotate, 0.0f);
    FVector Forward = GetActorForwardVector();
    FVector RotatedForward = Rot.RotateVector(Forward);
    FVector End = Start + RotatedForward * TraceLength;

    // 射线检测
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);


    bool bHit = UKismetSystemLibrary::LineTraceSingle(
        this,
        Start,
        End,
        ZombieData->AvoidanceConfig.AvoidanceTraceChannel,
        false,
        TArray<AActor*>(),
        EDrawDebugTrace::ForOneFrame,   // 可改为 ForDuration 调试
        HitResult,
        true
    );

    if (bHit && HitResult.GetActor())
    {
        AActor* HitActor = HitResult.GetActor();

        // 肘击方向：指向被命中者的水平向量
        FVector ElbowDir = CalculateDirection(HitActor);
      

        // 只对 ZombieBase 触发被动移动（蓝图原逻辑为获取组件后调用）
        if (AZombieBase* OtherZombie = Cast<AZombieBase>(HitActor))
        {
            FVector ScaledElbow = ElbowDir * InElbowPushStrength;
            AddActorWorldOffset(ScaledElbow, false, nullptr, ETeleportType::None);
            SpeedUp(InForwardStrength);
        }
      
    }


}



void AZombieBase::PassiveAvoidanceCheck(float InElbowPushStrength, float InForwardStrength)
{
   
    for (float Angle : ZombieData->AvoidanceConfig.AvoidanceCheckAngles)
    {
        TraceCheck(Angle, InElbowPushStrength,InForwardStrength);
    }
}

void AZombieBase::SpeedUp(float InForwardStrength)
{
    if (!ZombieData->AvoidanceConfig.bEnableSpeedUp) return;

    // 获取当前速度矢量
    FVector Velocity = GetVelocity();

    // 归一化方向（如果速度为零则跳过，避免零向量归一化）
    FVector VelocityDir = Velocity.GetSafeNormal();
    if (VelocityDir.IsNearlyZero())
        return;

    // 获取当前速度标量（厘米/秒）
    float CurrentSpeed = Velocity.Size();
 
    // 将速度值钳制在 1.0 ~ 2.0 之间，作为位移缩放系数
    float SpeedScale = FMath::Clamp(CurrentSpeed, 1.0f, 2.0f);

    // 计算本帧的额外位移
    FVector DeltaLocation = VelocityDir * SpeedScale;

    // 应用世界偏移（不扫描，不传送）
    AddActorWorldOffset(DeltaLocation* InForwardStrength, false, nullptr, ETeleportType::None);
}

void AZombieBase::PlayRetreatToMakeSpace()
{
    if (InHitState) return;
    if (!ZombieData->AvoidanceConfig.bEnableRetreat) return;
    if (ZombieData->AvoidanceConfig.RetreatMontage)
    {
        CurrentHitReactionMontage = ZombieData->AvoidanceConfig.RetreatMontage;
        InHitState = true;
        if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
        {

            AnimInst->Montage_Play(ZombieData->AvoidanceConfig.RetreatMontage);
            FOnMontageBlendingOutStarted BlendOutDelegate;
            BlendOutDelegate.BindUObject(this, &AZombieBase::OnRetreatBlendOut);
            AnimInst->Montage_SetBlendingOutDelegate(BlendOutDelegate, ZombieData->AvoidanceConfig.RetreatMontage);

        }
    }
}
void AZombieBase::OnRetreatBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bInterrupted)
    {
        // 记录冷却时间
        LastRetreatTime = GetWorld()->GetTimeSeconds();
    }

}
