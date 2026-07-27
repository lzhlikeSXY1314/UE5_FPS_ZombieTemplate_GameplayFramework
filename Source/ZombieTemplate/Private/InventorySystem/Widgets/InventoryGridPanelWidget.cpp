// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Widgets/InventoryGridPanelWidget.h"
#include "Datas/InventoryData.h"
#include <InventorySystem/Functions/InventoryStaticFunctions.h>
#include "Components/GridPanel.h"
#include "InventorySystem/Widgets/InventorySlotWidget.h"

void UInventoryGridPanelWidget::NativePreConstruct()
{
    if (IsValid(InventoryOptions))
    {
        InitializeGrid();
    }
}

void UInventoryGridPanelWidget::NativeConstruct()
{
    if (!IsDesignTime())
    {
        UInventoryData* GlobalOptions = UInventoryStaticFunctions::GetInventoryOptions(this);
        if (IsValid(GlobalOptions))
        {
            InventoryOptions = GlobalOptions;
            InitializeGrid();
        }
    }
}

void UInventoryGridPanelWidget::InitializeGrid()
{
    if (!Grid || !InventoryOptions) return;
    Grid->ClearChildren();

    const FInventoryGridConfig* ActiveConfig = nullptr;
    switch (SlotsType)
    {
    case E_SlotsType::Primary:
        ActiveConfig = &InventoryOptions->PrimaryConfig;
        break;

    case E_SlotsType::Temp:
        // 临时栏需要先判断全局启用开关
        if (InventoryOptions->bEnableTempSlots)
        {
            ActiveConfig = &InventoryOptions->TempConfig;
        }
        break;

        // 预留扩展：仓库、隐藏栏、装备栏，后续加配置直接补case
    case E_SlotsType::Storage:
    case E_SlotsType::HiddenSlots:
    case E_SlotsType::Equipment:
    default:
        // 未实现的类型默认用主背包配置兜底，也可以直接return不生成
        ActiveConfig = &InventoryOptions->PrimaryConfig;
        break;
    }

    if (!ActiveConfig || !IsValid(ActiveConfig->SlotWidgetClass)) return;

    Grid->ClearChildren();
    const int32 TotalSlots = ActiveConfig->Columns * ActiveConfig->Rows;

    for (int32 i = 0; i < TotalSlots; ++i)
    {
        const int32 Row = i / ActiveConfig->Columns;
        const int32 Col = i % ActiveConfig->Columns;

        UInventorySlotWidget* NewSlot = CreateWidget<UInventorySlotWidget>(this, ActiveConfig->SlotWidgetClass);
        if (!NewSlot) continue;

        NewSlot->SetSlotSize(InventoryOptions->SlotSize);
        NewSlot->Index = i;
        NewSlot->SlotsType = SlotsType;

        Grid->AddChildToGrid(NewSlot, Row, Col);
        SlotWidgets.Add(NewSlot);
    }

}
