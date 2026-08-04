// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Widgets/InspectItemWidget.h"
#include "InventorySystem/Functions/InventoryStaticFunctions.h"
#include "InventorySystem/Components/InventoryHUDComponent.h"

FReply UInspectItemWidget::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent)
{
    UInventoryHUDComponent* HUDComp = UInventoryStaticFunctions::GetInventoryHUDComponent(this);
    if (HUDComp)
    {
        HUDComp->OnMouseButtonDown(InMouseEvent.GetEffectingButton());
       
    }
    return FReply::Handled();
}
