// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventorySystem/Functions/InventoryStaticFunctions.h"
#include <InventorySystem/Widgets/ItemWidget.h>
#include "Items/InspectableItem.h"
#include <Components/GridPanel.h>
#include "InventoryHUDComponent.generated.h"

//ÉùÃ÷Î¯ÍÐ
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryDragDetected); //ÍÏ×§


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

#pragma endregion

class UInventoryWidget;
class UInventorySlotWidget;
class UInventoryGridPanelWidget;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool UseTempSlots;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FSlotStruct> InventorySlots;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FSlotStruct> TempInventorySlots;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FSlotStruct> HiddenSlots;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	E_SlotsType HiddenItemSlotsType;

	//UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	//TArray<FItemDataInfo> SavedPrimaryItemsArray;

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
	void SelectSlot(int32 InIndex, E_SlotsType InSlotType, bool InShouldPlaySound);

	void DeselectAllItemWidget();
	void DeselectAllSlots();
	void SetNameAndDescriptionText(const FText ItemName, const FText ItemDesc);

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Sound")
	TObjectPtr<UAudioComponent> UI_Sound = nullptr;

	UFUNCTION(Category = "Inventory|Sound")
	void PlayInventorySound(E_InventorySoundType SoundType, bool bUnstoppable = false);

	UFUNCTION()
	void SetRealSelectedSlot(int32 InIndex, E_SlotsType InSlotType);
#pragma region Delegate
public:
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Drag")
	FOnInventoryDragDetected OnDragDetected;



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
	void AddItem_HelperFunction(AInspectableItem* Item,  const EItemRotation Rotation, const TArray<int32>& Slots, const E_SlotsType SlotsType, const int32 Amount);

	UFUNCTION()
	void AddItemToSlots(AInspectableItem* Item,  int32 ItemAmount,const E_SlotsType SlotType);

	bool FindItemToStack(AInspectableItem*& Item, const TArray<FSlotStruct>& Slots, int32& CanAdd, bool IsStackable, const int32 MaxStack, const FText ItemName);
	
	void SetItemAmount(AInspectableItem* Item, const int32 Amount);

	UFUNCTION()
	void AddItemWidgetToGrib(AInspectableItem* Item, const bool CanDestory, const bool IsEquipped);

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

#pragma endregion


#pragma region DragWidget

private:
	UPROPERTY()
	UItemWidget* HidedItemWidgetWhenDragActive;

	UPROPERTY()
	UItemWidget* DragWidget;

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
#pragma endregion

};
