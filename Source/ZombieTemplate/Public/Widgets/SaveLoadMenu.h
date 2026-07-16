// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "SaveLoadMenu.generated.h"

class USaveSlotWidget;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadAnimationStart);


UCLASS()
class ZOMBIETEMPLATE_API USaveLoadMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativePreConstruct() override;


public:
    /** 当前选中的槽位索引（-1 表示未选中） */
    UPROPERTY(BlueprintReadOnly, Category = "SaveMenu")
    int32 SelectedSlotIndex = -1;

    // 绑定蓝图中放置的控件
    UPROPERTY(meta = (BindWidget))
    UScrollBox* SlotScrollBox;

    UPROPERTY(meta = (BindWidget))
    UButton* SaveButton;

    UPROPERTY(meta = (BindWidget))
    UButton* LoadButton;

    UPROPERTY(meta = (BindWidget))
    UButton* DeleteButton;

    UPROPERTY(meta = (BindWidget))
    UButton* CloseButton;

    /** 单个槽位控件的类（在蓝图中设置） */
    UPROPERTY(EditDefaultsOnly, Category = "SaveMenu")
    TSubclassOf<USaveSlotWidget> SlotWidgetClass;

    /** 滚动列表中每个槽位之间的间距 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SaveMenu")
    FMargin SlotPadding = FMargin(0.0f, 0.0f, 0.0f, 10.0f);

    /** 存储所有生成的槽位控件 */
    TArray<USaveSlotWidget*> SlotWidgets;

    /** 加载存档完成时触发，蓝图可绑定此委托开始播放动画 */
    UPROPERTY(BlueprintAssignable, Category = "SaveMenu")
    FOnLoadAnimationStart OnLoadAnimationStart;

    /** 蓝图在动画结束时调用此函数，以关闭菜单并恢复游戏 */
    UFUNCTION(BlueprintCallable, Category = "SaveMenu")
    void FinishLoadAndClose();

private:
    /** 刷新整个槽位列表 */
    void RefreshSlotList();

    /** 当某个槽位被点击时调用 */
    UFUNCTION()
    void OnSlotClicked(int32 SlotIndex);

    // 按钮点击回调
    UFUNCTION()
    void OnSaveClicked();
    UFUNCTION()
    void OnLoadClicked();
    UFUNCTION()
    void OnDeleteClicked();
    UFUNCTION()
    void OnCloseClicked();



};
