// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InspectableItem.h"
#include "InventorySystem/Functions/InventoryStaticFunctions.h"
#include <InventorySystem/Components/InventoryHUDComponent.h>

FName AInspectableItem::GetUniqueSaveID_Implementation() const
{
    // 如果指定了 SaveActorID，优先使用（用于动态生成的掉落物）
    if (SaveActorID != NAME_None)
        return SaveActorID;
    // 否则使用默认 FName（适用于关卡中放置的物品）
    return GetFName();
}

FActorSaveData AInspectableItem::GetSaveData_Implementation() const
{
    FActorSaveData Data;
    Data.ActorID = GetUniqueSaveID_Implementation();
    Data.ActorClassPath = GetClass()->GetPathName();
    Data.WorldTransform = GetActorTransform();
    // 基本物品没有弹药、配件等，保持默认即可
    return Data;
}

void AInspectableItem::RestoreState_Implementation(const FActorSaveData& Data)
{
    SetActorTransform(Data.WorldTransform);
}

void AInspectableItem::ResetToDefault_Implementation()
{ 
    // 如果记录过原始位置（静态放置的物品），则恢复之
    if (!OriginalWorldTransform.GetLocation().IsNearlyZero())
    {
        SetActorTransform(OriginalWorldTransform);
    }
}

// Sets default values
AInspectableItem::AInspectableItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    RootComponent = VisualMesh;



    // 设置碰撞：允许物理碰撞，同时保留射线检测（Visibility）
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    VisualMesh->SetCollisionObjectType(ECC_PhysicsBody);       // 物理物体类型
    VisualMesh->SetCollisionResponseToAllChannels(ECR_Block);  // 先阻挡所有
    // 然后根据需要微调（例如让 Pawn 推不动它，或忽略某些通道）
    VisualMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);       // 玩家不碰撞，避免卡住
    VisualMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);     // 摄像机忽略
    VisualMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);  // 确保射线可命中



    // 创建控件组件，附着到根组件
    InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
    InteractionWidget->SetupAttachment(VisualMesh);
    InteractionWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InteractionWidget->SetDrawSize(FVector2D(50.0f, 50.0f));
    InteractionWidget->SetWidgetSpace(EWidgetSpace::Screen);
}

void AInspectableItem::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);   // 必须调用父类

    if (VisualMesh)
    {
        VisualMesh->SetWorldScale3D(MeshScale);
    }
}


void AInspectableItem::OnInteract_Implementation(AActor* Interactor)
{
    UInventoryHUDComponent* InventoryHUD = UInventoryStaticFunctions::GetInventoryHUDComponent(this);
    if (!InventoryHUD) return;

    if (InventoryItemPayload.IsWeapon)
    {
        InventoryItemPayload.ItemAmount = FMath::Max(1, InventoryItemPayload.ItemAmount) ;
    }

    if (InventoryItemPayload.ItemAmount <= 0) return;
    if(!InventoryHUD->CanAddItem(this,E_SlotsType::Primary,InventoryItemPayload.ItemAmount)) return;

    InventoryHUD->AddItemToSlots(this, InventoryItemPayload.ItemAmount, E_SlotsType::Primary);
    InventoryHUD->PlayInventorySound(E_InventorySoundType::PickupItem,false);
}

void AInspectableItem::DiscardItemInInventory_Implementation(int32 Quantity)
{
}


// Called when the game starts or when spawned
void AInspectableItem::BeginPlay()
{
	Super::BeginPlay();
    VisualMesh->SetMassOverrideInKg(NAME_None, 10.0f, true); // 10kg
    VisualMesh->SetLinearDamping(0.1f);   // 平移阻力
    VisualMesh->SetAngularDamping(0.1f);  // 旋转阻力
    VisualMesh->SetSimulatePhysics(true);
    VisualMesh->SetEnableGravity(true);
    VisualMesh->SetWorldScale3D(MeshScale);

    OriginalWorldTransform = GetActorTransform(); //SaveGame


}
