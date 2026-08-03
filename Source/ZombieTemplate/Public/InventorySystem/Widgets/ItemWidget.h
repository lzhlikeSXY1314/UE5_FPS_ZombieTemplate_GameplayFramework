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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAmountChange, int32, InAmount);


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
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FString ItemDescription;

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
	int32 ShortcutIndex = -1;


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

	//Menu: 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Menu Button")
	bool bUseEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Menu Button")
	bool bShortcutEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Menu Button")
	bool bInspectEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Menu Button")
	bool bCombineEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Menu Button")
	bool bDiscardEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Menu Button")
	bool bSplitEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Menu Button")
	bool bEquipEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Menu Button")
	bool bAttachAttachmentEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data|Menu Button")
	bool bDetachAttachmentEnabled = false;

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


	UPROPERTY(BlueprintReadOnly,meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher_Equip;

	//UPROPERTY(BlueprintReadOnly,meta = (BindWidgetOptional))
	//TObjectPtr<UImage> KeyImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> EquippedImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(BlueprintReadOnly,meta = (BindWidgetOptional))
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



	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Widget|Animation")
	void PlayRotationAnimation(EItemRotation InRotation);
	virtual void PlayRotationAnimation_Implementation(EItemRotation InRotation);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Widget|Animation")
	void PlayRotationToDefaultAnimation();
	virtual void PlayRotationToDefaultAnimation_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Widget|Animation")
	void HideInfoOverlayAnimation();
	virtual void HideInfoOverlayAnimation_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Widget|Animation")
	void UpdateKeyImageInfo(bool Show, bool IsEquipped, int32 Index);
	virtual void UpdateKeyImageInfo_Implementation(bool Show, bool IsEquipped, int32 Index);


	UPROPERTY(BlueprintAssignable, Category = "ItemWIdget|Delegate")
	FOnAmountChange OnAmountChange;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> CachedMaterial;



	void LoadItemIcon();
	void LoadBulletIcon();


};
