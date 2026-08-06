// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTags.h"
#include "HerbCombineMenu.generated.h"



UCLASS()
class ZOMBIETEMPLATE_API UHerbCombineMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	//±³°ü£º»ù´¡²ÝÒ©ÊýÁ¿
	UPROPERTY(BlueprintReadOnly, Category = "HerbCombineMenu")
	int32 GreenHerbNum = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HerbCombineMenu")
	int32 RedHerbNum = 0;

	UPROPERTY(BlueprintReadOnly, Category = "HerbCombineMenu")
	int32 YellowHerbNum = 0;

	//±³°ü£ºÈ«²¿»ìºÏÒ©²ÝÊýÁ¿ RE4RÈ«²¿6ÖÖ
	UPROPERTY(BlueprintReadOnly, Category = "HerbCombineMenu")
	int32 MixedHerb_GG_Num = 0;	//ÂÌ+ÂÌ

	UPROPERTY(BlueprintReadOnly, Category = "HerbCombineMenu")
	int32 MixedHerb_GR_Num = 0;	//ÂÌ+ºì

	UPROPERTY(BlueprintReadOnly, Category = "HerbCombineMenu")
	int32 MixedHerb_GY_Num = 0;	//ÂÌ+»Æ

	UPROPERTY(BlueprintReadOnly, Category = "HerbCombineMenu")
	int32 MixedHerb_RY_Num = 0;	//ÂÌ+»Æ

	UPROPERTY(BlueprintReadOnly, Category = "HerbCombineMenu")
	int32 MixedHerb_GGG_Num = 0;	//ÂÌ+ÂÌ+ÂÌ

	UPROPERTY(BlueprintReadOnly, Category = "HerbCombineMenu")
	int32 MixedHerb_GGY_Num = 0;	//ÂÌ+ÂÌ+»Æ

	UPROPERTY(BlueprintReadOnly, Category = "HerbCombineMenu")
	int32 MixedHerb_GRY_Num = 0;	//ÂÌ+ºì+»Æ




	UFUNCTION(BlueprintNativeEvent, Category = "HerbCombineMenu|UI")
	void RefreshCombineRecipeList(const FGameplayTagContainer& SelectedHerbTags);
	virtual void RefreshCombineRecipeList_Implementation(const FGameplayTagContainer& SelectedHerbTags);

	UFUNCTION(BlueprintNativeEvent, Category = "HerbCombineMenu|UI")
	void UpdateHerbCounts();
	virtual void UpdateHerbCounts_Implementation();

	UFUNCTION(BlueprintCallable,BlueprintPure,Category = "HerbCombineMenu|Logic")
	static void MapHerbInt(int32 InNum1, int32 InNum2, bool bIsSame, int32& OutVal1, int32& OutVal2);

};
