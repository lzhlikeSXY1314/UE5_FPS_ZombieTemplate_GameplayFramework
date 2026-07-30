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
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include <Blueprint/WidgetLayoutLibrary.h>
#include <Blueprint/SlateBlueprintLibrary.h>

void UInventoryHUDComponent::BeginPlay()
{
    Super::BeginPlay();

    InitializeSlots();

    OnDragDetected.AddDynamic(this, &ThisClass::HandleDragDetected);
    OnChangingAdditionalSlots.AddDynamic(this, &ThisClass::OnAdditionSlotsChanged);
}

void UInventoryHUDComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!DragWidget) return;
    SnapDraggedItemToGridSlot(DeltaTime);
}

// Sets default values for this component's properties
UInventoryHUDComponent::UInventoryHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.bTickEvenWhenPaused = true;

}

void UInventoryHUDComponent::OpenInventory()
{
    if (!InventoryWidgetClass) return;

    APlayerController* PC = GetPlayerController();
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
        DeselectAllSlotsByType(E_SlotsType::Primary);
        DeselectAllSlotsByType(E_SlotsType::Temp);
        LoadItemWidgets();
    }

    // 将设置鼠标位置延迟到下一帧
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            if (APlayerController* PC = GetPlayerController())
            {
                int32 ViewportX, ViewportY;
                PC->GetViewportSize(ViewportX, ViewportY);
                PC->SetMouseLocation(ViewportX / 2, ViewportY / 2);
            }
        });

}

void UInventoryHUDComponent::CloseInventory()
{
    if (DragWidget) DragWidget->RemoveFromParent();
    DragWidget = nullptr;

    if(HidedItemWidgetWhenDragActive) HidedItemWidgetWhenDragActive->RemoveFromParent();
    HidedItemWidgetWhenDragActive = nullptr;

    LoadPrimarySlotsFromArray();
    PlayInventorySound(E_InventorySoundType::Close,false);
}

void UInventoryHUDComponent::OnMouseButtonDown()
{
    if (DragWidget)
    {
        RotateItemWidget(true);
        return;
    }

    if (InventoryWidget)
    {
        InventoryWidget->CloseInventory();
        CloseInventory();
        return;
    }
}

void UInventoryHUDComponent::OnMouseButtonUp()
{
    if (DragWidget) MoveItem();
}

void UInventoryHUDComponent::SelectSlot(int32 InIndex, E_SlotsType InSlotType, bool InShouldPlaySound)
{
    if (DragWidget)
    {
        const int32 InvCol = GetInventorySize(GetRealSelectedSlot().Type).X;
        const int32 InvRow = GetInventorySize(GetRealSelectedSlot().Type).Y;
        const FIntPoint ItemSize = DragWidget->InventoryItemPayload.ItemIconSize;

        if ((ItemSize.X > InvCol|| ItemSize.Y > InvRow) && DragWidget->Rotation == EItemRotation::Horizontal) return;
        if ((ItemSize.Y > InvCol || ItemSize.X > InvRow) && DragWidget->Rotation == EItemRotation::Vertical) return;
    }

    int32 L_SelectSlotIndex = FMath::Max(InIndex, 0);
    

    SelectSlotType = InSlotType;

    ApplyOffset(L_SelectSlotIndex); // 查空间，跟随拾取起点

    if (DragWidget) InShouldPlaySound &= (L_SelectSlotIndex != SelectSlotIndex || SelectSlotType != InSlotType);

    SelectSlotIndex = FMath::Max(L_SelectSlotIndex, 0);


    if (DragWidget)
    {
        InShouldPlaySound ? PlayInventorySound(E_InventorySoundType::SelectSlotWhenItemDragged) : PlayInventorySound(E_InventorySoundType::None);
        return;
    }


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

void UInventoryHUDComponent::DeselectAllSlotsByType(E_SlotsType InSlotType)
{
    for (UInventorySlotWidget* Slot : GetSlotsByType(InSlotType))
    {
        if (!Slot) continue;
        Slot->SelectSlot(false);
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


APlayerController* UInventoryHUDComponent::GetPlayerController()
{
    if (ACharacter* Char = GetOwner<ACharacter>())
    {
        if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
        {
            return PC;
        }
    }
    
    return nullptr;
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

void UInventoryHUDComponent::SetRealSelectedSlot(int32 InIndex, E_SlotsType InSlotType)
{
    RealSelectedSlot.Index = InIndex;
    RealSelectedSlot.Type = InSlotType;
}

void UInventoryHUDComponent::InitializeSlots()
{
    InventorySlots.Empty();
    TempInventorySlots.Empty();

    const int32 PrimarySize = GetInventorySize(E_SlotsType::Primary).X * GetInventorySize(E_SlotsType::Primary).Y;
    InventorySlots.Reserve(PrimarySize);

    for (int32 i = 0; i < PrimarySize; ++i)
    {
        FSlotStruct Slot;
        Slot.Index = i;
        InventorySlots.Add(Slot);
    }

    bool UseTempSlots = UInventoryStaticFunctions::GetInventoryOptions(this)->bEnableTempSlots;

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

    //Initialize Hidden Slot
    FSlotStruct SlotStruct;
    SlotStruct.Index = 0;
    SlotStruct.IsEmpty = true;
    SlotStruct.IsPartOfItem = false;
    SlotStruct.ItemReference = nullptr;
    HiddenItemSlotsType = E_SlotsType::Primary;
    HiddenSlots.Add(SlotStruct);

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
    OnChangingAdditionalSlots.Broadcast();
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

    if (Slots.IsValidIndex(0))
    {
        UItemWidget* ItemWidget = GetItemWidgetByIndex(Slots[0], SlotsType);

        if (ItemWidget)
        {
            ItemsWidgets.Remove(ItemWidget);
            ItemWidget->RemoveFromParent();
               //SelectSlot
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

        AddItemWidgetToGrib(Slot.ItemReference, E_SlotsType::Primary);
    }

}

int32 UInventoryHUDComponent::GetRowBySlotType(int32 InIndex, E_SlotsType InSlotType)
{
    if (InIndex < 0) return -1;
    if(!UInventoryStaticFunctions::GetInventoryOptions(this))  return -1;
    int32 Col = GetInventorySize(InSlotType).X;
    
    return UInventoryStaticFunctions::GetRow(InIndex, Col);
}

int32 UInventoryHUDComponent::GetColumnBySlotType(int32 InIndex, E_SlotsType InSlotType)
{
    if (InIndex < 0) return -1;
    if (!UInventoryStaticFunctions::GetInventoryOptions(this))  return -1;
    int32 Col = GetInventorySize(InSlotType).X;
    return UInventoryStaticFunctions::GetColumn(InIndex,Col);
}

UInventorySlotWidget* UInventoryHUDComponent::GetSlotWidgetByIndex(int32 InIndex, E_SlotsType InSlotType)
{
    TArray<UInventorySlotWidget*> Slots = GetSlotsByType(InSlotType);
    for (UInventorySlotWidget* Slot : Slots)
    {
        if (Slot && Slot->Index == InIndex)
            return Slot;
    }
    return nullptr;
}

void UInventoryHUDComponent::AddItemWidgetToGrib(AInspectableItem* Item, E_SlotsType SlotType)
{
    if (!ItemWidgetClass || !IsValid(Item)) return;
    if (Item->InventoryItemPayload.OccupiedSlots.Num() == 0) return;

    UInventoryData* InvData = UInventoryStaticFunctions::GetInventoryOptions(this);
    if (!InvData) return;

    const int32 Width = GetInventorySize(SlotType).X;
    const int32 FirstSlot = Item->InventoryItemPayload.OccupiedSlots[0];

    APlayerController* PC = GetPlayerController();
    if (!PC) return;

    UItemWidget* NewItemWidget = CreateWidget<UItemWidget>(PC, ItemWidgetClass);
    if (!NewItemWidget) return;

    NewItemWidget->InventoryItemPayload = Item->InventoryItemPayload;
    NewItemWidget->InventoryItemPayload.InventoryComponentReference = this;
    NewItemWidget->DefaultRotation = Item->InventoryItemPayload.Rotation;
    NewItemWidget->Rotation = Item->InventoryItemPayload.Rotation;
    NewItemWidget->SetItemWidgetAspectRatio();

    ItemsWidgets.Add(NewItemWidget);

    UGridPanel* Grid = GetGribInventoryWidget(SlotType);
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

/// <DragWidget>
void UInventoryHUDComponent::InitializeDragWidget()
{
    FSlotInfo SlotInfo = GetRealSelectedSlot();
    UItemWidget* ItemWidget = GetItemWidgetByIndex(SlotInfo.Index, SlotInfo.Type);
    if (!ItemWidget) return;
    EndBackItemWidgetToSlots();
    HidedItemWidgetWhenDragActive = ItemWidget;
    APlayerController* PC = GetPlayerController();
    if (!PC) return;

    FVector2D MousePos = FVector2D::ZeroVector;
    PC->GetMousePosition(MousePos.X, MousePos.Y);
    const FVector2D WidgetViewportPos = ItemWidget->GetCachedGeometry().GetAbsolutePosition();
    Offset = MousePos - WidgetViewportPos;

    if (DragWidget)
    {
        DragWidget->RemoveFromParent();
        DragWidget = nullptr;
    }
    DeselectAllItemWidget();
    HidedItemWidgetWhenDragActive->SetVisibility(ESlateVisibility::Collapsed);

    int32 FirstOccupiedSlotIndex = HidedItemWidgetWhenDragActive->GetFirstOccupiedSlotIndex();
    E_SlotsType SlotType = HidedItemWidgetWhenDragActive->InventoryItemPayload.SlotsType;
    if (GetSlots(SlotType).IsValidIndex(FirstOccupiedSlotIndex))
    {
        AInspectableItem* ItemRef = GetSlots(SlotType)[FirstOccupiedSlotIndex].ItemReference;
        if (!ItemRef) return;
        UItemWidget* NewItemWidget = CreateWidget<UItemWidget>(PC, ItemWidgetClass);
        if (!NewItemWidget) return;
        NewItemWidget->InventoryItemPayload = ItemWidget->InventoryItemPayload;
        NewItemWidget->DefaultRotation = ItemWidget->InventoryItemPayload.Rotation;
        NewItemWidget->Rotation = ItemWidget->InventoryItemPayload.Rotation;

        //获取从被点击的单元格到槽位项第一个单元格的偏移量
        int32 RealSelectedIndex = SlotInfo.Index;
        int32 Offsetcol = GetColumnBySlotType(RealSelectedIndex, SlotType) - GetColumnBySlotType(FirstOccupiedSlotIndex, SlotType);
        int32 OffsetRow = GetRowBySlotType(RealSelectedIndex, SlotType) - GetRowBySlotType(FirstOccupiedSlotIndex, SlotType);
        NewItemWidget->Offset = FVector2D(Offsetcol, OffsetRow);

        NewItemWidget->SetItemWidgetAspectRatio();
        if (InventoryWidget && InventoryWidget->CanvasPanel_Root)
        {
            UCanvasPanelSlot* CanvasPanelSlot = InventoryWidget->CanvasPanel_Root->AddChildToCanvas(NewItemWidget);
            CanvasPanelSlot->SetAutoSize(true);

            UInventorySlotWidget* Slot = GetSlotWidgetByIndex(FirstOccupiedSlotIndex, SlotInfo.Type); //很奇怪用数组寻求的值会出错
            if (!Slot) return;

           FGeometry CachedGeometry = Slot->GetCachedGeometry();
           FVector2D ViewportPos;
           FVector2D PixelPosition;

        
            USlateBlueprintLibrary::LocalToViewport(
               this,
               CachedGeometry,
               FVector2D::ZeroVector,
               PixelPosition,
               ViewportPos
            );
            UWidgetLayoutLibrary::SlotAsCanvasSlot(NewItemWidget)->SetPosition(ViewportPos);


            NewItemWidget->SetActiveBackgroundMaterial(true);
            DragWidget = NewItemWidget;
        }

    }

}

void UInventoryHUDComponent::SnapDraggedItemToGridSlot(float InDeltaTime)
{
    if (!DragWidget) return;
    UInventorySlotWidget* Slot = GetSlotWidgetByIndex(SelectSlotIndex, SelectSlotType);

    if (!Slot) return;
    UCanvasPanelSlot* DragWidgetSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(DragWidget);
    if (!DragWidgetSlot) return;

    FGeometry CachedGeometry = Slot->GetCachedGeometry();
    FVector2D ViewportPos;
    FVector2D PixelPosition;
    USlateBlueprintLibrary::LocalToViewport(
        this,
        CachedGeometry,
        FVector2D::ZeroVector,
        PixelPosition,
        ViewportPos
    );
    FVector2D CurrentPos = DragWidgetSlot->GetPosition();
    float InterpSpeed = 22.0f;
    FVector2D NewPos = FMath::Vector2DInterpTo(CurrentPos, ViewportPos, InDeltaTime, InterpSpeed);

    DragWidgetSlot->SetPosition(NewPos);
}

void UInventoryHUDComponent::ApplyOffset(int32& InIndex)
{
    if (!DragWidget) return;

    E_SlotsType L_SlotsType = GetRealSelectedSlot().Type;
    FVector2D L_Offset = DragWidget->Offset;

    const int32 Col = GetInventorySize(L_SlotsType).X;
    const int32 Row = GetInventorySize(L_SlotsType).Y;

    const int32 ItemSizeCol = DragWidget->InventoryItemPayload.ItemIconSize.X;
    const int32 ItemSizeRow = DragWidget->InventoryItemPayload.ItemIconSize.Y;

    bool IsRotated = DragWidget->Rotation != DragWidget->DefaultRotation;
    bool DefaultIsVertical = DragWidget->DefaultRotation == EItemRotation::Vertical;
    bool IsLongVertical = DragWidget->InventoryItemPayload.ItemIconSize.X < DragWidget->InventoryItemPayload.ItemIconSize.Y;

    InIndex = CalculateItemIndex(InIndex, L_Offset, Col, Row, ItemSizeRow, ItemSizeCol, IsRotated, DefaultIsVertical, IsLongVertical);

}

int32 UInventoryHUDComponent::CalculateItemIndex(int32 ClickIndex, FVector2D InOffset, int32 InventoryCols, int32 InventoryRows, int32 ItemRows, int32 ItemCols, bool bIsRotated, bool DefaultIsVertical, bool IsLongVerticalItem)
{
    // 防御性检查：避免除零错误（库存行列数至少为1）
    if (InventoryCols <= 0 || InventoryRows <= 0)
    {
        return 0;
    }

    int32 DefaultItemCol = 0;
    int32 DefaultItemRow = 0;
    int32 RelativeRow = 0;
    int32 RelativeCol = 0;

    //根据IsLongVerticalItem是否为垂直长物进行旋转
    if (IsLongVerticalItem)
    {


        if (DefaultIsVertical)
        {
            if (ItemCols < ItemRows)
            {
                DefaultItemCol = ItemRows;
                DefaultItemRow = ItemCols;
            }
            else
            {
                DefaultItemCol = ItemCols;
                DefaultItemRow = ItemRows;
            }

            RelativeCol = bIsRotated ? InOffset.Y : InOffset.X; // 点击点相对物品左上角的行偏移
            RelativeRow = bIsRotated ? DefaultItemCol - InOffset.X - 1 : InOffset.Y; // 点击点相对物品左上角的列偏移

        }
        else
        {
            //下面是正常的：
            DefaultItemCol = ItemCols;
            DefaultItemRow = ItemRows;

            RelativeRow = bIsRotated ? InOffset.X : InOffset.Y; // 点击点相对物品左上角的行偏移
            RelativeCol = bIsRotated ? ItemRows - InOffset.Y - 1 : InOffset.X; // 点击点相对物品左上角的列偏移



        }



    }
    else
    {

        if (DefaultIsVertical)
        {
            if (ItemCols > ItemRows)
            {
                DefaultItemCol = ItemRows;
                DefaultItemRow = ItemCols;
            }
            else
            {
                DefaultItemCol = ItemCols;
                DefaultItemRow = ItemRows;
            }

            RelativeCol = bIsRotated ? InOffset.Y : InOffset.X; // 点击点相对物品左上角的行偏移
            RelativeRow = bIsRotated ? DefaultItemCol - InOffset.X - 1 : InOffset.Y; // 点击点相对物品左上角的列偏移

        }
        else
        {
            DefaultItemCol = ItemCols;
            DefaultItemRow = ItemRows;

            RelativeRow = bIsRotated ? InOffset.X : InOffset.Y; // 点击点相对物品左上角的行偏移
            RelativeCol = bIsRotated ? ItemRows - InOffset.Y - 1 : InOffset.X; // 点击点相对物品左上角的列偏移
        }

    }

    // 1. 处理旋转：交换物品的行列偏移和实际尺寸

    const int32 ItemWidth = bIsRotated ? DefaultItemRow : DefaultItemCol;   // 物品实际宽度（列数）
    const int32 ItemHeight = bIsRotated ? DefaultItemCol : DefaultItemRow;  // 物品实际高度（行数）

    // 2. 提取点击点的行和列（基础坐标）
    const int32 ClickRow = ClickIndex / InventoryCols;    // 点击点所在行
    const int32 ClickCol = ClickIndex % InventoryCols;    // 点击点所在列

    // 3. 计算初始左上角索引（基于点击点和相对偏移）
    // 3.1 计算点击点上方、相对行范围内的总格子数（用于行内位置校准）
    const int32 CellsAboveRelativeRow = RelativeRow * InventoryCols;
    // 3.2 计算点击点在当前行内的列位置（考虑相对行上方的偏移）
    int32 LocalColInRow = ClickIndex - CellsAboveRelativeRow;

    // 3.3 处理列位置为负数的情况（点击点在相对行上方时校准）
    if (LocalColInRow < 0)
    {
        // 校准公式：补充相对行与点击行的差值对应的列偏移
        LocalColInRow += (RelativeRow - ClickRow) * InventoryCols;
    }

    // 3.4 初始左上角索引 = 行内列位置 - 相对列偏移
    int32 ItemTopLeftIndex = LocalColInRow - RelativeCol;

    // 4. 处理左边界越界：点击列 <= 相对列时，强制左上角列对齐当前行的左边界
    if (ClickCol <= RelativeCol)
    {
        // 左上角行 = 点击行 - 相对行（确保至少在第0行）
        const int32 AdjustedTopRow = FMath::Max(ClickRow - RelativeRow, 0);
        ItemTopLeftIndex = AdjustedTopRow * InventoryCols; // 列强制为0（当前行第一列）
    }

    // 5. 转换左上角索引为行和列，准备处理下/右边界
    int32 TopRow = ItemTopLeftIndex / InventoryCols;
    int32 TopCol = ItemTopLeftIndex % InventoryCols;

    // 6. 计算库存边界上限
    const int32 MaxRow = InventoryRows - 1;       // 最大行索引（0-based）
    const int32 MaxCol = InventoryCols - 1;       // 最大列索引（0-based）
    const int32 MaxValidTopRow = FMath::Max(0, MaxRow - ItemHeight + 1); // 允许的最大左上角行（确保物品不超出下边界）
    const int32 MaxValidTopCol = FMath::Max(0, MaxCol - ItemWidth + 1);  // 允许的最大左上角列（确保物品不超出右边界）

    // 7. 处理下边界和上边界：直接 clamping 到合法范围（替代单独的if判断）
    TopRow = FMath::Clamp(TopRow, 0, MaxValidTopRow);

    // 8. 处理右边界和左边界：直接 clamping 到合法范围
    TopCol = FMath::Clamp(TopCol, 0, MaxValidTopCol);

    // 9. 计算最终的左上角索引
    return TopRow * InventoryCols + TopCol;
}

void UInventoryHUDComponent::EndBackItemWidgetToSlots()
{
    if (!IsValid(HidedItemWidgetWhenDragActive)) return;
    int32 FirstOccupiedSlotIndex = HidedItemWidgetWhenDragActive->GetFirstOccupiedSlotIndex();
    E_SlotsType SlotType = HidedItemWidgetWhenDragActive->InventoryItemPayload.SlotsType;

    if (GetSlots(SlotType).IsValidIndex(FirstOccupiedSlotIndex))
    {
        AInspectableItem* ItemRef = GetSlots(SlotType)[FirstOccupiedSlotIndex].ItemReference;
        if (ItemRef)
        {
            HidedItemWidgetWhenDragActive->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            HidedItemWidgetWhenDragActive->RemoveFromParent();
        }
        HidedItemWidgetWhenDragActive = nullptr;

        return;
    }

}

void UInventoryHUDComponent::RotateItemWidget(bool InPlaySound)
{
    if (!DragWidget) return;

    if (DragWidget->InventoryItemPayload.ItemIconSize.X == DragWidget->InventoryItemPayload.ItemIconSize.Y) return;


    const int32 InvCol = GetInventorySize(GetRealSelectedSlot().Type).X;
    const int32 InvRow = GetInventorySize(GetRealSelectedSlot().Type).Y;
    const FIntPoint ItemSize = DragWidget->InventoryItemPayload.ItemIconSize;
    if (ItemSize.X > InvRow || ItemSize.X > InvCol || ItemSize.Y > InvRow || ItemSize.Y > InvCol) return;



    if (InPlaySound) PlayInventorySound(E_InventorySoundType::Rotate, true);
    DragWidget->PlayRotationAnimation(DragWidget->Rotation == EItemRotation::Horizontal ? EItemRotation::Vertical : EItemRotation::Horizontal);

    SelectSlot(GetRealSelectedSlot().Index, GetRealSelectedSlot().Type,false);

}


void UInventoryHUDComponent::HandleDragDetected()
{
    if (IsValid(DragWidget))
    {
        MoveItem();
        return;
    }

    FSlotInfo SlotInfo = GetRealSelectedSlot();
    UItemWidget* ItemWidget = GetItemWidgetByIndex(SlotInfo.Index, SlotInfo.Type);
    if (!ItemWidget) return;

    PlayInventorySound(E_InventorySoundType::OnDragStart,true);
    InitializeDragWidget();
    DeselectAllSlotsByType(SelectSlotType);
    SelectSlot(GetRealSelectedSlot().Index, GetRealSelectedSlot().Type, false);

}


/// </DragWidget>

bool UInventoryHUDComponent::IsEmptySlotsForItem(const int32 Index, const int32 Width, const int32 Height, const E_SlotsType SlotType, const EItemRotation Rotation, TArray<int32>& EmptySlots, const TArray<int32>& ExcludeSlots)
{
    const int32 ItemWidth = Rotation == EItemRotation::Horizontal ? Width : Height;
    const int32 ItemHeight = Rotation == EItemRotation::Horizontal ? Height : Width;

    if (CheckSize(Index, ItemWidth, ItemHeight, SlotType) &&
        CheckSlots(Index, ItemWidth, ItemHeight, EmptySlots, SlotType, ExcludeSlots))
    {
        return true;
    }
    return false;

}

bool UInventoryHUDComponent::CanItemAddedToSlots(const int32 SlotOfItem, const E_SlotsType ItemSlotType, const EItemRotation Rotation, const int32 SlotToAdd, const E_SlotsType SlotsTypeToAdd, TArray<int32>& EmptySlots)
{
    //交换
    if (IsValidSwappedItem() && ItemSlotType != E_SlotsType::HiddenSlots) return false;
    AInspectableItem* ItemInSlot;
    if (IsValidSwappedItem() && GetItemInSlot(SlotToAdd, SlotsTypeToAdd, ItemInSlot))
    {
        if (GetSlots(E_SlotsType::HiddenSlots)[0].ItemReference == ItemInSlot) return false;
        if (ItemInSlot->InventoryItemPayload.SlotsType == ItemSlotType) return false;
    }


    FSlotStruct SlotInfo;
    if (GetSlots(ItemSlotType).IsValidIndex(SlotOfItem))
    {
        SlotInfo = GetSlots(ItemSlotType)[SlotOfItem];
    }

    if (!IsValid(SlotInfo.ItemReference)) return false;
    const FIntPoint ItemSize = SlotInfo.ItemReference->InventoryItemPayload.ItemIconSize;
    TArray<int32> ExcludeSlots = SlotInfo.ItemReference->InventoryItemPayload.OccupiedSlots;

    if (IsEmptySlotsForItem(SlotToAdd, ItemSize.X, ItemSize.Y, SlotsTypeToAdd, Rotation, EmptySlots,
        ItemSlotType == SlotsTypeToAdd ? ExcludeSlots : TArray<int32>()))
    {
        return true;
    }
    return false;
     
}

void UInventoryHUDComponent::MoveItem()
{
    if (!DragWidget) return;
    TArray<int32> EmptySlots;
    const int32 FirstOccupiedSlotIndex = DragWidget->GetFirstOccupiedSlotIndex();
    E_SlotsType SlotType = DragWidget->InventoryItemPayload.SlotsType;

    bool IsEmptySpace = CanItemAddedToSlots(FirstOccupiedSlotIndex, SlotType, DragWidget->Rotation, SelectSlotIndex, SelectSlotType, EmptySlots);
    if (IsEmptySpace)
    {
        PlayInventorySound(E_InventorySoundType::OnDragEnd,true);
        AInspectableItem* Item;
        if(!GetItemInSlot(FirstOccupiedSlotIndex, SlotType, Item)) return;
        if (!Item) return;

        if (SelectSlotType != E_SlotsType::Equipment) ClearSlots(Item->InventoryItemPayload.SlotsType, Item->InventoryItemPayload.OccupiedSlots);
        //HiddenItemSlotsType = E_SlotsType::Primary; 目前不知道干什么的

        AddExistingItemToSlots(Item, SelectSlotType, DragWidget->Rotation, EmptySlots);

        if (SlotType != SelectSlotType) OnChangingAdditionalSlots.Broadcast();


        DragWidget->RemoveFromParent();
        DragWidget = nullptr;
        
        //其他比图说controls hints widget;
 


        return;
    }

     AInspectableItem* ItemUnderDragItem = nullptr;

     const int32 L_FirstSlotsIndex = DragWidget->GetFirstOccupiedSlotIndex();
     const E_SlotsType L_SlotsType = DragWidget->InventoryItemPayload.SlotsType;
     const EItemRotation L_Rotation = DragWidget->Rotation;
     const FIntPoint L_ItemSize = DragWidget->InventoryItemPayload.ItemIconSize;
     FSlotInfo SelectedSlotInfo;
     SelectedSlotInfo.Index = SelectSlotIndex;
     SelectedSlotInfo.Type = SelectSlotType;

    if (CanSwapDraggedItem(L_FirstSlotsIndex, L_SlotsType, SelectSlotIndex, SelectSlotType, L_Rotation, ItemUnderDragItem))
    {

        CreateSwapItemWidget(ItemUnderDragItem);
        SwapDraggedItem(L_FirstSlotsIndex, L_SlotsType, SelectedSlotInfo.Index, SelectedSlotInfo.Type, L_Rotation, L_ItemSize);
        return;
    }
    //交换一个垂直东西会有问题 注意修改



}

void UInventoryHUDComponent::CreateSwapItemWidget(AInspectableItem* ItemUnderDragItem)
{
    if (!ItemUnderDragItem) return;
    if (!ItemUnderDragItem->InventoryItemPayload.OccupiedSlots.IsValidIndex(0)) return;
    const int32 UnderDragItemSlotFirstIndex = ItemUnderDragItem->InventoryItemPayload.OccupiedSlots[0];
    const E_SlotsType UnderDragItemSlotType = ItemUnderDragItem->InventoryItemPayload.SlotsType;

    //ItemWidget获取被交换的ItemWidget控件
    UItemWidget* ItemWidget = GetItemWidgetByIndex(UnderDragItemSlotFirstIndex, UnderDragItemSlotType);
    if (!ItemWidget) return;
    ItemWidget->SetVisibility(ESlateVisibility::Collapsed);
    PlayInventorySound(E_InventorySoundType::OnDragEnd, true);

    //InitializeDragWidgetForSwappedItem
    if (DragWidget)
    {
        DragWidget->RemoveFromParent();
        DragWidget = nullptr;
    }

    APlayerController* PC = GetPlayerController();
    UItemWidget* NewItemWidget = CreateWidget<UItemWidget>(PC, ItemWidgetClass); //新建的DragWidget就是被置换的东东
    NewItemWidget->InventoryItemPayload = ItemWidget->InventoryItemPayload;
    NewItemWidget->Rotation = ItemWidget->InventoryItemPayload.Rotation;
    NewItemWidget->DefaultRotation = ItemWidget->InventoryItemPayload.Rotation;
    NewItemWidget->InventoryItemPayload.SlotsType = E_SlotsType::HiddenSlots; //不可缺
    NewItemWidget->InventoryItemPayload.OccupiedSlots = { 0 };//不可缺

    if (!NewItemWidget) return;
    NewItemWidget->SetItemWidgetAspectRatio();
    if (!InventoryWidget || !InventoryWidget->CanvasPanel_Root) return;
    UCanvasPanelSlot* NewItemWidgetSlot = InventoryWidget->CanvasPanel_Root->AddChildToCanvas(NewItemWidget);
    NewItemWidgetSlot->SetAutoSize(true);

    if (!ItemWidget->InventoryItemPayload.OccupiedSlots.IsValidIndex(0)) return;
    SelectSlot(ItemWidget->InventoryItemPayload.OccupiedSlots[0], SelectSlotType, false);  //
    DeselectAllItemWidget();
    NewItemWidget->SetActiveBackgroundMaterial(true);

    UInventorySlotWidget* SlotWidget = GetSlotWidgetByIndex(UnderDragItemSlotFirstIndex, UnderDragItemSlotType); //被交换的ItemWidget第一个格子
    if (!SlotWidget) return;
    SlotWidget->SelectSlot(false);

    const FGeometry& CachedGeometry = SlotWidget->GetCachedGeometry();
    FVector2D ViewportPos;
    FVector2D PixelPosition;

    USlateBlueprintLibrary::LocalToViewport(
        this,
        CachedGeometry,
        FVector2D::ZeroVector,
        PixelPosition,
        ViewportPos
    );
    UWidgetLayoutLibrary::SlotAsCanvasSlot(NewItemWidget)->SetPosition(ViewportPos);

    DragWidget = NewItemWidget;

    //InitializeDragWidgetForSwappedItem Finish

}


void UInventoryHUDComponent::AddExistingItemToSlots(AInspectableItem* Item, const E_SlotsType SlotsType, const EItemRotation Rotation, const TArray<int32>& EmptySlots)
{
    if (!IsValid(Item)) return;
    Item->InventoryItemPayload.OccupiedSlots = EmptySlots;
    Item->InventoryItemPayload.Rotation = Rotation;
    Item->InventoryItemPayload.SlotsType = SlotsType;
    Item->InventoryItemPayload.InventoryComponentReference = this;

    FillSlots(Item, EmptySlots, SlotsType);
    OnChangingAdditionalSlots.Broadcast();
    AddItemWidgetToGrib(Item, SelectSlotType);


}

TArray<int32> UInventoryHUDComponent::GetSlotsByItemSize(const int32 FirstSlot, const E_SlotsType SlotsType, const FIntPoint ItemSize, const EItemRotation Rotation)
{
    TArray<int32> EmptySlots;
    const int32 Height = Rotation == EItemRotation::Horizontal ? ItemSize.Y : ItemSize.X;
    const int32 Width = Rotation == EItemRotation::Horizontal ? ItemSize.X : ItemSize.Y;

    if (!CheckSize(FirstSlot, Width, Height, SlotsType)) return EmptySlots;

    for (int i = 0; i < Height; ++i)
    {
        const int32 TempIndex = FirstSlot + (i == 0 ? 0 : GetInventorySize(SlotsType).X * i);

        for (int x = TempIndex; x < TempIndex + Width; ++x)
        {
            if (GetSlots(SlotsType).IsValidIndex(x))
            {
                EmptySlots.Add(x);
            }
        }
    }
    return EmptySlots;

}

TArray<AInspectableItem*> UInventoryHUDComponent::GetAllItemsInSlots(TArray<int32> Slots, E_SlotsType SlotsType)
{
    TArray<AInspectableItem*> ItemsInSlots;

    for (int32 const& Slot : Slots)
    {
        if (!GetSlots(SlotsType).IsValidIndex(Slot)) continue;
        if (GetSlots(SlotsType)[Slot].IsEmpty) continue;

        AInspectableItem* Item = GetSlots(SlotsType)[Slot].ItemReference;

        if (IsValid(Item) && (ItemsInSlots.Find(Item) == -1))
        {
            ItemsInSlots.Add(Item);
        }
    }
    return ItemsInSlots;
}

bool UInventoryHUDComponent::CanSwapDraggedItem(const int32 ItemToIgnoreSlotIndex, const E_SlotsType ItemToIgnoreSlotsType, const int32 SelectedIndex, const E_SlotsType SelectedSlotsType, const EItemRotation DraggedItemRotation, AInspectableItem*& OutTargetItem)
{
    if (!DragWidget) return false;

    const FIntPoint ItemSize = DragWidget->InventoryItemPayload.ItemIconSize;
    const TArray<int32> TargetSlots = GetSlotsByItemSize(SelectedIndex, SelectedSlotsType, ItemSize, DraggedItemRotation);
    if (TargetSlots.Num() != ItemSize.X * ItemSize.Y) return false;

    TArray<AInspectableItem*> Items = GetAllItemsInSlots(TargetSlots, SelectedSlotsType);

    // 如果目标区域包含被拖拽物品自己的格子，移除它自身
    if (SelectedSlotsType == ItemToIgnoreSlotsType)
    {
        AInspectableItem* SelfItem;
        if (GetItemInSlot(ItemToIgnoreSlotIndex, ItemToIgnoreSlotsType, SelfItem))
        {
            Items.Remove(SelfItem);
        }
    }

    // 必须恰好剩1个有效物品
    if (Items.Num() == 1 && IsValid(Items[0]))
    {
        OutTargetItem = Items[0];  // 直接赋值指针
        return true;
    }

    OutTargetItem = nullptr;
    return false;

}



void UInventoryHUDComponent::SwapDraggedItem(const int32 ItemIndex, const E_SlotsType SlotsType, const int32 SelectedIndex, const E_SlotsType SelectedSlotsType, const EItemRotation DraggedItemRotation, FIntPoint ItemSize)
{
    //传入 是先前拖拽数据
   
    AInspectableItem* DraggedItem = nullptr; //要置放的物品
    if (!GetItemInSlot(ItemIndex, SlotsType, DraggedItem) || !IsValid(DraggedItem)) return;

    TArray<int32> TargetSlots = GetSlotsByItemSize(SelectedIndex, SelectedSlotsType, ItemSize, DraggedItemRotation);

    if (SelectedSlotsType != E_SlotsType::Equipment)
    {
        const int32 ExpectedSlotCount = ItemSize.X * ItemSize.Y;
        if (TargetSlots.Num() != ExpectedSlotCount) return;

    }

    TArray<AInspectableItem*> ItemsInTargetSlots = GetAllItemsInSlots(TargetSlots, SelectedSlotsType);

    if (SelectedSlotsType != E_SlotsType::Equipment)
    {
        // 初始校验：目标槽位中必须有且仅有1个物品（未处理拖拽物品本身前）
        if (ItemsInTargetSlots.Num() != 1 || !ItemsInTargetSlots.IsValidIndex(0))
        {
            // 如果拖拽物品的原槽位类型和目标槽位类型相同，可能目标槽位包含自身，需要移除后再检查
            if (DraggedItem->InventoryItemPayload.SlotsType == SlotsType)
            {
                const int32 DraggedItemIndex = ItemsInTargetSlots.Find(DraggedItem);
                if (DraggedItemIndex == -1)
                {

                    return; // 没有找到自身
                }
                ItemsInTargetSlots.RemoveAt(DraggedItemIndex);  // 移除自身后重新检查数量
            }

            // 最终校验：处理后必须剩余1个物品
            if (ItemsInTargetSlots.Num() != 1)
            {

                return;
            }
        }
    }

    if (!ItemsInTargetSlots.IsValidIndex(0)) return;

    AInspectableItem* ItemToSwap = ItemsInTargetSlots[0]; //获得了被交换的物品
    if (!IsValid(ItemToSwap)) return;

    if (!HiddenSlots.IsValidIndex(0) || !GetSlots(E_SlotsType::HiddenSlots).IsValidIndex(0)) return;

    FSlotStruct HiddenSlot;
    HiddenSlot.IsEmpty = false;
    HiddenSlot.ItemReference = ItemToSwap;

    HiddenItemSlotsType = E_SlotsType::Primary; //有啥用？？

    ClearSlots(DraggedItem->InventoryItemPayload.SlotsType, DraggedItem->InventoryItemPayload.OccupiedSlots);  // 清空拖拽物品原来的槽位
    ClearSlots(SelectedSlotsType, ItemToSwap->InventoryItemPayload.OccupiedSlots);    // 清空被交换物品原来的槽位

    ItemToSwap->InventoryItemPayload.SlotsType = E_SlotsType::HiddenSlots;  // 标记为隐藏槽位物品
    ItemToSwap->InventoryItemPayload.OccupiedSlots = { 0 };                  // 隐藏槽位只有1个（索引0）
    HiddenSlots[0] = HiddenSlot;                      // 写入隐藏槽位数据

    AddExistingItemToSlots(DraggedItem, SelectedSlotsType, DraggedItemRotation, TargetSlots);

}

bool UInventoryHUDComponent::IsValidSwappedItem() const
{
    if (HiddenSlots.IsValidIndex(0) && HiddenSlots[0].ItemReference)
        return true;
    return false;
}

void UInventoryHUDComponent::OnAdditionSlotsChanged()
{
    if ((!IsSlotsHaveItems(E_SlotsType::Temp)))
    {
        if (!IsValidSwappedItem())	SavePrimarySlotsInArray();
    }
}

bool UInventoryHUDComponent::IsSlotsHaveItems(const E_SlotsType SlotsType)
{
    for (const FSlotStruct& Slot : GetSlots(SlotsType))
    {
        if (!Slot.IsEmpty) return true;
    }
    return false;
}

void UInventoryHUDComponent::SavePrimarySlotsInArray()
{
    SavedPrimaryItemsArray.Empty();

    for (const auto& Slot : GetSlots(E_SlotsType::Primary))
    {
        if (!Slot.IsEmpty && !Slot.IsPartOfItem && IsValid(Slot.ItemReference))
        {
            FItemDataInfo Data;
            Data.Item = Slot.ItemReference;
            Data.Slots = Slot.ItemReference->InventoryItemPayload.OccupiedSlots;
            Data.Rotation = Slot.ItemReference->InventoryItemPayload.Rotation;
            Data.SlotsType = Slot.ItemReference->InventoryItemPayload.SlotsType;
            SavedPrimaryItemsArray.Add(Data);
        }
    }
}

void UInventoryHUDComponent::LoadPrimarySlotsFromArray()
{
    if (SavedPrimaryItemsArray.Num() > 0 && IsSlotsHaveItems(E_SlotsType::Temp) || IsValidSwappedItem())
    {
        ClearAllSlotsByType(E_SlotsType::Primary);
        ClearAllSlotsByType(E_SlotsType::Temp);

        for (const FItemDataInfo& Data : SavedPrimaryItemsArray)
        {
            if (!IsValid(Data.Item)) continue;


            Data.Item->InventoryItemPayload.OccupiedSlots = Data.Slots;
            Data.Item->InventoryItemPayload.Rotation = Data.Rotation;
            Data.Item->InventoryItemPayload.SlotsType = Data.SlotsType;

            FillSlots(Data.Item, Data.Slots, Data.SlotsType);


        }

        if (HiddenSlots.IsValidIndex(0) && IsValid(HiddenSlots[0].ItemReference))
        {
            HiddenSlots[0].ItemReference = nullptr;
            HiddenSlots[0].IsEmpty = true;
            HiddenItemSlotsType = E_SlotsType::Primary;
        }
    }
}

void UInventoryHUDComponent::ClearAllSlotsByType(const E_SlotsType SlotsType)
{
    for (const FSlotStruct& Slot : GetSlots(SlotsType))
    {
        if (!GetSlots(SlotsType).IsValidIndex(Slot.Index)) continue;
        GetSlots(SlotsType)[Slot.Index].IsEmpty = true;
        GetSlots(SlotsType)[Slot.Index].IsPartOfItem = false;
        GetSlots(SlotsType)[Slot.Index].ItemReference = nullptr;

    }

    for (int i = ItemsWidgets.Num() - 1; i >= 0; --i)
    {
        UItemWidget* Item = ItemsWidgets[i];
        if (!IsValid(Item)) continue;

        if (Item->InventoryItemPayload.SlotsType == SlotsType)
        {
            Item->RemoveFromParent();
            ItemsWidgets.RemoveAt(i);
        }
    }
}










