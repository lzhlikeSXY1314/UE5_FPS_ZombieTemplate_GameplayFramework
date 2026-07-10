// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ZombieGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ZOMBIETEMPLATE_API AZombieGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AZombieGameMode();

	virtual void BeginPlay() override;

};
