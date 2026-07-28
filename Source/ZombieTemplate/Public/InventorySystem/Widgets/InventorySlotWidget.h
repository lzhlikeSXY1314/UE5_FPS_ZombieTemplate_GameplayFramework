// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySystem/structs/InventoryTypes.h"
#include "InventorySlotWidget.generated.h"

class UImage;
class USizeBox;
class UMaterialInstance;
class UInventoryHUDComponent;


UCLASS(BlueprintType)
class ZOMBIETEMPLATE_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    virtual void NativePreConstruct() override;
    virtual void NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void NativeOnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& PointerEvent, UDragDropOperation*& OutDragDropOperation) override;

public:

    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    void SetSlotSize(float Size);

    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    void SelectSlot(bool bSelect);

    UFUNCTION(BlueprintCallable, Category = "Inventory Slot")
    UMaterialInstance* GetSlotMaterial(E_SlotState SlotState);

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slot Data", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float SlotPadding = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slot Material")
    TObjectPtr<UMaterialInstance> MI_FaceSlotNotSelected;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite ,Category = "Slot Material")
    TObjectPtr<UMaterialInstance> MI_FaceSlotSelected;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Slot Material")
    TObjectPtr<UMaterialInstance> MI_ItemBackgroundActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Data", meta = (ExposeOnSpawn = true))
    int32 Index;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Data", meta = (ExposeOnSpawn = true))
    E_SlotsType SlotsType;


private:

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> FaceImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> SlotBackgroundImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USizeBox> SlotSizeBox;


};
