// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include <Kismet/GameplayStatics.h>
#include "Components/StaticMeshComponent.h"  


// Sets default values
AProjectileBase::AProjectileBase()
{

    PrimaryActorTick.bCanEverTick = false;

    // ---------- 根组件：碰撞球体 ----------
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(1.0f);                     // 半径 1 cm
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 初始关闭碰撞
    CollisionSphere->SetCollisionObjectType(ECC_GameTraceChannel2); // 自定义通道
    CollisionSphere->SetNotifyRigidBodyCollision(true);           // 碰撞事件

    RootComponent = CollisionSphere;

    // ---------- 粒子拖尾组件 ----------
    TrailParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailParticle"));
    TrailParticle->SetupAttachment(CollisionSphere);
    TrailParticle->bAutoActivate = false; 
    TrailParticle->SetRelativeScale3D(FVector(0.1f));


    // ---------- 投射物移动组件 ----------
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionSphere;
    ProjectileMovement->InitialSpeed = 0.0f;
    ProjectileMovement->MaxSpeed = 0.0f;
    ProjectileMovement->ProjectileGravityScale = 0.0f;   // 无重力，直线飞行
    ProjectileMovement->bRotationFollowsVelocity = true;  // 跟随速度旋转
    ProjectileMovement->bShouldBounce = false;

    // 绑定碰撞事件
    CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);

    // 默认隐藏（池中待机）
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(false);
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();
    // 确保初始状态为停用
    if (!bIsActive)
    {
        DeactivateToPool();
    }
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    // 忽略自身、发射者或无效碰撞
    if (!bIsActive || !OtherActor || OtherActor == this || OtherActor == GetOwner()) return;

    // 判断被击中的组件是否为静态网格体
    bool bIsStaticMesh = (Cast<UStaticMeshComponent>(OtherComp) != nullptr);

    // 1. 生成击中特效（仅对静态网格体）
    if (bIsStaticMesh && ImpactParticle)
    {
        UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ImpactParticle,
            Hit.ImpactPoint,
            Hit.ImpactNormal.Rotation(),
            true
        );
        if (PSC)
        {
            PSC->SetWorldScale3D(FVector(0.3f));
        }
    }

    // 2. 如果被击中的组件启用了物理模拟，施加冲量
    if (OtherComp && OtherComp->IsSimulatingPhysics())
    {
        FVector Impulse = GetVelocity().GetSafeNormal() * HitImpulse;
        OtherComp->AddImpulseAtLocation(Impulse, Hit.ImpactPoint);
    }

    DeactivateToPool();

}

void AProjectileBase::FireInDirection(const FVector& ShootDirection)
{
    if (ProjectileMovement)
    {
        ProjectileMovement->Velocity = ShootDirection * ProjectileMovement->InitialSpeed;
    }
}

void AProjectileBase::AddIgnoreActor(AActor* Actor)
{
    if (CollisionSphere && Actor)
    {
        CollisionSphere->MoveIgnoreActors.Add(Actor);
    }
}



void AProjectileBase::ActivateFromPool(const FVector& StartLocation, const FVector& Direction, float Speed)
{
    if (bIsActive) return;
    bIsActive = true;
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SetActorLocation(StartLocation);

    // 设置速度
    ProjectileMovement->Velocity = Direction * Speed;
    ProjectileMovement->SetActive(true);
    // 激活拖尾粒子
    TrailParticle->Activate(true);

    // 设置自动回收定时器（例如 0.5 秒后未命中则回收）
    GetWorld()->GetTimerManager().SetTimer(AutoDeactivateTimer, this, &AProjectileBase::DeactivateToPool, 0.5f, false);
}

void AProjectileBase::DeactivateToPool()
{
    if (!bIsActive) return;
    bIsActive = false;
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProjectileMovement->Velocity = FVector::ZeroVector;
    ProjectileMovement->SetActive(false);
    TrailParticle->Deactivate();
    GetWorld()->GetTimerManager().ClearTimer(AutoDeactivateTimer);
}


