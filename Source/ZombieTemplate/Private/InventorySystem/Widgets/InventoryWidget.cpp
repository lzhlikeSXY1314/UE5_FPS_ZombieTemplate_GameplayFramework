// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Widgets/InventoryWidget.h"
#include "Components/TextBlock.h"
#include "InventorySystem/Widgets/InventoryGridPanelWidget.h"
#include "InventorySystem/Functions/InventoryStaticFunctions.h"
#include "InventorySystem/Components/InventoryHUDComponent.h"
#include "Widgets/ShortcutWidget.h"

void UInventoryWidget::NativeConstruct()
{
    CloseInventoryKeys.Add(EKeys::Tab);
    CloseInventoryKeys.Add(EKeys::Escape);
    CloseInventoryKeys.Add(EKeys::I);
}



FReply UInventoryWidget::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey PressedKey = InKeyEvent.GetKey();

    if (InKeyEvent.IsRepeat())
    {
        return FReply::Handled();
    }

    if (CloseInventoryKeys.Contains(PressedKey))
    {
        CloseInventory();
        return FReply::Handled();
    }

    if (PressedKey == EKeys::SpaceBar)
    {
        OnMouseButtonUp(PressedKey);
        return FReply::Handled();
    }

    if (PressedKey == EKeys::LeftShift || PressedKey == EKeys::RightShift)
    {
        UInventoryHUDComponent* HUDComp = UInventoryStaticFunctions::GetInventoryHUDComponent(this);
        if (HUDComp)
        {
            HUDComp->OnDragDetected.Broadcast();
        }
        return FReply::Handled();
    }


    if (PressedKey == EKeys::LeftAlt || PressedKey == EKeys::RightAlt)
    {
        UInventoryHUDComponent* HUDComp = UInventoryStaticFunctions::GetInventoryHUDComponent(this);
        if (HUDComp)
        {
            HUDComp->AutoSort();
        }
        return FReply::Handled();
    }


    if (PressedKey == EKeys::R)
    {
        UInventoryHUDComponent* HUDComp = UInventoryStaticFunctions::GetInventoryHUDComponent(this);

        if (HUDComp->ShortcutWidget)
        {
            HUDComp->ShortcutWidget->RomoveShortcutItemData();
            return FReply::Handled();
        }
    }

    return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

FReply UInventoryWidget::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent)
{

    OnMouseButtonDown(InMouseEvent.GetEffectingButton());
    return FReply::Handled();

}

FReply UInventoryWidget::NativeOnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        OnMouseButtonUp(EKeys::LeftMouseButton);
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonUp(MyGeometry, InMouseEvent);
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

void UInventoryWidget::ShowHideTempAnim_Implementation(bool ShowTemp)
{
}

void UInventoryWidget::ShowHideTempSlots(bool Show)
{
    if (WB_Temp)
    {
        WB_Temp->SetVisibility(Show ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}


void UInventoryWidget::OnMouseButtonDown(FKey InKey)
{

    UInventoryHUDComponent* HUDComp = UInventoryStaticFunctions::GetInventoryHUDComponent(this);
    if (HUDComp)
    {
        HUDComp->OnMouseButtonDown(InKey);
    }

}

void UInventoryWidget::OnMouseButtonUp(FKey InKey)
{
    UInventoryHUDComponent* HUDComp = UInventoryStaticFunctions::GetInventoryHUDComponent(this);
    if (HUDComp)
    {
        HUDComp->OnMouseButtonUp(InKey);
    }
}
