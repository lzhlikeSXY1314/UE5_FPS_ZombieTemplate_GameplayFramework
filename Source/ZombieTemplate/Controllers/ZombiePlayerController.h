// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZombiePlayerController.generated.h"

UCLASS()
class ZOMBIETEMPLATE_API AZombiePlayerController : public APlayerController
{
	GENERATED_BODY()
	
	// 构造函数
	AZombiePlayerController();

	// 重写 BeginPlay
	virtual void BeginPlay() override;


};
