// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShortcutSlot.generated.h"

//UENUM(BlueprintType)
//enum class EShortcutDirection  : uint8
//{
//	Up,     
//	Down,   
//	Left,  
//	Right 
//};


class UImage;
class UTextBlock;

UCLASS()
class ZOMBIETEMPLATE_API UShortcutSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> BackgroundIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> WeaponIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> BaseSlotsText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ShortcutSlot|Data")
	int32 Index;



};
