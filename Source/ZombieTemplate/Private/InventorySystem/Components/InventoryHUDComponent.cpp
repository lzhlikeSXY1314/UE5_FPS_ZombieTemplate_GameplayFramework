// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Components/InventoryHUDComponent.h"
#include "InventorySystem/Widgets/InventoryWidget.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include <Kismet/GameplayStatics.h>
#include "InventorySystem/Widgets/InventoryGridPanelWidget.h"
#include "InventorySystem/Widgets/InventorySlotWidget.h"
#include "InventorySystem/Functions/InventoryStaticFunctions.h" 
#include "Components/GridSlot.h"
#include "InventorySystem/Widgets/NameAndDecription.h"


void UInventoryHUDComponent::BeginPlay()
{
    InitializeSlots();
}

// Sets default values for this component's properties
UInventoryHUDComponent::UInventoryHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


}

void UInventoryHUDComponent::OpenInventory()
{
    if (!InventoryWidgetClass) return;

    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char) return;

    APlayerController* PC = Cast<APlayerController>(Char->GetController());
    if (!PC) return;

    if (!InventoryWidget)
    {
        InventoryWidget = CreateWidget<UInventoryWidget>(PC, InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->AddToViewport();
            InventoryWidget->SetVisibility(ESlateVisibility::Collapsed); // 默认隐藏
        }
    }

    if (InventoryWidget)
    {
        InventoryWidget->SetVisibility(ESlateVisibility::Visible); // 显示
        PlayInventorySound(E_InventorySoundType::Open,true);
        InventoryWidget->SetFocus();
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
        PC->SetPause(true);
        SetNameAndDescriptionText(FText::GetEmpty(), FText::GetEmpty());

        LoadItemWidgets();
    }

    int32 ViewportX, ViewportY;
    PC->GetViewportSize(ViewportX, ViewportY);

    int32 CenterX = ViewportX / 2;
    int32 CenterY = ViewportY / 2;

    PC->SetMouseLocation(CenterX, CenterY);

}

void UInventoryHUDComponent::CloseInventory()
{

    PlayInventorySound(E_InventorySoundType::Close,false);
}

void UInventoryHUDComponent::SelectSlot(int32 InIndex, E_SlotsType InSlotType, bool InShouldPlaySound)
{
    
    const int32 L_SelectSlotIndex = FMath::Max(InIndex, 0);
    SelectSlotType = InSlotType;
    SelectSlotIndex = FMath::Max(L_SelectSlotIndex, 0);

    for (UInventorySlotWidget* L_Slot : GetSlotsByType(SelectSlotType))
    {
        if (!L_Slot) continue;

        if (L_Slot->Index == SelectSlotIndex)
        {
            //看看悬浮在ItemWidget
            DeselectAllItemWidget();
            bool IsHoveredItemWidget = false;
            UItemWidget* ItemWidgetRef = nullptr;

            if (GetSlots(SelectSlotType).IsValidIndex(SelectSlotIndex))
            {
                IsHoveredItemWidget = !GetSlots(SelectSlotType)[SelectSlotIndex].IsEmpty;
            }
          
            if (IsHoveredItemWidget)
            {
                ItemWidgetRef = GetItemWidgetByIndex(SelectSlotIndex, SelectSlotType);
                if (!ItemWidgetRef) return;
                
                SetNameAndDescriptionText(GetSlots(SelectSlotType)[SelectSlotIndex].ItemReference->InventoryItemPayload.ItemName, GetSlots(SelectSlotType)[SelectSlotIndex].ItemReference->InventoryItemPayload.ItemDescription);
                ItemWidgetRef->SelectItemBackgroundMaterial(true,false);
                L_Slot->SelectSlot(false);

                if (SelectedItemWidget != ItemWidgetRef)
                {   
                    if (InShouldPlaySound)
                    {
                        PlayInventorySound(E_InventorySoundType::SelectItem);
                    }
                    SelectedItemWidget = ItemWidgetRef;

                }

            }
            else
            {
                L_Slot->SelectSlot(true);
                SetNameAndDescriptionText (FText::GetEmpty(), FText::GetEmpty());
                PlayInventorySound(InShouldPlaySound ? E_InventorySoundType::SelectSlot : E_InventorySoundType::None, false);
            }
         
        }
        else
        {
            L_Slot->SelectSlot(false);
        }
    }

    // 第二步：遍历所有其他类型的 Slot，全部取消选中
    E_SlotsType OtherType = (SelectSlotType == E_SlotsType::Primary) ? E_SlotsType::Temp : E_SlotsType::Primary;
    TArray<UInventorySlotWidget*> OtherTypeSlots = GetSlotsByType(OtherType);
    for (UInventorySlotWidget* L_Slot : OtherTypeSlots)
    {
        if (L_Slot)
        {
            L_Slot->SelectSlot(false);
        }
    }




}

void UInventoryHUDComponent::DeselectAllItemWidget()
{
    for (UItemWidget* ItemWidget : ItemsWidgets)
    {
        if (!ItemWidget) continue;
        ItemWidget->SelectItemBackgroundMaterial(false,false);
    }
}

void UInventoryHUDComponent::SetNameAndDescriptionText(const FText ItemName, const FText ItemDesc)
{
    if (!InventoryWidget) return;
    if (!InventoryWidget->WB_ItemInfo) return;
    InventoryWidget->WB_ItemInfo->SetItemInfo(ItemName, ItemDesc);
}

void UInventoryHUDComponent::PlayInventorySound(E_InventorySoundType SoundType, bool bUnstoppable)
{
    if (UI_Sound)
    {
        UI_Sound->Stop();
        UI_Sound = nullptr;
    }
    USoundBase* Sound = UInventoryStaticFunctions::GetInventorySound(this, SoundType);
    UAudioComponent* NewAudioComp = UGameplayStatics::SpawnSound2D(GetWorld(),Sound,1.0f,1.0f,0.0f,nullptr,false );
    if (!bUnstoppable && NewAudioComp)
    {
        UI_Sound = NewAudioComp;
    }
    else
    {
        UI_Sound = nullptr;
    }


}

TArray<UInventorySlotWidget*> UInventoryHUDComponent::GetSlotsByType(E_SlotsType SlotsType) const
{
    if (!InventoryWidget) return {};
    switch (SlotsType)
    {
    case E_SlotsType::Primary:
        if (InventoryWidget->WB_Primary)
            return InventoryWidget->WB_Primary->SlotWidgets;
        break;

    case E_SlotsType::Temp:
        if (InventoryWidget->WB_Temp)
            return InventoryWidget->WB_Temp->SlotWidgets;
        break;

    default:
        break;
    }

    return {};

}


UGridPanel* UInventoryHUDComponent::GetGribInventoryWidget(const E_SlotsType SlotType)
{
    if (!InventoryWidget) return nullptr;
    if (!InventoryWidget->WB_Primary) return nullptr;
    if (!InventoryWidget->WB_Temp) return nullptr;

    return (SlotType == E_SlotsType::Primary)
        ? InventoryWidget->WB_Primary->Grid
        : InventoryWidget->WB_Temp->Grid;

}

void UInventoryHUDComponent::InitializeSlots()
{
    InventorySlots.Empty();

    const int32 PrimarySize = GetInventorySize(E_SlotsType::Primary).X * GetInventorySize(E_SlotsType::Primary).Y;
    InventorySlots.Reserve(PrimarySize);

    for (int32 i = 0; i < PrimarySize; ++i)
    {
        FSlotStruct Slot;
        Slot.Index = i;
        InventorySlots.Add(Slot);
    }

    if (UseTempSlots)
    {
        const int32 TempSize = GetInventorySize(E_SlotsType::Temp).X * GetInventorySize(E_SlotsType::Temp).Y;
        TempInventorySlots.Reserve(TempSize);

        for (int32 i = 0; i < TempSize; ++i)
        {
            FSlotStruct Slot;
            Slot.Index = i;
            TempInventorySlots.Add(Slot);
        }
    }
    else
    {
        TempInventorySlots.Empty();

    }

}

FIntPoint UInventoryHUDComponent::GetInventorySize(const E_SlotsType SlotsType)
{
    UInventoryData* Config = UInventoryStaticFunctions::GetInventoryOptions(this); //(Width,Height)
    if (Config)
    {
        if (SlotsType == E_SlotsType::Primary)
        {
            return FIntPoint(Config->PrimaryConfig.Columns, Config->PrimaryConfig.Rows);
        }
        if (SlotsType == E_SlotsType::Temp)
        {
            return FIntPoint(Config->TempConfig.Columns, Config->TempConfig.Rows);
        }
    }
    return FIntPoint(0, 0);
}

TArray<FSlotStruct>& UInventoryHUDComponent::GetSlots(const E_SlotsType Type)
{
    if (Type == E_SlotsType::Primary)
        return InventorySlots;
    if (Type == E_SlotsType::Temp)
        return TempInventorySlots;
    if (Type == E_SlotsType::HiddenSlots)
        return HiddenSlots;
    return InventorySlots;
}

bool UInventoryHUDComponent::CheckSlots(const int32 Index, const int32 Width, const int32 Height, TArray<int32>& EmptySlots, const E_SlotsType SlotType, const TArray<int32>& ExcludeSlots)
{
    EmptySlots.Empty();
    for (int i = 0; i < Height; ++i)
    {
        const int32 TempIndex = Index + (i == 0 ? 0 : GetInventorySize(SlotType).X * i);
        for (int x = TempIndex; x < TempIndex + Width; ++x)
        {
            if (GetSlots(SlotType).IsValidIndex(x) && (GetSlots(SlotType)[x].IsEmpty || ExcludeSlots.Contains(x)))
            {
                EmptySlots.Add(x);
            }
            else
            {
                EmptySlots.Empty();
                return false;
            }

        }
    }
    return true;
}

bool UInventoryHUDComponent::CheckSize(const int32 Index, const int32 Width, const int32 Height, const E_SlotsType SlotsType)
{
    if (UInventoryStaticFunctions::GetRow(Index, GetInventorySize(SlotsType).X) ==
        UInventoryStaticFunctions::GetRow(Index + Width - 1,
            GetInventorySize(SlotsType).X) 
            &&
        UInventoryStaticFunctions::GetColumn(Index, GetInventorySize(SlotsType).X) ==
        UInventoryStaticFunctions::GetColumn(
            Index + (GetInventorySize(SlotsType).X * (Height - 1)),
            GetInventorySize(SlotsType).X))
    {
        return true;
    }
    return false;
}

void UInventoryHUDComponent::FillSlots(AInspectableItem* Item, const TArray<int32>& Slots, const E_SlotsType SlotsType)
{
    if (!IsValid(Item) || Slots.Num() == 0) return;
    Item->InventoryItemPayload.SlotsType = SlotsType;
    Item->InventoryItemPayload.OccupiedSlots = Slots;

    TArray<FSlotStruct>& SlotArray = GetSlots(SlotsType);

    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        const int32 SlotIndex = Slots[i];
        if (!SlotArray.IsValidIndex(SlotIndex)) continue;

        FSlotStruct& Slot = SlotArray[SlotIndex];
        Slot.IsEmpty = false;
        Slot.ItemReference = Item;
        Slot.IsPartOfItem = (i != 0);
    }

}

void UInventoryHUDComponent::FillItemToSlots(AInspectableItem* Item, const int32 Amount, const EItemRotation Rotation, const TArray<int32>& Slots, TArray<FSlotStruct>& SlotsStruct)
{
    if (!Item) return;
    AInspectableItem* NewItem = NewObject<AInspectableItem>(this, Item->GetClass());
    if (!NewItem) return;

    NewItem->InventoryItemPayload = Item->InventoryItemPayload;
    NewItem->InventoryItemPayload.ItemAmount = Amount;
    NewItem->InventoryItemPayload.Rotation = Rotation;
    NewItem->InventoryItemPayload.OccupiedSlots = Slots;
    NewItem->InventoryItemPayload.InventoryComponentReference = this;

    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        const int32 SlotIndex = Slots[i];
        if (!SlotsStruct.IsValidIndex(SlotIndex)) continue;

        FSlotStruct& Slot = SlotsStruct[SlotIndex];
        Slot.IsEmpty = false;
        Slot.ItemReference = NewItem;
        Slot.IsPartOfItem = (i != 0);
    }

}

bool UInventoryHUDComponent::FindEmptySlots(const E_SlotsType SlotsType, const int32 Width, const int32 Height, EItemRotation& Rotation, TArray<int32>& EmptySlots, const TArray<int32>& ExcludeSlots)
{
    const TArray<FSlotStruct>& Slots = GetSlots(SlotsType);
    for (const FSlotStruct& InventorySlot : Slots)
    {
        if (InventorySlot.IsEmpty || ExcludeSlots.Contains(InventorySlot.Index))
        {
            // Horizontal
            if (CheckSize(InventorySlot.Index, Width, Height, SlotsType) &&
                CheckSlots(InventorySlot.Index, Width, Height, EmptySlots, SlotsType, ExcludeSlots))
            {
                Rotation = EItemRotation::Horizontal;
                return true;
            }
            //Vertical
            if (CheckSize(InventorySlot.Index, Height, Width, SlotsType) &&
                CheckSlots(InventorySlot.Index, Height,
                    Width, EmptySlots, SlotsType, ExcludeSlots))
            {
                Rotation = EItemRotation::Vertical;
                return true;
            }
        }
    }

    return false;
}

bool UInventoryHUDComponent::CanAddItem(AInspectableItem* Item, const E_SlotsType SlotsType, int32 ItemAmount)
{
    if (!Item) return 0;
    if (ItemAmount <= 0) return 0;
    const int32 MaxStack = Item->InventoryItemPayload.MaxStack;
    TArray<FSlotStruct> Slots = GetSlots(SlotsType);

    TArray<int32> EmptySlots;
    EItemRotation Rotation = EItemRotation::Horizontal;
   
    int32 CanAdd;
    AInspectableItem* ItemRef = nullptr;
    if (Item->InventoryItemPayload.IsStackable)
    {
        while (FindItemToStack(ItemRef, Slots, CanAdd,  true, MaxStack, Item->InventoryItemPayload.ItemName))
        {
            return true;
        }
    }

    while (FindEmptySlots(SlotsType, Item->InventoryItemPayload.ItemIconSize.X, Item->InventoryItemPayload.ItemIconSize.Y, Rotation, EmptySlots, TArray<int32>()))
    {
        return true;
    }

    return false;
}

void UInventoryHUDComponent::AddItem_HelperFunction(AInspectableItem* Item, const EItemRotation Rotation, const TArray<int32>& Slots, const E_SlotsType SlotsType, const int32 Amount)
{
    if (!Item) return;
    AInspectableItem* NewItem = NewObject<AInspectableItem>(this,Item->GetClass());
    if (!NewItem) return;
    NewItem->InventoryItemPayload = Item->InventoryItemPayload;
    NewItem->InventoryItemPayload.ItemAmount = Amount;
    NewItem->InventoryItemPayload.Rotation = Rotation;
    NewItem->InventoryItemPayload.OccupiedSlots = Slots;
    NewItem->InventoryItemPayload.InventoryComponentReference = this;

    FillSlots(NewItem, Slots, SlotsType);

}

void UInventoryHUDComponent::AddItemToSlots(AInspectableItem* Item , int32 ItemAmount, const E_SlotsType SlotType)
{
    if (!IsValid(Item)) return;
    if (ItemAmount <= 0) return;

    const int32 Width = Item->InventoryItemPayload.ItemIconSize.X;
    const int32 Height = Item->InventoryItemPayload.ItemIconSize.Y;
    const int32 MaxStack = Item->InventoryItemPayload.MaxStack;

    TArray<int32> EmptySlots;
    EItemRotation Rotation = EItemRotation::Horizontal;

    //如果是堆叠的物品，先找到可目前背包可堆叠的物品，逐个查看数量进行添加
    if (Item->InventoryItemPayload.IsStackable)
    {
        int32 CanAdd;
        AInspectableItem* ItemRef;

        TArray<FSlotStruct> Slots = GetSlots(SlotType);

        while (FindItemToStack(ItemRef, Slots, CanAdd, true, MaxStack, Item->InventoryItemPayload.ItemName))
        {
            if (CanAdd >= ItemAmount)
            {
                SetItemAmount(ItemRef, ItemRef->InventoryItemPayload.ItemAmount + ItemAmount);
                return;
            }
            SetItemAmount(ItemRef, ItemRef->InventoryItemPayload.ItemAmount + CanAdd);
            ItemAmount -= CanAdd;

            if (ItemAmount <= 0) return; //保险
        }
    }


    while (ItemAmount > 0 && FindEmptySlots(SlotType, Width, Height, Rotation, EmptySlots, TArray<int32>()))
    {
        //不堆叠
        if (!Item->InventoryItemPayload.IsStackable)
        {
            AddItem_HelperFunction(Item, Rotation, EmptySlots, SlotType, 1);
            ItemAmount--;
        }
        else
        {
            if (ItemAmount <= MaxStack)
            {
                AddItem_HelperFunction(Item, Rotation, EmptySlots, SlotType, ItemAmount);
                return;
            }
            AddItem_HelperFunction(Item, Rotation, EmptySlots, SlotType, MaxStack);
            ItemAmount -= MaxStack;
        }
       
    }

    Item->InventoryItemPayload.CanDestory ? Item->InventoryItemPayload.ItemAmount = ItemAmount : 1;
    return;

}

bool UInventoryHUDComponent::FindItemToStack(AInspectableItem*& Item, const TArray<FSlotStruct>& Slots, int32& CanAdd, bool IsStackable, const int32 MaxStack, const FText ItemName)
{
    if (IsStackable)
    {
        for (const FSlotStruct& Slot : Slots)
        {
            if (!Slot.IsEmpty && !Slot.IsPartOfItem && IsValid(Slot.ItemReference) && Slot.ItemReference->InventoryItemPayload.ItemName.EqualTo(ItemName))
            {
                if (Slot.ItemReference->InventoryItemPayload.ItemAmount < MaxStack)
                {
                    CanAdd = MaxStack - Slot.ItemReference->InventoryItemPayload.ItemAmount;
                    Item = Slot.ItemReference;
                    return true;
                }
            }
        }
    }

    return false;
}

void UInventoryHUDComponent::SetItemAmount(AInspectableItem* Item, const int32 Amount)
{
    if (IsValid(Item))
    {
        if (Amount <= 0) RemoveItemByRef(Item, 1, true);
        else Item->InventoryItemPayload.ItemAmount = Amount;

        if (Item->InventoryItemPayload.OccupiedSlots.IsValidIndex(0))
        {
            const int32 Index = Item->InventoryItemPayload.OccupiedSlots.IsValidIndex(0);
            const E_SlotsType SlotsType = Item->InventoryItemPayload.SlotsType;

            UItemWidget* ItemWidget = GetItemWidgetByIndex(Index, SlotsType);
            if (!ItemWidget) return;
            // 设置数量
            ItemWidget->InventoryItemPayload.ItemAmount = Amount;
            ItemWidget->SetAmount();
        }
    }
}

void UInventoryHUDComponent::AddItemWidgetToGrib(AInspectableItem* Item, const bool CanDestory, const bool IsEquipped)
{
    if (!ItemWidgetClass || !IsValid(Item)) return;
    if (Item->InventoryItemPayload.OccupiedSlots.Num() == 0) return;

    UInventoryData* InvData = UInventoryStaticFunctions::GetInventoryOptions(this);
    if (!InvData) return;

    const int32 Width = InvData->PrimaryConfig.Columns;
    const int32 FirstSlot = Item->InventoryItemPayload.OccupiedSlots[0];

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
    if (!PC) return;

    UItemWidget* NewItemWidget = CreateWidget<UItemWidget>(PC, ItemWidgetClass);
    if (!NewItemWidget) return;

    NewItemWidget->InventoryItemPayload = Item->InventoryItemPayload;
    NewItemWidget->InventoryItemPayload.InventoryComponentReference = this;
    NewItemWidget->InventoryItemPayload.IsEquipped = IsEquipped;
    NewItemWidget->DefaultRotation = Item->InventoryItemPayload.Rotation;
    NewItemWidget->SetItemWidgetAspectRatio();

    ItemsWidgets.Add(NewItemWidget);

    UGridPanel* Grid = GetGribInventoryWidget(Item->InventoryItemPayload.SlotsType);
    if (!Grid) return;

    const int32 Col = UInventoryStaticFunctions::GetColumn(FirstSlot, Width);
    const int32 Row = UInventoryStaticFunctions::GetRow(FirstSlot, Width);

    UGridSlot* GridSlot = Grid->AddChildToGrid(NewItemWidget, Row, Col);
    if (!GridSlot) return;

    GridSlot->SetLayer(2);

    const bool bIsVertical = (Item->InventoryItemPayload.Rotation == EItemRotation::Vertical);
    const int32 RowSpan = bIsVertical ? Item->InventoryItemPayload.ItemIconSize.X : Item->InventoryItemPayload.ItemIconSize.Y;
    const int32 ColSpan = bIsVertical ? Item->InventoryItemPayload.ItemIconSize.Y : Item->InventoryItemPayload.ItemIconSize.X;

    GridSlot->SetRowSpan(RowSpan);
    GridSlot->SetColumnSpan(ColSpan);
}

void UInventoryHUDComponent::ClearSlots(const E_SlotsType SlotsType, const TArray<int32>& Slots)
{
    for (const int32& SlotIndex : Slots)
    {
        if (GetSlots(SlotsType).IsValidIndex(SlotIndex))
        {

            GetSlots(SlotsType)[SlotIndex].IsEmpty = true;
            GetSlots(SlotsType)[SlotIndex].IsPartOfItem = false;
            GetSlots(SlotsType)[SlotIndex].ItemReference = nullptr;

        }
    }
}

void UInventoryHUDComponent::RemoveItemsInSlot(const int32 SlotIndex, const E_SlotsType SlotType, const int32 Amount, const bool RemoveAll)
{
    if (Amount <= 0) return;
    AInspectableItem* Item;
    if (GetItemInSlot(SlotIndex, SlotType, Item))
    {
        if (RemoveAll)
        {
            ClearSlots(SlotType, Item->InventoryItemPayload.OccupiedSlots);
            return;
        }
        const int32 TempAmount = Item->InventoryItemPayload.ItemAmount - Amount;
        if (TempAmount <= 0)
        {
            //has other
            ClearSlots(SlotType, Item->InventoryItemPayload.OccupiedSlots);
            return; // true;
        }
        SetItemAmount(Item, TempAmount);
    }
    return;
}

void UInventoryHUDComponent::RemoveItemByRef(const AInspectableItem* Item, const int32 Amount, const bool RemoveAll)
{
    if (!IsValid(Item)) return;
    if (!Item->InventoryItemPayload.OccupiedSlots.IsValidIndex(0)) return;
    RemoveItemsInSlot(Item->InventoryItemPayload.OccupiedSlots[0], Item->InventoryItemPayload.SlotsType, Amount, RemoveAll);
}

UItemWidget* UInventoryHUDComponent::GetItemWidgetByIndex(const int32 Index, const E_SlotsType SlotType)
{
    for (UItemWidget* ItemWidget : ItemsWidgets)
    {
        if (!ItemWidget) continue;

        if (ItemWidget->InventoryItemPayload.OccupiedSlots.Contains(Index) && ItemWidget->InventoryItemPayload.SlotsType == SlotType)
        {
            return ItemWidget;
        }
    }

    return nullptr;
}

bool UInventoryHUDComponent::GetItemInSlot(const int32 SlotIndex, const E_SlotsType SlotType, AInspectableItem*& Item)
{
    if (GetSlots(SlotType).IsValidIndex(SlotIndex) && IsValid(GetSlots(SlotType)[SlotIndex].ItemReference))
    {
        Item = GetSlots(SlotType)[SlotIndex].ItemReference;
        return true;
    }
    Item = nullptr;
    return false;
}

void UInventoryHUDComponent::ClearAllItemWidgets()
{
    for (UItemWidget* ItemWidget : ItemsWidgets)
    {
        ItemWidget->RemoveFromParent();
    }
    ItemsWidgets.Empty();
}

void UInventoryHUDComponent::LoadItemWidgets()
{

    ClearAllItemWidgets();

    for (FSlotStruct& Slot : GetSlots(E_SlotsType::Primary))
    {
        if (Slot.IsEmpty || !Slot.ItemReference || Slot.IsPartOfItem) continue;

        AddItemWidgetToGrib(Slot.ItemReference,false,false);
    }
}












