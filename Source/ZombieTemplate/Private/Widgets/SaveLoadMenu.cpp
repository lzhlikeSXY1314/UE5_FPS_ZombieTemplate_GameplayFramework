// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SaveLoadMenu.h"
#include "Widgets/SaveSlotWidget.h"
#include "SaveSystem/ZombieSaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "SaveSystem/ZombieSaveGame.h"
#include <Components/ScrollBoxSlot.h>
#include <Blueprint/WidgetBlueprintLibrary.h>

void USaveLoadMenu::NativeConstruct()
{
    Super::NativeConstruct();

    if (SaveButton) SaveButton->OnClicked.AddDynamic(this, &USaveLoadMenu::OnSaveClicked);
    if (LoadButton) LoadButton->OnClicked.AddDynamic(this, &USaveLoadMenu::OnLoadClicked);
    if (DeleteButton) DeleteButton->OnClicked.AddDynamic(this, &USaveLoadMenu::OnDeleteClicked);
    if (CloseButton) CloseButton->OnClicked.AddDynamic(this, &USaveLoadMenu::OnCloseClicked);

    // 获取玩家控制器
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        // 设置输入模式为 UI Only（不锁定鼠标）
        UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(PC, nullptr, EMouseLockMode::DoNotLock, false);
        // 暂停游戏
        UGameplayStatics::SetGamePaused(GetWorld(), true);
        // 显示鼠标光标
        PC->bShowMouseCursor = true;
    }
}

void USaveLoadMenu::NativePreConstruct()
{
    Super::NativePreConstruct();

    // 编辑器预览时刷新列表，显示占位槽位
    RefreshSlotList();
}

void USaveLoadMenu::RefreshSlotList()
{
    if (!SlotScrollBox || !SlotWidgetClass) return;
    SlotScrollBox->ClearChildren();
    SlotScrollBox->ClearChildren();
    SlotWidgets.Empty();

    UZombieSaveManager* SaveMgr = UZombieSaveManager::GetSaveManager(this);
    
    if (!IsDesignTime())
    {
        if (!SaveMgr) return;
    }
    
    TArray<FString> SlotNames = UZombieSaveManager::GetAllSlotNames();
    for (int32 i = 0; i < SlotNames.Num(); ++i)
    {
        USaveSlotWidget* SlotWidget = CreateWidget<USaveSlotWidget>(GetWorld(), SlotWidgetClass);
        if (!SlotWidget) continue;


        if (!IsDesignTime())
        {
            // 获取槽位信息（可能为 nullptr）
            UZombieSaveGame* SaveData = SaveMgr->GetSlotInfo(SlotNames[i]);
            bool bIsEmpty = (SaveData == nullptr);
            FDateTime Time = bIsEmpty ? FDateTime::Now() : SaveData->SaveTimestamp;
            SlotWidget->SetSlotInfo(i, SlotNames[i], Time, bIsEmpty);
            SlotWidget->OnSlotSelected.AddDynamic(this, &USaveLoadMenu::OnSlotClicked);
        }
      
        SlotScrollBox->AddChild(SlotWidget);
        SlotWidgets.Add(SlotWidget);

        // 设置底部间隔
        if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(SlotWidget->Slot))
        {
            ScrollSlot->SetPadding(SlotPadding); // 底部10像素
        }

    }
}

void USaveLoadMenu::OnSlotClicked(int32 SlotIndex)
{
    SelectedSlotIndex = SlotIndex;
    // 这里更新选中视觉效果（例如改变选中按钮颜色）
    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        if (SlotWidgets[i])
        {
            SlotWidgets[i]->SetSelected(i == SlotIndex);
        }
    }
}

void USaveLoadMenu::OnSaveClicked()
{
    if (SelectedSlotIndex < 0) return;
    FString SlotName = FString::Printf(TEXT("Slot%d"), SelectedSlotIndex);
    UZombieSaveManager* SaveMgr = UZombieSaveManager::GetSaveManager(this);
    if (SaveMgr)
    {
        SaveMgr->SaveGame(SlotName);
        RefreshSlotList();   // 刷新显示时间
    }
}

void USaveLoadMenu::OnLoadClicked()
{
    if (SelectedSlotIndex < 0) return;
    FString SlotName = FString::Printf(TEXT("Slot%d"), SelectedSlotIndex);
    UZombieSaveManager* SaveMgr = UZombieSaveManager::GetSaveManager(this);
    if (SaveMgr)
    {
        UZombieSaveGame* Data = SaveMgr->LoadGame(SlotName);
        if (Data)
        {
            APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
            SaveMgr->ApplySaveData(Data, PC);
            // 加载成功后可以关闭菜单
            OnCloseClicked();
            /*RemoveFromParent();*/
        }
    }
}

void USaveLoadMenu::OnDeleteClicked()
{
    if (SelectedSlotIndex < 0) return;
    FString SlotName = FString::Printf(TEXT("Slot%d"), SelectedSlotIndex);
    UZombieSaveManager* SaveMgr = UZombieSaveManager::GetSaveManager(this);
    if (SaveMgr)
    {
        SaveMgr->DeleteSave(SlotName);
        RefreshSlotList();
    }
}

void USaveLoadMenu::OnCloseClicked()
{
    // 获取玩家控制器
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        // 设置输入模式为仅游戏
        UWidgetBlueprintLibrary::SetInputMode_GameOnly(PC, true);
        // 取消暂停游戏
        UGameplayStatics::SetGamePaused(GetWorld(), false);
        // 隐藏鼠标光标
        PC->bShowMouseCursor = false;
    }

    RemoveFromParent();
}