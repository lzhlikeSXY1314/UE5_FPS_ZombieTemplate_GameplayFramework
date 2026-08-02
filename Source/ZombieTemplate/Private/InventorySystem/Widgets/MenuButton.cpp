// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Widgets/MenuButton.h"
#include "Widgets/ItemMenu.h"

FReply UMenuButton::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent)
{
	if (ItemMenu && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		ItemMenu->OnItemActionSelected.Broadcast(ActionType);
	}

	if (ItemMenu && InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		ItemMenu->CloseMenu();
	}

	return FReply::Handled();
}

void UMenuButton::SetItemMenu(UItemMenu* InMenu)
{
	ItemMenu = InMenu;
}
