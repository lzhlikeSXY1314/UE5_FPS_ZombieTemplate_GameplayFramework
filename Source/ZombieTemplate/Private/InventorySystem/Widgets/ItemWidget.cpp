// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Widgets/ItemWidget.h"
#include <Datas/InventoryData.h>
#include "InventorySystem/Functions/InventoryStaticFunctions.h"
#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include <Kismet/KismetMaterialLibrary.h>


void UItemWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SelectItemBackgroundMaterial(false,false);
    LoadItemIconAsync();
    LoadBulletIconAsync();
    SetAmount();
    SetItemWidgetAspectRatio();
}



void UItemWidget::SelectItemBackgroundMaterial(bool bIsSelected, bool bIsOpenMenu)
{
	if (!Background) return;
	UInventoryData* InvData = UInventoryStaticFunctions::GetInventoryOptions(this);
	if (!InvData) return;
	EItemBackgroundState TargetState;
    if (bIsOpenMenu)
    {
        TargetState = EItemBackgroundState::Menu;
    }
    else
    {
        TargetState = bIsSelected
            ? EItemBackgroundState::Selected
            : EItemBackgroundState::NotSelected;
    }

    UMaterialInstance* TargetMat = InvData->ItemBackgroundMaterials.FindRef(TargetState);
    if (!TargetMat)  TargetMat = InvData->ItemBackgroundMaterials.FindRef(EItemBackgroundState::NotSelected);
    if (TargetMat)   Background->SetBrushFromMaterial(TargetMat);
}

void UItemWidget::LoadItemIconAsync()
{
    if (!ItemIcon) return;
    TSoftObjectPtr<UTexture2D> SoftRef = InventoryItemPayload.ItemIconSoftRef;
    if (!SoftRef.IsValid())
    {
        ItemIcon->SetBrushFromTexture(nullptr);
        return;
    }
    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
    Streamable.RequestAsyncLoad(
        SoftRef.ToSoftObjectPath(),
        FStreamableDelegate::CreateUObject(this, &UItemWidget::OnItemIconLoaded, SoftRef)
    );

}

void UItemWidget::OnItemIconLoaded(TSoftObjectPtr<UTexture2D> SoftRef)
{
    if (UTexture2D* Tex = SoftRef.Get())
    {
        if (!ItemIcon) return;
        if (!CachedMaterial || CachedMaterial->Parent != ItemMaterial)
        {
            CachedMaterial = UKismetMaterialLibrary::CreateDynamicMaterialInstance(
                this, ItemMaterial, NAME_None, EMIDCreationFlags::None);
        }

        if (CachedMaterial)
        {
            CachedMaterial->SetTextureParameterValue(FName("Icon"), Tex);

            float Angle = (Rotation == EItemRotation::Vertical) ? -0.25f : 0.0f;
            CachedMaterial->SetScalarParameterValue(FName("Angle"), Angle);

            ItemIcon->SetBrushFromMaterial(CachedMaterial);
        }
    }
}

void UItemWidget::LoadBulletIconAsync()
{
    if (!BulletImage) return;
    TSoftObjectPtr<UTexture2D> SoftRef = InventoryItemPayload.BulletIconSoftRef;
    if (!SoftRef.IsValid())
    {
        BulletImage->SetBrushFromTexture(nullptr);
        return;
    }
    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
    Streamable.RequestAsyncLoad(
        SoftRef.ToSoftObjectPath(),
        FStreamableDelegate::CreateUObject(this, &UItemWidget::OnBulletIconLoaded, SoftRef)
    );

}

void UItemWidget::SetAmount()
{
    //if (!Amount_Text) return;
    if (InventoryItemPayload.IsWeapon)
    {
        OnAmountChange.Broadcast(InventoryItemPayload.AmmoAmount);
        return;
    }
    if(BulletImage) BulletImage->SetVisibility(ESlateVisibility::Hidden);

    OnAmountChange.Broadcast(InventoryItemPayload.ItemAmount);
}

void UItemWidget::SetActiveBackgroundMaterial(bool InActive)
{
    Background->SetBrushFromMaterial(
        InActive
        ? UInventoryStaticFunctions::GetInventoryOptions(this)->ItemBackgroundMaterials[EItemBackgroundState::Dragging]
        : UInventoryStaticFunctions::GetInventoryOptions(this)->ItemBackgroundMaterials[EItemBackgroundState::NotSelected]
    );
}

int32 UItemWidget::GetFirstOccupiedSlotIndex()
{
    if(InventoryItemPayload.OccupiedSlots.IsValidIndex(0)) return InventoryItemPayload.OccupiedSlots[0];
    return -1;
}

void UItemWidget::PlayRotationAnimation_Implementation(EItemRotation InRotation)
{
    Rotation = InRotation;

    const float CurrentWidth = SizeBox_Root->GetWidthOverride();
    const float CurrentHeight = SizeBox_Root->GetHeightOverride();

    SizeBox_Root->SetWidthOverride(CurrentHeight);
    SizeBox_Root->SetHeightOverride(CurrentWidth);

    
    if (CachedMaterial)
    {
        float Angle = (Rotation == EItemRotation::Vertical) ? -0.25f : 0.0f;
        CachedMaterial->SetScalarParameterValue(FName("Angle"), Angle);
    }

}

void UItemWidget::PlayRotationToDefaultAnimation_Implementation()
{
}

void UItemWidget::HideInfoOverlayAnimation_Implementation()
{
}

void UItemWidget::SetItemWidgetAspectRatio()
{
    if (!SizeBox_Root) return;
    int32 Cols = (DefaultRotation == EItemRotation::Horizontal)
        ? InventoryItemPayload.ItemIconSize.X
        : InventoryItemPayload.ItemIconSize.Y;

    int32 Rows = (DefaultRotation == EItemRotation::Horizontal)
        ? InventoryItemPayload.ItemIconSize.Y
        : InventoryItemPayload.ItemIconSize.X;

    UInventoryData* InvData = UInventoryStaticFunctions::GetInventoryOptions(this);
    if (!InvData) return;

    float Width = Cols * InvData->SlotSize;
    float Height = Rows * InvData->SlotSize;

    SizeBox_Root->SetWidthOverride(Width);
    SizeBox_Root->SetHeightOverride(Height);
}

void UItemWidget::OnBulletIconLoaded(TSoftObjectPtr<UTexture2D> SoftRef)
{
    if (UTexture2D* Tex = SoftRef.Get())
    {
        if (!BulletImage) return;
        BulletImage->SetBrushFromTexture(Tex);
        BulletImage->SetDesiredSizeOverride(FVector2D(InventoryItemPayload.BulletImageSize.X, InventoryItemPayload.BulletImageSize.Y));
    }
}

