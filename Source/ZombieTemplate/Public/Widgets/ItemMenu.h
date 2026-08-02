// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySystem/Widgets/MenuButton.h"
#include "ItemMenu.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemActionSelected, E_ItemActionType, SelectedAction);

class UInventoryHUDComponent;
class UMenuButton;
class USpinBox;


UCLASS()
class ZOMBIETEMPLATE_API UItemMenu : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;

public:
	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
	UMenuButton* WBP_Use;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
	UMenuButton* WBP_Shortcut;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
	UMenuButton* WBP_Inspect;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
	UMenuButton* WBP_Combine;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
	UMenuButton* WBP_Discard;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
	UMenuButton* WBP_Split;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
	UMenuButton* WBP_Equip;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
	UMenuButton* WBP_AttachAttachment;

	UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
	UMenuButton* WBP_DetachAttachment;

	UPROPERTY(BlueprintReadOnly)
	UInventoryHUDComponent* InventoryHUDComponent = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "Item Menu")
	FOnItemActionSelected OnItemActionSelected;


	UFUNCTION(BlueprintCallable, Category = "Menu Button Initialization")
	void InitMenuButtonVisibility(
		bool bUseEnabled,
		bool bShortcutEnabled,
		bool bInspectEnabled,
		bool bCombineEnabled,
		bool bDiscardEnabled,
		bool bSplitEnabled,
		bool bEquipEnabled,
		bool bAttachAttachmentEnabled,
		bool bDetachAttachmentEnabled,
		bool IsEquipped
	);


	UFUNCTION()
	void HandleMenuButtonEvents(E_ItemActionType ActionType);

	UFUNCTION()
	void CloseMenu();
};
