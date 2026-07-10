// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombiePlayerController.h"

AZombiePlayerController::AZombiePlayerController()
{
    // 可以在这里初始化控制器相关设置
}

void AZombiePlayerController::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AZombiePlayerController 已启动！"));
}
