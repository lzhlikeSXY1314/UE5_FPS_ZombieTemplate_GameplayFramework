// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "SaveSlotWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotSelected, int32, SlotIndex);

UCLASS()
class ZOMBIETEMPLATE_API USaveSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    /** 此槽位对应的索引 (0~9) */
    int32 SlotIndex;

    /** 被选中时触发的委托，广播槽位索引 */
    UPROPERTY(BlueprintAssignable, Category = "SaveSlot")
    FOnSlotSelected OnSlotSelected;

    /** 设置此槽位显示的信息（名称、时间） */
    UFUNCTION(BlueprintCallable, Category = "SaveSlot")
    void SetSlotInfo(int32 InIndex, const FString& SlotName, const FDateTime& Timestamp, bool bIsEmpty);

    /** 未选中状态的材质 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* NoSelectMaterial;

    /** 选中状态的材质 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* SelectMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* HoverMaterial;

    /** 应用选中/未选中的视觉样式 */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetSelected(bool bSelected);


protected:
    virtual void NativeConstruct() override;

    // 界面绑定（必须命名为与蓝图中控件名称相同）
    UPROPERTY(meta = (BindWidget))
    UTextBlock* SlotNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TimeText;

    UPROPERTY(meta = (BindWidget))
    UButton* SelectButton;

private:
    UFUNCTION()
    void OnSelectButtonClicked();
};
