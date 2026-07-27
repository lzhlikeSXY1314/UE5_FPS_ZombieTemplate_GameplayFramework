// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Widgets/NameAndDecription.h"
#include "Components/TextBlock.h"

void UNameAndDecription::SetItemInfo(const FText& ItemName, const FText& ItemDesc)
{
    if (NameTextBlock)
    {
        NameTextBlock->SetText(ItemName);
    }

    // ¸üÐÂÃèÊö
    if (DescriptionTextBlock)
    {
        DescriptionTextBlock->SetText(ItemDesc);
    }

}