// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySystem/Components/InventoryHUDComponent.h"
#include "InventorySystem/Widgets/InventoryWidget.h"
#include "GameFramework/Character.h"
#include "Components/AudioComponent.h"
#include <Kismet/GameplayStatics.h>
#include "InventorySystem/Widgets/InventoryGridPanelWidget.h"
#include "InventorySystem/Widgets/InventorySlotWidget.h"

// Sets default values for this component's properties
UInventoryHUDComponent::UInventoryHUDComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


}

void UInventoryHUDComponent::OpenInventory()
{
    if (!InventoryWidgetClass) return;

    ACharacter* Char = Cast<ACharacter>(GetOwner());
    if (!Char) return;

    APlayerController* PC = Cast<APlayerController>(Char->GetController());
    if (!PC) return;

    if (!InventoryWidget)
    {
        InventoryWidget = CreateWidget<UInventoryWidget>(PC, InventoryWidgetClass);
        if (InventoryWidget)
        {
            InventoryWidget->AddToViewport();
            InventoryWidget->SetVisibility(ESlateVisibility::Collapsed); // Ä¬ÈÏÒþ²Ø
        }
    }

    if (InventoryWidget)
    {
        InventoryWidget->SetVisibility(ESlateVisibility::Visible); // ÏÔÊ¾
        PlayInventorySound(E_InventorySoundType::Open,true);
        InventoryWidget->SetFocus();
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
        PC->SetPause(true);

    }

    int32 ViewportX, ViewportY;
    PC->GetViewportSize(ViewportX, ViewportY);

    int32 CenterX = ViewportX / 2;
    int32 CenterY = ViewportY / 2;

    PC->SetMouseLocation(CenterX, CenterY);

}

void UInventoryHUDComponent::CloseInventory()
{

    PlayInventorySound(E_InventorySoundType::Close,false);
}

void UInventoryHUDComponent::SelectSlot(int32 InIndex, E_SlotsType InSlotType, bool InShouldPlaySound)
{
    const int32 L_SelectSlotIndex = fmax(InIndex,0);
    SelectSlotType = InSlotType;
    SelectSlotIndex = fmax(L_SelectSlotIndex, 0);
    for (UInventorySlotWidget* L_Slot : GetSlotsByType(SelectSlotType))
    {
        if (!L_Slot) continue;

        if (L_Slot->Index == SelectSlotIndex)
        {
            L_Slot->SelectSlot(true);
            PlayInventorySound(InShouldPlaySound ? E_InventorySoundType::SelectSlot : E_InventorySoundType::None, false);
        }
        else
        {
            L_Slot->SelectSlot(false);
        }
    }


}

void UInventoryHUDComponent::PlayInventorySound(E_InventorySoundType SoundType, bool bUnstoppable)
{
    if (UI_Sound)
    {
        UI_Sound->Stop();
        UI_Sound = nullptr;
    }
    USoundBase* Sound = UInventoryStaticFunctions::GetInventorySound(this, SoundType);
    UAudioComponent* NewAudioComp = UGameplayStatics::SpawnSound2D(GetWorld(),Sound,1.0f,1.0f,0.0f,nullptr,false );
    if (!bUnstoppable && NewAudioComp)
    {
        UI_Sound = NewAudioComp;
    }
    else
    {
        UI_Sound = nullptr;
    }


}

TArray<UInventorySlotWidget*> UInventoryHUDComponent::GetSlotsByType(E_SlotsType SlotsType) const
{
    if (!InventoryWidget) return {};
    switch (SlotsType)
    {
    case E_SlotsType::Primary:
        if (InventoryWidget->WB_Primary)
            return InventoryWidget->WB_Primary->SlotWidgets;
        break;

    case E_SlotsType::Temp:
        if (InventoryWidget->WB_Temp)
            return InventoryWidget->WB_Temp->SlotWidgets;
        break;

    default:
        break;
    }

    return {};

}






