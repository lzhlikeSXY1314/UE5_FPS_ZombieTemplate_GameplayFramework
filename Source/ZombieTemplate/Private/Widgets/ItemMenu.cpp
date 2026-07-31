// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/ItemMenu.h"
#include "InventorySystem/Widgets/MenuButton.h"

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

}