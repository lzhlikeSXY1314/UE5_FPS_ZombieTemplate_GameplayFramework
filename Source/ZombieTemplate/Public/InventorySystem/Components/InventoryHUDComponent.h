// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventorySystem/Functions/InventoryStaticFunctions.h"
#include <InventorySystem/Widgets/ItemWidget.h>
#include "Items/InspectableItem.h"
#include <Components/GridPanel.h>
#include <InventorySystem/Structs/InventoryTypes.h>
#include "Widgets/ItemMenu.h"
#include "InventoryHUDComponent.generated.h"


//声明委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryDragDetected); //拖拽
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChangingAdditionalSlots);

#pragma region Structs
USTRUCT(BlueprintType)
struct FSlotStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	bool IsEmpty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	bool IsPartOfItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	AInspectableItem* ItemReference;

	FSlotStruct()
		: Index(-1), IsEmpty(true),
		IsPartOfItem(false),
		ItemReference(nullptr)
	{
	}
};

USTRUCT(BlueprintType)
struct FSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	int32 Index = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	E_SlotsType Type = E_SlotsType::Primary;
};


USTRUCT(BlueprintType)
struct FItemDataInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	AInspectableItem* Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	TArray<int32> Slots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	EItemRotation Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	E_SlotsType SlotsType;

	FItemDataInfo() :
		Item(nullptr),
		Rotation(EItemRotation::Horizontal),
		SlotsType(E_SlotsType::Primary)
	{
	}
};


USTRUCT(BlueprintType)
struct FShortcut
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Shortcut")
	bool IsEmpty;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Shortcut")
	int32 Index;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Shortcut")
	AInspectableItem* Item;

	FShortcut() : IsEmpty(true), Index(-1), Item(nullptr)
	{
	}
};


#pragma endregion

class UInventoryWidget;
class UInventorySlotWidget;
class UInventoryGridPanelWidget;
class UItemMenu;
class UItemActionConfirmWidget;
class UShortcutWidget;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZOMBIETEMPLATE_API UInventoryHUDComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


public:	
	// Sets default values for this component's properties
	UInventoryHUDComponent();

public:	

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FSlotStruct> InventorySlots;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FSlotStruct> TempInventorySlots;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FSlotStruct> HiddenSlots;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	E_SlotsType HiddenItemSlotsType;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	AInspectableItem* EquippedItem;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Inventory")
	TSubclassOf<UItemWidget> ItemWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<UItemWidget*> ItemsWidgets;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory HUD")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;
	
	void OpenInventory();
	void CloseInventory();

	void OnMouseButtonDown(FKey InKey);
	void OnMouseButtonUp(FKey InKey);

	void SelectSlot(int32 InIndex, E_SlotsType InSlotType, bool InShouldPlaySound);

	void DeselectAllItemWidget();
	void DeselectAllSlotsByType(E_SlotsType InSlotType);
	void SetNameAndDescriptionText(const FText ItemName, const FText ItemDesc);

	UPROPERTY( BlueprintReadOnly, Category = "Inventory|Sound")
	TObjectPtr<UAudioComponent> UI_Sound = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Sound")
	void PlayInventorySound(E_InventorySoundType SoundType, bool bUnstoppable = false);

	UFUNCTION()
	void SetRealSelectedSlot(int32 InIndex, E_SlotsType InSlotType);
#pragma region Delegate
public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Drag")
	FOnInventoryDragDetected OnDragDetected;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Drag")
	FOnChangingAdditionalSlots OnChangingAdditionalSlots;

private:
	UFUNCTION()
	void HandleDragDetected();

#pragma endregion



private:
	UFUNCTION()
	APlayerController* GetPlayerController();

	UFUNCTION(Category = "Inventory|Slots")
	TArray<UInventorySlotWidget*> GetSlotsByType(E_SlotsType SlotsType) const;

	UPROPERTY()
	TObjectPtr<UInventoryWidget> InventoryWidget = nullptr;

	UPROPERTY()
	TObjectPtr<UItemWidget> SelectedItemWidget = nullptr;

	UPROPERTY()
	E_SlotsType SelectSlotType = E_SlotsType::Primary;

	UPROPERTY()
	int32 SelectSlotIndex = 0;

	UPROPERTY()
	FSlotInfo RealSelectedSlot;

	UFUNCTION()
	UGridPanel* GetGribInventoryWidget(const E_SlotsType SlotType);


	UFUNCTION() 
	FSlotInfo GetRealSelectedSlot() { return RealSelectedSlot; }
	
	UPROPERTY()
	bool UseTempSlots;

	UFUNCTION()
	void DestroyAllTempObjects();

#pragma region InventoryFunctions
public:
	UFUNCTION()
	void InitializeSlots();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Functions")
	FIntPoint GetInventorySize(const E_SlotsType SlotsType = E_SlotsType::Primary);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Functions")
	TArray<FSlotStruct>& GetSlots(const E_SlotsType Type = E_SlotsType::Primary);

	UFUNCTION()
	bool CheckSlots(const int32 Index, const int32 Width, const int32 Height, TArray<int32>& EmptySlots,const E_SlotsType SlotType = E_SlotsType::Primary,const TArray<int32>& ExcludeSlots = TArray<int32>());

	UFUNCTION()
	bool CheckSize(const int32 Index, const int32 Width, const int32 Height, const E_SlotsType SlotsType);

	UFUNCTION()
	void FillSlots(AInspectableItem* Item, const TArray<int32>& Slots, const E_SlotsType SlotsType);

	UFUNCTION()
	void FillItemToSlots(AInspectableItem* Item, const int32 Amount, const EItemRotation Rotation, const TArray<int32>& Slots, TArray<FSlotStruct>& SlotsStruct); // AddItemToSlotsLocal

	UFUNCTION()
	bool FindEmptySlots(const E_SlotsType SlotsType, const int32 Width, const int32 Height, EItemRotation& Rotation, TArray<int32>& EmptySlots, const TArray<int32>& ExcludeSlots);

	UFUNCTION()
	bool CanAddItem(AInspectableItem* Item, const E_SlotsType SlotsType = E_SlotsType::Primary, int32 ItemAmount = 0);

	UFUNCTION()
	AInspectableItem* AddItem_HelperFunction(AInspectableItem* Item,  const EItemRotation Rotation, const TArray<int32>& Slots, const E_SlotsType SlotsType, const int32 Amount);

	UFUNCTION()
	void AddItemToSlots(AInspectableItem* Item,  int32 ItemAmount,const E_SlotsType SlotType);

	bool FindItemToStack(AInspectableItem*& Item, const TArray<FSlotStruct>& Slots, int32& CanAdd, bool IsStackable, const FString ItemName);
	
	void SetItemAmount(AInspectableItem* Item, const int32 Amount);

	UFUNCTION()
	void AddItemWidgetToGrib(AInspectableItem* Item, E_SlotsType SlotType);

	UFUNCTION()
	void ClearSlots(const E_SlotsType SlotsType, const TArray<int32>& Slots);

	UFUNCTION()
	void RemoveItemsInSlot(const int32 SlotIndex, const E_SlotsType SlotType, const int32 Amount, const bool RemoveAll);

	UFUNCTION()
	void RemoveItemByRef(const AInspectableItem* Item, const int32 Amount, const bool RemoveAll);

	UFUNCTION()
	UItemWidget* GetItemWidgetByIndex(const int32 Index, const E_SlotsType SlotType);


	UFUNCTION()
	bool GetItemInSlot(const int32 SlotIndex, const E_SlotsType SlotType, AInspectableItem*& Item);

	UFUNCTION()
	void ClearAllItemWidgets();

	UFUNCTION()
	void LoadItemWidgets();

	UFUNCTION()
	int32 GetRowBySlotType(int32 InIndex, E_SlotsType InSlotType);

	UFUNCTION()
	int32 GetColumnBySlotType(int32 InIndex, E_SlotsType InSlotType);

	UFUNCTION()
	UInventorySlotWidget* GetSlotWidgetByIndex(int32 InIndex, E_SlotsType InSlotType);

	UFUNCTION()
	void SplitItem(const int32 Index, E_SlotsType SlotType, const int32 Split, bool ReverseSplit);

	UFUNCTION()
	void MinoritySplitItem(const int32 Index, E_SlotsType SlotType);

	UFUNCTION()
	void DiscardItem(const int32 SlotIndex, const E_SlotsType SlotType, const int32 Amount = 1, const bool RemoveAll = false);

	UFUNCTION()
	void RemoveItemAmountFromInventory(const int32 InAmount, const FString TargetItemName);

	UFUNCTION()
	int32 FindAllItemAmountByName(FString& ItemName);

	UFUNCTION()
	void UpdateEquipStateByName(const FString& ItemName, bool ClearAllWeaponState = false);

	UFUNCTION()
	void UpdateAllWeaponWidgetAmmoByName(const FString& ItemName, int32 Amount);


#pragma endregion


#pragma region DragWidget

private:
	UPROPERTY()
	UItemWidget* HidedItemWidgetWhenDragActive;

	UPROPERTY()
	UItemWidget* DragWidget;

	UPROPERTY()
	UItemWidget* DragCancelWidget;

	UPROPERTY()
	FVector2D Offset;


	UFUNCTION()
	void InitializeDragWidget();

	UFUNCTION()
	void SnapDraggedItemToGridSlot(float InDeltaTime);

	UFUNCTION()
	void ApplyOffset(int32& InIndex);

	UFUNCTION()
	int32 CalculateItemIndex(int32 ClickIndex,  FVector2D InOffset, int32 InventoryCols, int32 InventoryRows, int32 ItemRows, int32 ItemCols, bool bIsRotated, bool DefaultIsVertical, bool IsLongVerticalItem);

	UFUNCTION()
	void EndBackItemWidgetToSlots();

	UFUNCTION()
	void RotateItemWidget(bool InPlaySound);

	UFUNCTION()
	void CancelDrag(bool InPlaySound);



#pragma endregion

#pragma region MoveItem

public:


private:
	bool IsEmptySlotsForItem(const int32 Index, const int32 Width, const int32 Height, const E_SlotsType SlotType, const EItemRotation Rotation, TArray<int32>& EmptySlots, const TArray<int32>& ExcludeSlots);
	
	bool CanItemAddedToSlots(const int32 SlotOfItem, const E_SlotsType ItemSlotType, const EItemRotation Rotation, const int32 SlotToAdd, const E_SlotsType SlotsTypeToAdd, TArray<int32>& EmptySlots);

	void MoveItem();

	void AddExistingItemToSlots(AInspectableItem* Item, const E_SlotsType SlotsType, const EItemRotation Rotation, const TArray<int32>& EmptySlots);

	TArray<int32> GetSlotsByItemSize(const int32 FirstSlot, const E_SlotsType SlotsType, const FIntPoint ItemSize, const EItemRotation Rotation);

	TArray<AInspectableItem*> GetAllItemsInSlots(TArray<int32> Slots, E_SlotsType SlotsType);

	bool CanSwapDraggedItem(const int32 ItemToIgnoreSlotIndex, const E_SlotsType ItemToIgnoreSlotsType,const int32 SelectedIndex, const E_SlotsType SelectedSlotsType,  const EItemRotation DraggedItemRotation, AInspectableItem*& OutTargetItem);

	void CreateSwapItemWidget(AInspectableItem* ItemUnderDragItem);

	void SwapDraggedItem(const int32 ItemIndex, const E_SlotsType SlotsType, const int32 SelectedIndex, const E_SlotsType SelectedSlotsType, const EItemRotation DraggedItemRotation, FIntPoint ItemSize);

	bool IsValidSwappedItem() const;

	bool CanStackDraggedItem(const int32 SelectedSlotIndex, const E_SlotsType SelectedSlotType, const int32 DraggedItemSlotIndex, const E_SlotsType DraggedItemSlotType, const EItemRotation DraggedItemRotation);

	int32 CalculateItemAmountAfterStuck(const int32 SelectedSlotIndex, const E_SlotsType SelectedSlotType, const int32 DraggedItemSlotIndex, const E_SlotsType DraggedItemSlotType, int32& SelectedItemAmount);

	void StackDraggedItem(const int32 SelectedSlotIndex, const E_SlotsType SelectedSlotType, const int32 DraggedItemSlotIndex, const E_SlotsType DraggedItemSlotType,  const EItemRotation DraggedItemRotation);

#pragma endregion

#pragma region SavePrimary

private:

	UPROPERTY()
	TArray<FItemDataInfo> SavedPrimaryItemsArray;

	UFUNCTION()
	void OnAdditionSlotsChanged();

	UFUNCTION()
	bool IsSlotsHaveItems(const E_SlotsType SlotsType);

	UFUNCTION()
	void SavePrimarySlotsInArray();

		UFUNCTION()
	void LoadPrimarySlotsFromArray();

	UFUNCTION()
	void ClearAllSlotsByType(const E_SlotsType SlotsType);

#pragma endregion

#pragma region ControlHint

private:
	void UpdateControlHints(EInventoryStatus InStatus = EInventoryStatus::Opened);

#pragma endregion

#pragma region Shortcuts
	UPROPERTY()
	TArray<FShortcut> Shortcuts;

	UPROPERTY()
	int32 SelectedShortcut;

	UFUNCTION()
	FORCEINLINE TArray<FShortcut>& GetShortcuts() { return Shortcuts; }

	UFUNCTION()
	int32 FindItemInShortcuts(AInspectableItem* Item);

	UFUNCTION()
	void RemoveItemFromShortcut(const int32 Index);

	UFUNCTION()
	TArray<AInspectableItem*> GetAllShortcutItems();
#pragma endregion

#pragma region Sort
public:
	void AutoSort();
#pragma endregion

#pragma region Menu
public:

	UFUNCTION()
	void CreateItemMenuWidget();

	UFUNCTION()
	void CloseItemMenuWidget(bool InPlaySound = true);

	UFUNCTION()
	void MenuButtonResponseFunction(E_ItemActionType ActionType);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemMenu> ItemMenuClass;

	UPROPERTY()
	UItemMenu* ItemMenuWidget;

#pragma endregion

#pragma region ConfirmWidget
public:

private:
	
	UPROPERTY()
	UItemActionConfirmWidget* ConfirmWidget = nullptr;

	UPROPERTY()
	bool bIgnoreMouseUp = false; //用于关闭Menu控件鼠标抬起又出现Menu

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemActionConfirmWidget> ConfirmWidgetClass;

	UPROPERTY()
	FSlotInfo CreateMenuInfo{ CreateMenuInfo.Index = -1, CreateMenuInfo.Type = E_SlotsType::Primary };

	UFUNCTION()
	void CreateItemActionConfirmWidget(AInspectableItem* Item, bool ShowNumBlock, int32 InMaxValue, E_ItemActionType ActionType, bool ShowMsg = false);

	UFUNCTION(BlueprintCallable, Category = "HUDInv|Comfirmation")
	void ComfirmationMes(bool InYes, E_ItemActionType ActionType);

#pragma endregion

#pragma region ShortcutWidget

public:
	UFUNCTION()
	void CreateShortcutWidget();

	UFUNCTION()
	void CancelShortcutWidget();

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Function")
	void LoadShortcutItemData(UShortcutWidget* InShortcutWidget);

	UFUNCTION(BlueprintCallable, Category = "Shortcut|Function")
	void UpdateItemShortcutIndexByName(FString ItemName, int32 ShortcutIndex);

	UFUNCTION(BlueprintCallable,Category = "Shortcut|Function")
	void UpdateAllItemWidgetShortcutState(); //用于Shortcut中实时更换背包视觉


	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UShortcutWidget> ShortcutWidgetClass;

	UPROPERTY()
	UShortcutWidget* ShortcutWidget = nullptr;



#pragma endregion
};
