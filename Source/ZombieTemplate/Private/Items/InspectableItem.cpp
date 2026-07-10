// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/InspectableItem.h"


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
    Destroy();
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
}
