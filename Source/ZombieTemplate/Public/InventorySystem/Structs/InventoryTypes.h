// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class E_SlotsType : uint8
{
    Primary,
    Temp,
    Storage,
    HiddenSlots,
    Equipment
};

UENUM(BlueprintType)
enum class E_SlotState : uint8
{
    NotSelected UMETA(DisplayName = "Not Selected"),
    Selected    UMETA(DisplayName = "Selected"),
    Active      UMETA(DisplayName = "Active")
};

UENUM(BlueprintType)
enum class E_InventorySoundType : uint8
{
    None UMETA(DisplayName = "None"),
    Open UMETA(DisplayName = "Open"),
    Close UMETA(DisplayName = "Close"),
    OpenMenu UMETA(DisplayName = "OpenMenu"),
    CloseMenu UMETA(DisplayName = "CloseMenu"),
    SelectItem UMETA(DisplayName = "SelectItem"),
    SelectSlot UMETA(DisplayName = "SelectSlot"),
    SelectSlotWhenItemDragged UMETA(DisplayName = "SelectSlotWhenItemDragged"),
    SelectButton UMETA(DisplayName = "SelectButton"),
    OnDragStart UMETA(DisplayName = "OnDragStart"),
    OnDragEnd UMETA(DisplayName = "OnDragEnd"),
    Rotate UMETA(DisplayName = "Rotate"),
    DragCanceled UMETA(DisplayName = "DragCanceled"),
    SelectShortcut_InInventory UMETA(DisplayName = "SelectShortcut_InInventory"),
    PushButton UMETA(DisplayName = "PushButton"),
    ShowQuestion UMETA(DisplayName = "ShowQuestion"),
    CombineCompiled UMETA(DisplayName = "CombineCompiled"),
    CombineFailed UMETA(DisplayName = "CombineFailed"),
    AddItemToShortcut UMETA(DisplayName = "AddItemToShortcut"),
    Autosort UMETA(DisplayName = "Autosort"),
    ShowMessage UMETA(DisplayName = "ShowMessage"),
    PickupItem UMETA(DisplayName = "PickupItem"),
    TakeAll UMETA(DisplayName = "TakeAll"),
    SwitchTab UMETA(DisplayName = "SwitchTab"),
    EndInspectItem UMETA(DisplayName = "EndInspectItem"),
    CloseShortcuts UMETA(DisplayName = "CloseShortcuts"),
    CloseQuestionMessage UMETA(DisplayName = "CloseQuestionMessage"),
    CancelCombine UMETA(DisplayName = "CancelCombine"),
    SlotsSelectionWhileDraggingAnIncompatibleItem UMETA(DisplayName = "SlotsSelectionWhileDraggingAnIncompatibleItem"),
    UnableToPlaceDraggableItemInSlots UMETA(DisplayName = "UnableToPlaceDraggableItemInSlots")
};