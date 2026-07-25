// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include <InventorySystem/Structs/InventoryTypes.h>
#include "InventoryData.generated.h"



class UInventorySlotWidget;
class UMaterialInstance;


USTRUCT(BlueprintType)
struct FInventoryGridConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "1", ClampMax = "10"))
	int32 Columns = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "1", ClampMax = "10"))
	int32 Rows = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grid", meta = (ClampMin = "20.0", ClampMax = "200.0"))
	float SlotSize = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory Grid")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;
};

UCLASS()
class ZOMBIETEMPLATE_API UInventoryData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Primary")
	FInventoryGridConfig PrimaryConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Temp")
	bool bEnableTempSlots = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Temp", meta = (EditCondition = "bEnableTempSlots", EditConditionHides))
	FInventoryGridConfig TempConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Sounds")
	TMap<E_InventorySoundType, USoundBase*> InventorySounds;

};
