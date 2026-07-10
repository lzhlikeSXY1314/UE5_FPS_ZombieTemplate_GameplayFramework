// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;

UCLASS()
class ZOMBIETEMPLATE_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileBase();

    void BeginPlay();

    // ---------- 对象池接口 ----------
/** 从池中激活子弹（设置位置、方向、速度、伤害等） */
    void ActivateFromPool(const FVector& StartLocation, const FVector& Direction, float Speed);

    /** 停用子弹，返回池中 */
    void DeactivateToPool();

    /** 查询是否处于激活状态 */
    bool IsActive() const { return bIsActive; }


protected:

    // 碰撞球体（根组件）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    // 粒子拖尾组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* TrailParticle;

    // 投射物移动组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UProjectileMovementComponent* ProjectileMovement;

    // 击中冲量大小
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Impact")
    float HitImpulse = 1000.0f;

    // 击中时生成的粒子特效（在蓝图中指定）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Impact")
    class UParticleSystem* ImpactParticle;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


public:
    // 发射时设置方向与速度
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void FireInDirection(const FVector& ShootDirection);

 
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void AddIgnoreActor(AActor* Actor);

private:
    bool bIsActive = false;

    FTimerHandle AutoDeactivateTimer;
};
