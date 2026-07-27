// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySystem/structs/InventoryTypes.h"
#include "InventoryGridPanelWidget.generated.h"

class UGridPanel;
class UInventorySlotWidget;
class UOverlay;
class UInventoryData;
class UInventoryHUDComponent;

UCLASS()
class ZOMBIETEMPLATE_API UInventoryGridPanelWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInventoryData> InventoryOptions;

	UPROPERTY(BlueprintReadOnly,meta = (BindWidget))
	TObjectPtr<UOverlay> SlotsOverlay;



	UPROPERTY(EditDefaultsOnly, Category = "Inventory Grid")
	TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Data", meta = (ExposeOnSpawn = true))
	E_SlotsType SlotsType;



public:
	UFUNCTION(BlueprintCallable, Category = "Inventory Grid")
	void InitializeGrid();

	UPROPERTY()
	UInventoryHUDComponent* HUD_ComponentReference = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory Grid")
	TArray<UInventorySlotWidget*> SlotWidgets;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UGridPanel> Grid;
};
