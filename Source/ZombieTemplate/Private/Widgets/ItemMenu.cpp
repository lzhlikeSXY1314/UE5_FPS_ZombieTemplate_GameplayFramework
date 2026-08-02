// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ItemMenu.h"
#include "InventorySystem/Widgets/MenuButton.h"
#include "InventorySystem/Components/InventoryHUDComponent.h"




void UItemMenu::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (WBP_Use)                WBP_Use->SetItemMenu(this);
    if (WBP_Shortcut)           WBP_Shortcut->SetItemMenu(this);
    if (WBP_Inspect)            WBP_Inspect->SetItemMenu(this);
    if (WBP_Combine)            WBP_Combine->SetItemMenu(this);
    if (WBP_Discard)            WBP_Discard->SetItemMenu(this);
    if (WBP_Split)              WBP_Split->SetItemMenu(this);
    if (WBP_Equip)              WBP_Equip->SetItemMenu(this);
    if (WBP_AttachAttachment)   WBP_AttachAttachment->SetItemMenu(this);
    if (WBP_DetachAttachment)   WBP_DetachAttachment->SetItemMenu(this);


}

FReply UItemMenu::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        CloseMenu();
    }
    return FReply::Handled();
}


void UItemMenu::InitMenuButtonVisibility(
    bool bUseEnabled,
    bool bShortcutEnabled,
    bool bInspectEnabled,
    bool bCombineEnabled,
    bool bDiscardEnabled,
    bool bSplitEnabled,
    bool bEquipEnabled,
    bool bAttachAttachmentEnabled,
    bool bDetachAttachmentEnabled
)
{
    auto SetButtonVisibility = [this](UMenuButton* Button, bool bIsVisible)
        {
            if (IsValid(Button))
            {
                Button->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
            }
        };

    SetButtonVisibility(WBP_Use, bUseEnabled);
    SetButtonVisibility(WBP_Shortcut, bShortcutEnabled);
    SetButtonVisibility(WBP_Inspect, bInspectEnabled);
    SetButtonVisibility(WBP_Combine, bCombineEnabled);
    SetButtonVisibility(WBP_Discard, bDiscardEnabled);
    SetButtonVisibility(WBP_Split, bSplitEnabled);
    SetButtonVisibility(WBP_Equip, bEquipEnabled);
    SetButtonVisibility(WBP_AttachAttachment, bAttachAttachmentEnabled);
    SetButtonVisibility(WBP_DetachAttachment, bDetachAttachmentEnabled);

    OnItemActionSelected.AddDynamic(this, &UItemMenu::HandleMenuButtonEvents);
}



void UItemMenu::HandleMenuButtonEvents(E_ItemActionType ActionType)
{
    if (InventoryHUDComponent)
    {
        InventoryHUDComponent->MenuButtonResponseFunction(ActionType);
    }
}

void UItemMenu::CloseMenu()
{
    if (InventoryHUDComponent)
    {
        InventoryHUDComponent->CloseItemMenuWidget();
    }
}
