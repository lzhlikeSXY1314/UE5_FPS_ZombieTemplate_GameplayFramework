// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NameAndDecription.generated.h"

class UTextBlock;

UCLASS()
class ZOMBIETEMPLATE_API UNameAndDecription : public UUserWidget
{
	GENERATED_BODY()
public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> NameTextBlock;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DescriptionTextBlock;

    UFUNCTION(BlueprintCallable, Category = "Item Info")
    void SetItemInfo(const FText& ItemName, const FText& ItemDesc);
};
