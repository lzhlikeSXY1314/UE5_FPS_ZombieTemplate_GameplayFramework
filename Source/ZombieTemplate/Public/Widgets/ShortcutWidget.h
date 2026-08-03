// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySystem/Widgets/ItemWidget.h"
#include "ShortcutWidget.generated.h"



UCLASS()
class ZOMBIETEMPLATE_API UShortcutWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShortcutSlot|Data")
	int32 SelectedIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ShortcutSlot|Data")
	FInventoryItemPayload ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ShortcutSlot|Data")
	TMap<int32, FInventoryItemPayload> ShortcutItemMap;


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "ShortcutSlot|Function")
	void LoadShortcutItemData();
	virtual void LoadShortcutItemData_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "ShortcutSlot|Function")
	void RomoveShortcutItemData();
	virtual void RomoveShortcutItemData_Implementation();
};
