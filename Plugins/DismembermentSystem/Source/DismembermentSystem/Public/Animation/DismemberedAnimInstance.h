// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DismemberedAnimInstance.generated.h"

/*
	 这是用于**被切断的肢体**的动画实例。
	 它的作用是：从父骨架复制动画姿态，并应用控制绑定。
 */

UCLASS(Blueprintable)
class DISMEMBERMENTSYSTEM_API UDismemberedAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName Limb;
};
