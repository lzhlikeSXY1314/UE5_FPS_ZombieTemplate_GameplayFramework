// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SaveSlotWidget.h"

void USaveSlotWidget::SetSelected(bool bSelected)
{
    if (!SelectButton) return;
    FButtonStyle NewStyle = SelectButton->WidgetStyle;
    if (bSelected)
    {
        NewStyle.Normal.SetResourceObject(SelectMaterial);
        NewStyle.Hovered.SetResourceObject(SelectMaterial);
        NewStyle.Pressed.SetResourceObject(SelectMaterial);
    }
    else
    {
        NewStyle.Normal.SetResourceObject(NoSelectMaterial);
        NewStyle.Hovered.SetResourceObject(HoverMaterial);
        NewStyle.Pressed.SetResourceObject(SelectMaterial);
    }

    // 应用新样式
    SelectButton->SetStyle(NewStyle);

}

void USaveSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (SelectButton)
    {
        SelectButton->OnClicked.AddDynamic(this, &USaveSlotWidget::OnSelectButtonClicked);
    }
}

void USaveSlotWidget::SetSlotInfo(int32 InIndex, const FString& SlotName, const FDateTime& Timestamp, bool bIsEmpty)
{
    SlotIndex = InIndex;
    if (SlotNameText)
    {
        SlotNameText->SetText(FText::FromString(SlotName));
    }
    if (TimeText)
    {
        if (bIsEmpty)
        {
            TimeText->SetText(FText::FromString(TEXT("Empty")));
        }
        else
        {
            TimeText->SetText(FText::FromString(Timestamp.ToString()));
        }
    }

}


void USaveSlotWidget::OnSelectButtonClicked()
{
    OnSlotSelected.Broadcast(SlotIndex);
}
