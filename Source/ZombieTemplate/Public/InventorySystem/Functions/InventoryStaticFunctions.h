// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Datas/InventoryData.h"
#include <InventorySystem/Structs/InventoryTypes.h>
#include "InventoryStaticFunctions.generated.h"


class USoundBase;
class UInventoryHUDComponent;

UCLASS()
class ZOMBIETEMPLATE_API UInventoryStaticFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory", meta = (WorldContext = "WorldContextObject"))
	static UInventoryData* GetInventoryOptions(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory|Sound", meta = (WorldContext = "WorldContextObject"))
	static USoundBase* GetInventorySound(UObject* WorldContextObject, E_InventorySoundType SoundType);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory", meta = (WorldContext = "WorldContextObject"))
	static UInventoryHUDComponent* GetInventoryHUDComponent(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Static Functions")
	static int32 GetColumn(const int32 ArrayIndex, const int32 Columns);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Static Functions")
	static int32 GetRow(const int32 ArrayIndex, const int32 Columns);
};
