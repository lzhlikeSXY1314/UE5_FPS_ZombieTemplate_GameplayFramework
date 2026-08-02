// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Widgets/ItemActionConfirmWidget.h"
#include "Components/SpinBox.h"
#include <Kismet/KismetMathLibrary.h>

void UItemActionConfirmWidget::InitializeText_Implementation(E_ItemActionType, const FString& InItemName)
{
}

void UItemActionConfirmWidget::InitializeNumBlock_Implementation(bool InShowNumBlock, int32 InMinValue, int32 InMaxValue)
{
}

int32 UItemActionConfirmWidget::GetValue()
{
    if (!SpinBox_Count)
    {
        return 0;
    }

    const float FloatValue = SpinBox_Count->GetValue();
    const int32 IntValue = UKismetMathLibrary::FTrunc(FloatValue);

    return IntValue;
}