// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "InventoryWidget.generated.h"



class UInventoryGridPanelWidget;
class UTextBlock;
class UImage;
class UNameAndDecription;
class UCanvasPanel;
class UControlsHints;

UCLASS()
class ZOMBIETEMPLATE_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
protected:



	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel_Root;

	// 主背包网格
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGridPanelWidget> WB_Primary;

	// 临时背包网格
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGridPanelWidget> WB_Temp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UControlsHints> WB_ControlHints;

	// 绑定物品信息面板
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UNameAndDecription> WB_ItemInfo;

	// 主背包标题文本
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BaseSlotsText;

	// 临时背包标题文本
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TempSlotsText;



	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetText(const FText& BaseSlots, const FText& TempSlots);

	void CloseInventory();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Widget|Animation")
	void ShowHideTempAnim(bool ShowTemp);
	virtual void ShowHideTempAnim_Implementation(bool ShowTemp);

	UFUNCTION(BlueprintCallable)
	void ShowHideTempSlots(bool Show = false);


private:

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TArray<FKey> CloseInventoryKeys;

	void OnMouseButtonDown(FKey InKey);

	void OnMouseButtonUp(FKey InKey);



};
