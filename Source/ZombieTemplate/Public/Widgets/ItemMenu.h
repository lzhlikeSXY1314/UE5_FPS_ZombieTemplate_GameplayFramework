// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemMenu.generated.h"

class UMenuButton;

UCLASS()
class ZOMBIETEMPLATE_API UItemMenu : public UUserWidget
{
	GENERATED_BODY()
	
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
		bool bDetachAttachmentEnabled
	);

};
