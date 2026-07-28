// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include"InventorySystem/Structs/InventoryTypes.h"

#include "ItemWidget.generated.h"

class UImage;
class UTextBlock;
class UWidgetSwitcher;
class USizeBox;
class UInventoryHUDComponent;

// 核心物品数据结构体
USTRUCT(BlueprintType)
struct FInventoryItemPayload
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TSoftObjectPtr<UTexture2D> ItemIconSoftRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TSoftObjectPtr<UTexture2D> BulletIconSoftRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FVector2D BulletImageSize = FVector2D(52.f, 26.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FIntPoint ItemIconSize = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FText ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	bool IsEquipped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	bool IsWeapon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (EditCondition = "IsWeapon"))
	int32 AmmoAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", meta = (EditCondition = "IsStackable", MinValue = 1, MaxValue = 99))
	int32 ItemAmount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	bool IsStackable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data",meta = (EditCondition = "IsStackable"))
    int32 MaxStack = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	bool CanBePickedToInventory = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	bool CanDestory = false;

	UPROPERTY(BlueprintReadOnly, Category = "Item Data")
	UInventoryHUDComponent* InventoryComponentReference;


	UPROPERTY(BlueprintReadOnly, Category = "Item Widget Data")
	TArray<int32> OccupiedSlots;

	UPROPERTY(BlueprintReadOnly, Category = "Item Widget Data")
	E_SlotsType SlotsType = E_SlotsType::Primary;

	UPROPERTY(BlueprintReadOnly, Category = "Item Widget Data")
	EItemRotation Rotation = EItemRotation::Horizontal;

	UPROPERTY(BlueprintReadOnly, Category = "Item Widget Data")
	bool IsLongVertical = false;

};


UCLASS()
class ZOMBIETEMPLATE_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Background;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> BulletImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Amount_Text;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher_Equip;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> KeyImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> EquippedImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBox_ItemShortcut;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBox_BulletRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Widget Data")
	EItemRotation Rotation;

	UPROPERTY(BlueprintReadOnly, Category = "Item Widget Data")
	EItemRotation DefaultRotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Widget|Material")
	TObjectPtr<UMaterialInterface> ItemMaterial;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Item Widget Data")
	FInventoryItemPayload InventoryItemPayload;

	UPROPERTY()
	FVector2D Offset;

	UFUNCTION(BlueprintCallable, Category = "Item Widget|Background")
	void SelectItemBackgroundMaterial(bool bIsSelected, bool bIsOpenMenu);

	UFUNCTION()
	void SetItemWidgetAspectRatio();

	UFUNCTION()
	void SetAmount();

	UFUNCTION()
	void SetActiveBackgroundMaterial(bool InActive);

	UFUNCTION()
	int32 GetFirstOccupiedSlotIndex();

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedMaterial;

	void LoadItemIconAsync();
	void LoadBulletIconAsync();



	void OnItemIconLoaded(TSoftObjectPtr<UTexture2D> SoftRef);
	void OnBulletIconLoaded(TSoftObjectPtr<UTexture2D> SoftRef);


};
