// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Widgets/InventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Materials/MaterialInstance.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "InventorySystem/Components/InventoryHUDComponent.h"
#include "InventorySystem/Functions/InventoryStaticFunctions.h"



void UInventorySlotWidget::NativePreConstruct()
{
    if (FaceImage)
    {
        UWidgetLayoutLibrary::SlotAsCanvasSlot(FaceImage)->SetOffsets(SlotPadding);
    }
}

void UInventorySlotWidget::NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent)
{
    UInventoryHUDComponent* HUDComp = UInventoryStaticFunctions::GetInventoryHUDComponent(this);
    if (HUDComp)
    {
        HUDComp->SetRealSelectedSlot(Index,SlotsType);
        HUDComp->SelectSlot(Index, SlotsType, true);
    }
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    FReply ParentReply = Super::NativeOnMouseButtonDown(MyGeometry, MouseEvent);
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        TSharedRef<SWidget> SlateWidget = TakeWidget();
        return FReply::Handled().DetectDrag(SlateWidget,EKeys::LeftMouseButton);
    }
    return ParentReply;

}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& PointerEvent, UDragDropOperation*& OutDragDropOperation)
{
    Super::NativeOnDragDetected(MyGeometry, PointerEvent, OutDragDropOperation);
    if (UInventoryHUDComponent* HUDComp = UInventoryStaticFunctions::GetInventoryHUDComponent(this))
    {
        HUDComp->OnDragDetected.Broadcast();
    }
}



void UInventorySlotWidget::SetSlotSize(float Size)
{
    if (SlotSizeBox)
    {
        SlotSizeBox->SetWidthOverride(Size);
        SlotSizeBox->SetHeightOverride(Size);
    }
}

void UInventorySlotWidget::SelectSlot(bool bSelect)
{
    if (!FaceImage) return;

    E_SlotState State = bSelect ? E_SlotState::Selected : E_SlotState::NotSelected;
    FaceImage->SetBrushFromMaterial(GetSlotMaterial(State));
}

UMaterialInstance* UInventorySlotWidget::GetSlotMaterial(E_SlotState SlotState)
{
    switch (SlotState)
    {
    case E_SlotState::NotSelected:
        return MI_FaceSlotNotSelected;

    case E_SlotState::Selected:
        return MI_FaceSlotSelected;

    case E_SlotState::Active:
        return MI_ItemBackgroundActive;

    default:
        return MI_FaceSlotNotSelected;
    }
}

