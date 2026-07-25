// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ZombieGameInstance.generated.h"

class UInventoryData;

UCLASS()
class ZOMBIETEMPLATE_API UZombieGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory System")
	TObjectPtr<UInventoryData> InventoryOptions;
};
