// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Widgets/InventoryWidget.h"
#include "Components/TextBlock.h"
#include "InventorySystem/Widgets/InventoryGridPanelWidget.h"
#include "InventorySystem/Functions/InventoryStaticFunctions.h"
#include "InventorySystem/Components/InventoryHUDComponent.h"

void UInventoryWidget::NativeConstruct()
{
    CloseInventoryKeys.Add(EKeys::Tab);
    CloseInventoryKeys.Add(EKeys::Escape);
    CloseInventoryKeys.Add(EKeys::I);
}



FReply UInventoryWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey PressedKey = InKeyEvent.GetKey();
    if (CloseInventoryKeys.Contains(PressedKey))
    {
        CloseInventory();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

FReply UInventoryWidget::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        CloseInventory();
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonDown(MyGeometry, InMouseEvent);
}

void UInventoryWidget::SetText(const FText& BaseSlots, const FText& TempSlots)
{
    // 设置主背包标题，判空兜底
    if (IsValid(BaseSlotsText))
    {
        BaseSlotsText->SetText(BaseSlots);
    }

    // 设置临时栏标题，判空兜底
    if (IsValid(TempSlotsText))
    {
        TempSlotsText->SetText(TempSlots);
    }
}

void UInventoryWidget::CloseInventory()
{
    SetVisibility(ESlateVisibility::Collapsed);

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        PC->SetPause(false);
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }

    UInventoryHUDComponent* HUDComp = UInventoryStaticFunctions::GetInventoryHUDComponent(this);
    if (HUDComp)
    {
        HUDComp->CloseInventory();
    }

}
