// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Widgets/HerbCombineMenu.h"


void UHerbCombineMenu::RefreshCombineRecipeList_Implementation(const FGameplayTagContainer& SelectedHerbTags)
{
}

void UHerbCombineMenu::UpdateHerbCounts_Implementation()
{
}

void UHerbCombineMenu::MapHerbInt(int32 InNum1, int32 InNum2, bool bIsSame, int32& OutVal1, int32& OutVal2)
{
    if (bIsSame)
    {
        // 同种草药
        OutVal1 = (InNum1 >= 1) ? 1 : 0;
        OutVal2 = (InNum1 >= 2) ? 1 : 0;
    }
    else
    {
        //不同种
        OutVal1 = (InNum1 >= 1) ? 1 : 0;
        OutVal2 = (InNum2 >= 1) ? 1 : 0;

    }
}
