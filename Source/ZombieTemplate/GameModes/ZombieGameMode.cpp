// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieGameMode.h"
#include "Controllers/ZombiePlayerController.h"
#include "Characters/ZombiePlayer.h"

AZombieGameMode::AZombieGameMode()
{
	PlayerControllerClass = AZombiePlayerController::StaticClass();
	DefaultPawnClass = AZombiePlayer::StaticClass();
}

void AZombieGameMode::BeginPlay()
{
}
