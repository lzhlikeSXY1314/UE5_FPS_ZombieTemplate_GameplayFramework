// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuButton.generated.h"

class UItemMenu;

UENUM(BlueprintType)
enum class E_ItemActionType : uint8
{
    Use,
    Shortcut,
    Inspect,
    Combine,
    Discard,
    Split,
    Equip,
    AttachAttachment,
    DetachAttachment
};

USTRUCT(BlueprintType)
struct FItemActionInfo
{
    GENERATED_BODY()

    // 动作显示名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionInfo")
    FText DisplayName;

    // 动作图标
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActionInfo")
    TObjectPtr<UTexture2D> IconTexture;

    // 默认构造函数（可选）
    FItemActionInfo()
        : DisplayName(FText::GetEmpty())
        , IconTexture(nullptr)
    {
    }

    FItemActionInfo(const FText& InName, UTexture2D* InIcon)
        : DisplayName(InName)
        , IconTexture(InIcon)
    {
    }
};


class UTextBlock;
class UImage;

UCLASS()
class ZOMBIETEMPLATE_API UMenuButton : public UUserWidget
{
	GENERATED_BODY()
	
protected:
    virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& InMouseEvent) override;

public:

    UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
    UImage* Foreground;

    UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
    UImage* MenuButtonIcon;

    UPROPERTY(meta = (BindWidget), VisibleAnywhere, BlueprintReadOnly, Category = "Menu Button")
    UTextBlock* Name_Text;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu Button Data")
    E_ItemActionType ActionType = E_ItemActionType::Equip;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu Button Data")
    TMap<E_ItemActionType, FItemActionInfo> ActionInfoMap;

    UPROPERTY()
    UItemMenu* ItemMenu;

    UFUNCTION()
    void SetItemMenu(UItemMenu* InMenu);

};
