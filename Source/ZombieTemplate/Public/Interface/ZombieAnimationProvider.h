// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ZombieAnimationProvider.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UZombieAnimationProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ZOMBIETEMPLATE_API IZombieAnimationProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// 注意：用 BlueprintNativeEvent 以便在蓝图中重写，或用 BlueprintImplementableEvent 纯蓝图实现
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation")
	UAnimSequenceBase* GetIdleAnimation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation")
	UAnimSequenceBase* GetWalkAnimation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation")
	bool IsSprintAttacking() const;
	virtual bool IsSprintAttacking_Implementation() const = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation")
	bool SetIsSprintAttacking(bool bNewValue);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation")
	void EnableAttackCollisionDetection();

	// 对周围丧尸施加推力
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation")
	void ApplyCrowdPush(float InElbowPushStrength, float InForwardStrength);

	/** 咬合动画：驱动嘴部变形目标（jawOpen） */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animation")
	void Bite_Anim();

};
