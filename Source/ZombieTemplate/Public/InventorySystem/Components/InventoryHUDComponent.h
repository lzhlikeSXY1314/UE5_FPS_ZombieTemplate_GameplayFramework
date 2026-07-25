// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventorySystem/Functions/InventoryStaticFunctions.h"
#include "InventoryHUDComponent.generated.h"


class UInventoryWidget;
class UInventorySlotWidget;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZOMBIETEMPLATE_API UInventoryHUDComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryHUDComponent();

public:	

	UPROPERTY(EditDefaultsOnly, Category = "Inventory HUD")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;
	
	void OpenInventory();
	void CloseInventory();
	void SelectSlot(int32 InIndex, E_SlotsType InSlotType, bool InShouldPlaySound);

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Sound")
	TObjectPtr<UAudioComponent> UI_Sound = nullptr;

	UFUNCTION(Category = "Inventory|Sound")
	void PlayInventorySound(E_InventorySoundType SoundType, bool bUnstoppable = false);

private:
	UFUNCTION(Category = "Inventory|Slots")
	TArray<UInventorySlotWidget*> GetSlotsByType(E_SlotsType SlotsType) const;

	UPROPERTY()
	TObjectPtr<UInventoryWidget> InventoryWidget = nullptr;

	UPROPERTY()
	E_SlotsType SelectSlotType = E_SlotsType::Primary;

	UPROPERTY()
	int32 SelectSlotIndex = 0;

};
