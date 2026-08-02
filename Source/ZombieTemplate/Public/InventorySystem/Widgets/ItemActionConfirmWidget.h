// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuButton.h"
#include "ItemActionConfirmWidget.generated.h"


class URichTextBlock;
class UButton;
class USpinBox;


UCLASS()
class ZOMBIETEMPLATE_API UItemActionConfirmWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	
	// 提示文本
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	URichTextBlock* RichText_Question;

	// 数量调节
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Btn_Minus;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USpinBox* SpinBox_Count;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Btn_Plus;

	// 确认取消
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Btn_Yes;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* Btn_No;
	

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ConfirmWidget")
	void InitializeText(E_ItemActionType ActionType, const FString& InItemName);
	virtual void InitializeText_Implementation(E_ItemActionType ActionType, const FString& InItemName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ConfirmWidget")
	void InitializeNumBlock(bool InShowNumBlock, int32 InMinValue, int32 InMaxValue);
	virtual void InitializeNumBlock_Implementation(bool InShowNumBlock, int32 InMinValue, int32 InMaxValue);

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "ConfirmWidget")
	int32 GetValue();

};
