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
        DeselectAllSlots();
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

    PlayInventorySound(E_InventorySoundType::Close,false);
}

void UInventoryHUDComponent::SelectSlot(int32 InIndex, E_SlotsType InSlotType, bool InShouldPlaySound)
{
    int32 L_SelectSlotIndex = FMath::Max(InIndex, 0);
    
    if (DragWidget) InShouldPlaySound &= (L_SelectSlotIndex != SelectSlotIndex || SelectSlotType != InSlotType);

    SelectSlotType = InSlotType;

    ApplyOffset(L_SelectSlotIndex); // 查空间，跟随拾取起点

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

void UInventoryHUDComponent::DeselectAllSlots()
{
    for (UInventorySlotWidget* Slot : GetSlotsByType(E_SlotsType::Primary))
    {
        if (!Slot) continue;
        Slot->SelectSlot(false);
    }

    for (UInventorySlotWidget* Slot : GetSlotsByType(E_SlotsType::Temp))
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

int32 UInventoryHUDComponent::GetRowBySlotType(int32 InIndex, E_SlotsType InSlotType)
{
    if (InIndex < 0) return -1;
    if(!UInventoryStaticFunctions::GetInventoryOptions(this))  return -1;
    int32 Col = InSlotType == E_SlotsType::Primary ? UInventoryStaticFunctions::GetInventoryOptions(this)->PrimaryConfig.Columns : UInventoryStaticFunctions::GetInventoryOptions(this)->TempConfig.Columns;
    return UInventoryStaticFunctions::GetRow(InIndex, Col);
}

int32 UInventoryHUDComponent::GetColumnBySlotType(int32 InIndex, E_SlotsType InSlotType)
{
    if (InIndex < 0) return -1;
    if (!UInventoryStaticFunctions::GetInventoryOptions(this))  return -1;
    int32 Col = InSlotType == E_SlotsType::Primary ? UInventoryStaticFunctions::GetInventoryOptions(this)->PrimaryConfig.Columns : UInventoryStaticFunctions::GetInventoryOptions(this)->TempConfig.Columns;
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


void UInventoryHUDComponent::AddItemWidgetToGrib(AInspectableItem* Item, const bool CanDestory, const bool IsEquipped)
{
    if (!ItemWidgetClass || !IsValid(Item)) return;
    if (Item->InventoryItemPayload.OccupiedSlots.Num() == 0) return;

    UInventoryData* InvData = UInventoryStaticFunctions::GetInventoryOptions(this);
    if (!InvData) return;

    const int32 Width = InvData->PrimaryConfig.Columns;
    const int32 FirstSlot = Item->InventoryItemPayload.OccupiedSlots[0];

    APlayerController* PC = GetPlayerController();
    if (!PC) return;

    UItemWidget* NewItemWidget = CreateWidget<UItemWidget>(PC, ItemWidgetClass);
    if (!NewItemWidget) return;

    NewItemWidget->InventoryItemPayload = Item->InventoryItemPayload;
    NewItemWidget->InventoryItemPayload.InventoryComponentReference = this;
    NewItemWidget->InventoryItemPayload.IsEquipped = IsEquipped;
    NewItemWidget->DefaultRotation = Item->InventoryItemPayload.Rotation;
    NewItemWidget->Rotation = Item->InventoryItemPayload.Rotation;
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

    if(!UInventoryStaticFunctions::GetInventoryOptions(this)) return;

    const int32 Col = L_SlotsType == E_SlotsType::Primary ? UInventoryStaticFunctions::GetInventoryOptions(this)->PrimaryConfig.Columns : UInventoryStaticFunctions::GetInventoryOptions(this)->TempConfig.Columns;
    const int32 Row = L_SlotsType == E_SlotsType::Primary ? UInventoryStaticFunctions::GetInventoryOptions(this)->PrimaryConfig.Rows : UInventoryStaticFunctions::GetInventoryOptions(this)->TempConfig.Rows;

    const int32 ItemSizeCol = DragWidget->InventoryItemPayload.ItemIconSize.X;
    const int32 ItemSizeRow = DragWidget->InventoryItemPayload.ItemIconSize.Y;

    bool IsRotated = DragWidget->InventoryItemPayload.Rotation != DragWidget->DefaultRotation;
    bool DefaultIsVertical = DragWidget->DefaultRotation == EItemRotation::Vertical;
    bool IsLongVertical = DragWidget->InventoryItemPayload.IsLongVertical;

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

void UInventoryHUDComponent::HandleDragDetected()
{
    if (IsValid(DragWidget))
    {
        //Drag End
        return;
    }

    PlayInventorySound(E_InventorySoundType::OnDragStart,true);
    InitializeDragWidget();
    DeselectAllSlots();
    SelectSlot(GetRealSelectedSlot().Index, GetRealSelectedSlot().Type, false);

}




/// </DragWidget>












