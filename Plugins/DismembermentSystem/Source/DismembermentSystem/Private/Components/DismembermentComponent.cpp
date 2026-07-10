// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DismembermentComponent.h"
#include <Data/LimbMap.h>
#include "Rendering/SkeletalMeshRenderData.h" 
#include "SkeletalRenderPublic.h"
#include "Animation/DismemberedAnimInstance.h"
#include "PhysicsEngine/PhysicsAsset.h" //Neb+5.2+

#define AllMeshes for(USkeletalMeshComponent* Mesh : GetAllMeshes()) if(Mesh) Mesh

// Sets default values for this component's properties
UDismembermentComponent::UDismembermentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	FBodyInstance Body;
	Body.SetObjectType(ECC_PhysicsBody);
	Body.SetResponseToAllChannels(ECR_Ignore);
	Body.SetResponseToChannel(ECC_WorldStatic, ECR_Block);
	Body.SetResponseToChannel(ECC_WorldStatic, ECR_Block);
	LimbCollisionSettings = Body;

	if (GetOwner() && DismembermentMeshComponents.Num() < 1 && SourceMeshComponents.Num() < 1 && !bOverrideSourceMesh)
	{
		USkeletalMeshComponent* OwnerSourceMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

		if (OwnerSourceMesh)
		{
			SourceMeshComponents.Add(OwnerSourceMesh->GetName());
			DismembermentMeshComponents.Add(OwnerSourceMesh);
		}
	}

}

// Called when the game starts
void UDismembermentComponent::BeginPlay()
{
	Super::BeginPlay();

	SkeletalMeshComponent = Cast<USkeletalMeshComponent>(GetOwner()->GetComponentByClass(USkeletalMeshComponent::StaticClass()));

	UpdateLimbMap();
	
}

USkeletalMesh* UDismembermentComponent::GetMesh() const   //执行
{
	return GetSkeletalMeshComponent()->GetSkeletalMeshAsset();
}

USkeletalMeshComponent* UDismembermentComponent::GetSkeletalMeshComponent() const  //执行
{
	if (bOverrideSourceMesh && DismembermentMeshComponents.IsValidIndex(0)) return DismembermentMeshComponents[0];

	return SkeletalMeshComponent;
}

//不执行
TArray<USkeletalMeshComponent*> UDismembermentComponent::GetAllAttachedMeshes() const
{
	TArray<USceneComponent*> Children;
	GetSkeletalMeshComponent()->GetChildrenComponents(true, Children);

	TArray<USkeletalMeshComponent*> ChildMeshes;

	for (USceneComponent* Child : Children)
	{
		if (!Child->IsA(USkeletalMeshComponent::StaticClass())) continue;
		if (Child->ComponentTags.Contains("Dismembered Limb")) continue;

		USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(Child);
		if (Mesh) ChildMeshes.Add(Mesh);
	}

	ChildMeshes.RemoveSwap(GetSkeletalMeshComponent());

	return ChildMeshes;
}

//不执行
TArray<USkeletalMeshComponent*> UDismembermentComponent::GetAllAttachedMeshes(USkeletalMeshComponent* Component) const
{
	TArray<USceneComponent*> Children;
	Component->GetChildrenComponents(true, Children);

	TArray<USkeletalMeshComponent*> ChildMeshes;

	for (USceneComponent* Child : Children)
	{
		if (!Child->IsA(USkeletalMeshComponent::StaticClass())) continue;

		USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(Child);
		if (Mesh) ChildMeshes.Add(Mesh);
	}

	return ChildMeshes;
}

//不执行
TArray<USkeletalMeshComponent*> UDismembermentComponent::GetAllAttachedMeshes(const FName LimbName) const
{
	TArray<USceneComponent*> Children;
	SkeletalMeshComponent->GetChildrenComponents(true, Children);

	TArray<USkeletalMeshComponent*> ChildMeshes;

	for (USceneComponent* Child : Children)
	{
		if (!Child->IsA(USkeletalMeshComponent::StaticClass())) continue;
		// If Attached to a Socket then make sure the socket exist on the limb
		if (Child->GetAttachSocketName() == NAME_None) if (!LimbMap->GetLimb(LimbName).Get().Contains(Child->GetAttachSocketName())) continue;

		USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(Child);
		if (Mesh) ChildMeshes.Add(Mesh);
	}

	return ChildMeshes;
}



TArray<class USkeletalMeshComponent*> UDismembermentComponent::GetAllMeshes() const
{
	if (bOverrideSourceMesh && DismembermentMeshComponents.Num() > 0) return DismembermentMeshComponents;

	if (!bSupportAttachedChildMeshes) return { GetSkeletalMeshComponent() };

	TArray<USkeletalMeshComponent*> Meshes = GetAllAttachedMeshes();

	if (Meshes.Num() < 1) return { GetSkeletalMeshComponent() };

	Meshes.Insert(GetSkeletalMeshComponent(), 0);

	return Meshes;
}

bool UDismembermentComponent::IsLimbMapOutdated() const
{
	if (!LimbMap) return true;
	return LimbMap->Mesh != GetSkeletalMeshComponent()->GetSkeletalMeshAsset();
}

void UDismembermentComponent::UpdateLimbMap()
{
	if (!GetSkeletalMeshComponent()->GetSkeletalMeshAsset())
	{
		LimbMap = nullptr;
		return;
	}

	LimbMap = CreateLimbMap();

	DismemberedVertices.SetNumZeroed(LimbMap->VertexNum);
}

ULimbMap* UDismembermentComponent::CreateLimbMap()
{
	ULimbMap* NewMap = NewObject<ULimbMap>(this, ULimbMap::StaticClass(), NAME_None, RF_Transient);

	NewMap->Mesh = GetMesh();
	NewMap->Initialize();

	return NewMap;
}

void UDismembermentComponent::UpdateMissingLimbs(USkeletalMeshComponent* InDismemberedLimb, FName BoneName)
{
	// 在所有其他操作完成后，将肢体添加到“已丢失肢体”映射表中
	FLimb& Limb = LimbMap->GetLimb(BoneName);

	for (FName Bone : Limb.Get())
	{
		// 这样做可能会出现问题：覆盖掉已经被切断的子肢体
		MissingLimbs.Add(Bone, InDismemberedLimb);
	}
}

bool UDismembermentComponent::CheckBoneFilter(FName BoneName) const
{
	if (ExcludedBones.Contains(BoneName)) return false;
	if (WhitelistBones.Num() > 0 && !WhitelistBones.Contains(BoneName)) return false;

	return true;
}

int32 UDismembermentComponent::GetVertexNum(const USkeletalMeshComponent* InMesh, const int32 LOD) const
{
	return InMesh->GetSkeletalMeshRenderData()->LODRenderData[LOD].GetNumVertices();
}

void UDismembermentComponent::DismemberLimb(FName BoneName, FVector Impulse)
{
	DismemberLimb_Internal(BoneName, Impulse);
}

void UDismembermentComponent::DestroyLimb(const FName BoneName)
{
	/* Scales the Bone and its child bones down to 0 */
	if (bSupportAttachedChildMeshes)
	{
		AllMeshes->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);
	}
	else
	{
		GetSkeletalMeshComponent()->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);
	}
}

void UDismembermentComponent::DismemberLimb_Internal(FName BoneName, const FVector& Impulse)
{
	if (bDisableDismemberment) return;
	if (RemappedBones.Contains(BoneName)) BoneName = *RemappedBones.Find(BoneName);
	if (!CheckBoneFilter(BoneName)) return;
	if (MissingLimbs.Contains(BoneName)) return;
	if (IsLimbMapOutdated()) UpdateLimbMap();

	PreDismemberment(BoneName, Impulse);

	// 在角色的模型上隐藏该肢体
	AllMeshes->HideBoneByName(BoneName, EPhysBodyOp::PBO_Term);

	/* 断开约束，让肢体不再受物理约束影响。如果不断开，会出现奇怪的问题：比如手臂明明已经切掉了，却还像挂在空中、卡住模型。 */
	AllMeshes->BreakConstraint(FVector(0), FVector(0), BoneName);

	if (BoneName != FName("head"))
	{
		/* 为被切断的肢体创建新的骨骼网格组件 */
		USkeletalMeshComponent* DismemberedLimb = CreateDismemberedLimb(BoneName);

		/* 设置动画实例，让骨骼能够正常更新动画 */
		SetDismemberedAnimInstance(DismemberedLimb, BoneName);

		/* 移除所有已经被切断过的肢体 */
		RemovePreviouslyDismemberedLimbs(DismemberedLimb);


		BeginFrameDelayedDismemberment(DismemberedLimb, BoneName, Impulse);
	}


}

void UDismembermentComponent::BeginFrameDelayedDismemberment(USkeletalMeshComponent* Component, FName BoneName, FVector Impulse)
{
	FDismemberedLimbFrameDelay Data;
	Data.SkeletalMeshComponent = Component;
	Data.BoneName = BoneName;
	Data.Impulse = Impulse;

	FrameDelayedDismemberedLimbs.Add(Data);

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UDismembermentComponent::DismemberLimbFrameDelayed);
}

void UDismembermentComponent::DismemberLimbFrameDelayed()
{
	for (FDismemberedLimbFrameDelay& Data : FrameDelayedDismemberedLimbs)
	{
		USkeletalMeshComponent* CachedDismemberedLimb = Data.SkeletalMeshComponent;

		if (!CachedDismemberedLimb) continue;


		CachedDismemberedLimb->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		// 获取当前世界位置并稍微上移
		FVector AdjustedLocation = CachedDismemberedLimb->GetComponentLocation();
		AdjustedLocation.Z += 5.0f;
		FRotator AdjustedRotation = CachedDismemberedLimb->GetComponentRotation();
		AdjustedRotation.Pitch = 0.0f;
		CachedDismemberedLimb->SetWorldLocation(AdjustedLocation);
		CachedDismemberedLimb->SetWorldRotation(AdjustedRotation);

		CachedDismemberedLimb->SetMasterPoseComponent(nullptr);

		CachedDismemberedLimb->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CachedDismemberedLimb->SetCollisionObjectType(ECC_GameTraceChannel1);
		CachedDismemberedLimb->SetCollisionResponseToAllChannels(ECR_Ignore);
		CachedDismemberedLimb->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		CachedDismemberedLimb->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		CachedDismemberedLimb->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		// 阻挡自定义的 GoreTarget 通道
		CachedDismemberedLimb->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);

		CreateDismemberPhysicsAsset(CachedDismemberedLimb, Data.BoneName);


		CachedDismemberedLimb->SetSimulatePhysics(true);
		CachedDismemberedLimb->AddImpulse(Data.Impulse, Data.BoneName, true);
		RemovePreviouslyDismemberedLimbs(CachedDismemberedLimb);
		UpdateMissingLimbs(CachedDismemberedLimb, Data.BoneName);
		PostDismemberment(Data.BoneName, CachedDismemberedLimb); 

	}

	FrameDelayedDismemberedLimbs.Reset();
}

USkeletalMeshComponent* UDismembermentComponent::CreateDismemberedLimb(const FName BoneName)
{
	// 创建新的骨骼网格组件，用于生成断肢
	USkeletalMeshComponent* DisLimb = NewObject<USkeletalMeshComponent>(GetOwner());

	// 给断肢添加标签：“已切断的肢体”
	DisLimb->ComponentTags.Add("Dismembered Limb");

	// 将断肢附加到原角色的对应骨骼上，保持位置、旋转、缩放一致
	//DisLimb->AttachToComponent(GetSkeletalMeshComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale, BoneName); 些许问题

	///////////////////////// fix

	FTransform BoneTransform = GetSkeletalMeshComponent()->GetSocketTransform(BoneName, RTS_World);
	DisLimb->SetWorldTransform(BoneTransform);

	/////////////////////////

	// 注册组件，让断肢在场景中生效
	DisLimb->RegisterComponent();

	// 复制原角色的骨骼网格资源
	DisLimb->SetSkeletalMesh(GetSkeletalMeshComponent()->GetSkeletalMeshAsset());

	// 复制原角色身上所有材质
	for (int32 i = 0; i < GetSkeletalMeshComponent()->GetNumMaterials(); i++)
	{
		DisLimb->SetMaterial(i, GetSkeletalMeshComponent()->GetMaterial(i));
	}

	// 复制是否接收贴花（如血迹、弹痕）
	DisLimb->bReceivesDecals = GetSkeletalMeshComponent()->bReceivesDecals;

	// 复制物理资源（物理碰撞、刚体结构）
	DisLimb->SetPhysicsAsset(GetSkeletalMeshComponent()->GetPhysicsAsset());

	// 设置碰撞数据
	DisLimb->SetCollisionObjectType(LimbCollisionSettings.GetObjectType());
	DisLimb->SetCollisionEnabled(LimbCollisionSettings.GetCollisionEnabled());
	DisLimb->SetCollisionResponseToChannels(LimbCollisionSettings.GetResponseToChannels());

	// 设置是否影响导航网格
	DisLimb->SetCanEverAffectNavigation(CanEverAffectNavigation());

	// 复制顶点颜色到断肢模型
	CopyVertexColorsToMesh(GetSkeletalMeshComponent(), DisLimb);

	// 如果支持复制子物体附件
	if (bSupportAttachedChildMeshes)
	{
		// 为所有附加的网格创建断肢模型
		for (USkeletalMeshComponent* Mesh : GetAllAttachedMeshes())
		{
			// 如果带有“不可切断”标签，直接跳过
			if (Mesh->ComponentTags.Contains("No Dismemberment")) continue;

			// 如果该附件有绑定插槽，但不属于当前肢体，跳过
			if (Mesh->GetAttachSocketName() != NAME_None && !LimbMap->GetLimb(BoneName).Get().Contains(Mesh->GetAttachSocketName())) continue;

			// 如果是需要转移的物品（枪、剑等），不创建新模型，直接转移附件
			if (Mesh->ComponentTags.Contains("Transfer Dismemberment"))
			{
				Mesh->AttachToComponent(DisLimb, FAttachmentTransformRules::SnapToTargetIncludingScale, Mesh->GetAttachSocketName());
				continue;
			}

			// 复制附件网格，用于切断肢体（否则视为武器等独立物品）
			USkeletalMeshComponent* Child = NewObject<USkeletalMeshComponent>(GetOwner());

			// 添加标签
			Child->ComponentTags.Add("Dismembered Limb");

			// 附加到断肢上
			Child->AttachToComponent(DisLimb, FAttachmentTransformRules::SnapToTargetIncludingScale, Mesh->GetAttachSocketName());

			// 注册组件
			Child->RegisterComponent();

			// 复制网格
			Child->SetSkeletalMesh(Mesh->GetSkeletalMeshAsset());

			// 复制材质
			for (int32 i = 0; i < Mesh->GetNumMaterials(); i++)
			{
				Child->SetMaterial(i, Mesh->GetMaterial(i));
			}

			// 复制贴花设置
			Child->bReceivesDecals = Mesh->bReceivesDecals;

			// 设置主姿势组件，让附件跟着断肢动画同步
			Child->SetLeaderPoseComponent(DisLimb);

			// 复制顶点颜色
			CopyVertexColorsToMesh(Mesh, Child);
		}
	}

	// 返回创建好的断肢组件
	return DisLimb;
}

void UDismembermentComponent::SetDismemberedAnimInstance(USkeletalMeshComponent* Component, FName BoneName) const
{
	Component->SetAnimInstanceClass(DismemberedAnimInstance);
	UDismemberedAnimInstance* AnimInstance = Cast<UDismemberedAnimInstance>(Component->GetAnimInstance());
	if (AnimInstance) AnimInstance->Limb = BoneName;
}

void UDismembermentComponent::RemovePreviouslyDismemberedLimbs(USkeletalMeshComponent* Component)
{
/*
 *  隐藏已经被切断的肢体。
 *  例如：先切断手掌，再切断手臂。
 *  新生成的手臂模型上，不应该再显示已经被切掉的手掌。
 */

	for (const TTuple<FName, USkeletalMeshComponent*>& Child : MissingLimbs)
	{

	
		Component->HideBoneByName(Child.Key, EPhysBodyOp::PBO_Term);

	}



}

void UDismembermentComponent::CreateDismemberPhysicsAsset(USkeletalMeshComponent* Component, FName InLimb) const
{
	UPhysicsAsset* NewPhysicsAsset = DuplicateObject<UPhysicsAsset>(Component->GetPhysicsAsset(), Component);

	TArray<FName> BoneNames;
	Component->GetBoneNames(BoneNames);
	for (const FName Bone : BoneNames)
	{
		const int32 Index = NewPhysicsAsset->FindBodyIndex(Bone);

		if (Index == INDEX_NONE) continue;
		if (LimbMap->GetLimb(InLimb).Get().Contains(Bone)) continue;

		TerminatePhysicsBodies(NewPhysicsAsset, Index); // <--- From: FPhysicsAssetUtils::DestroyBody(NewPhysicsAsset, Index);
	}

	Component->SetPhysicsAsset(NewPhysicsAsset, true);
}

void UDismembermentComponent::TerminatePhysicsBodies(UPhysicsAsset* PhysicsAsset, int32 Index) const
{
	// 安全检查：确保物理资源有效（为空则直接崩溃报错）
	check(PhysicsAsset);

// 首先必须修正碰撞禁用表（CollisionDisableTable）
// 移除所有引用了目标物理体索引的元素
// 调整所有引用了大于目标索引的物理体元素（索引减1）

	TMap<FRigidBodyIndexPair, bool> NewCDT;
	// 遍历所有物理体之间的碰撞关系
	for (int32 i = 1; i < PhysicsAsset->SkeletalBodySetups.Num(); i++)
	{
		for (int32 j = 0; j < i; j++)
		{
			FRigidBodyIndexPair Key(j, i);

			// 如果碰撞表中存在这组物理体的关系，并且不涉及要删除的物理体
			if (PhysicsAsset->CollisionDisableTable.Find(Key))
			{
				if (i != Index && j != Index)
				{
					// 调整索引：大于被删除索引的数值减1
					int32 NewI = (i > Index) ? i - 1 : i;
					int32 NewJ = (j > Index) ? j - 1 : j;

					FRigidBodyIndexPair NewKey(NewJ, NewI);
					NewCDT.Add(NewKey, 0);
				}
			}
		}
	}

	// 用修复后的新碰撞禁用表替换旧表
	PhysicsAsset->CollisionDisableTable = NewCDT;

	// 移除所有绑定在这个物理体上的约束（关节/物理连接）
	// 这个过程比较繁琐且性能消耗稍高
	TArray<int32> Constraints;
	PhysicsAsset->BodyFindConstraints(Index, Constraints);

	// 循环删除所有关联的约束
	while (Constraints.Num() > 0)
	{
		PhysicsAsset->ConstraintSetup.RemoveAt(Constraints[0]);
		// 重新查找，确保全部删除干净
		PhysicsAsset->BodyFindConstraints(Index, Constraints);
	}

	// 从数组中移除指定的物理体设置（对象会被引擎自动垃圾回收）
	PhysicsAsset->SkeletalBodySetups.RemoveAt(Index);

	// 更新物理体索引映射表
	PhysicsAsset->UpdateBodySetupIndexMap();
	// 更新边界与物理体数组，修复所有索引
	PhysicsAsset->UpdateBoundsBodiesArray();


}

void UDismembermentComponent::PreDismemberment(const FName BoneName, FVector Impulse)
{
	OnPreDismemberment.Broadcast(BoneName, Impulse);
}

void UDismembermentComponent::PostDismemberment(const FName BoneName, USkeletalMeshComponent* DismemberMesh)
{
	OnPostDismemberment.Broadcast(BoneName, DismemberMesh);
}

FLinearColor UDismembermentComponent::GetCurrentVertexColor(const USkeletalMeshComponent* Mesh, const int32 VertexIndex, const int32 LOD) const
{
	// 如果没有模型或没有颜色顶点缓冲区，则返回失败（默认白色）
	FColor FallbackColor = FColor(255, 255, 255, 255);
	if (!Mesh->GetSkeletalMeshAsset() || !Mesh->MeshObject)
	{
		return FallbackColor.ReinterpretAsLinear();
	}

	// 如果模型有自定义覆盖顶点颜色，则直接使用该颜色
	if (Mesh->LODInfo.Num() > 0 &&
		Mesh->LODInfo.IsValidIndex(LOD) &&
		Mesh->LODInfo[LOD].OverrideVertexColors != nullptr &&
		Mesh->LODInfo[LOD].OverrideVertexColors->IsInitialized() &&
		VertexIndex < (int32)Mesh->LODInfo[LOD].OverrideVertexColors->GetNumVertices())
	{
		return Mesh->LODInfo[LOD].OverrideVertexColors->VertexColor(VertexIndex).ReinterpretAsLinear();
	}

	// 获取当前LOD层级的渲染数据
	FSkeletalMeshLODRenderData& LODData = Mesh->MeshObject->GetSkeletalMeshRenderData().LODRenderData[LOD];

	// 如果顶点颜色缓冲区未初始化，返回默认白色
	if (!LODData.StaticVertexBuffers.ColorVertexBuffer.IsInitialized())
	{
		return FallbackColor.ReinterpretAsLinear();
	}

	// 通过顶点索引，找到该顶点所属的渲染区域（Section）和区域内的相对索引
	int32 SectionIndex;
	int32 VertIndex;
	LODData.GetSectionFromVertexIndex(VertexIndex, SectionIndex, VertIndex);

	// 安全检查：确保Section索引有效
	check(SectionIndex < LODData.RenderSections.Num());
	const FSkelMeshRenderSection& Section = LODData.RenderSections[SectionIndex];

	// 获取该Section的起始顶点偏移
	int32 VertexBase = Section.BaseVertexIndex;

	// 最终返回模型上该顶点的真实颜色
	return LODData.StaticVertexBuffers.ColorVertexBuffer.VertexColor(VertexBase + VertIndex).ReinterpretAsLinear();
}

TArray<FLinearColor> UDismembermentComponent::GetCurrentVertexColors(const USkeletalMeshComponent* Mesh, const int32 LODIndex) const
{
	TArray<FLinearColor> Out;
	Out.SetNumUninitialized(GetVertexNum(Mesh, LODIndex));

	for (int32 i = 0; i < GetVertexNum(Mesh, LODIndex); i++)
	{
		Out[i] = GetCurrentVertexColor(Mesh, i, LODIndex);
	}

	return Out;
}

void UDismembermentComponent::CopyVertexColorsToMesh(const USkeletalMeshComponent* FromMesh, USkeletalMeshComponent* ToMesh)
{
	if (!FromMesh || !ToMesh || !FromMesh->GetSkeletalMeshRenderData()) return;

	int32 NumLODs = FromMesh->GetSkeletalMeshRenderData()->LODRenderData.Num();

	for (int32 LOD = 0; LOD < NumLODs; ++LOD)
	{
		TArray<FLinearColor> Colors = GetCurrentVertexColors(FromMesh, LOD);

		ToMesh->SetVertexColorOverride_LinearColor(LOD, Colors);
	}
}

void UDismembermentComponent::SetColorOfChannel(FLinearColor& Color, float Value, EDismemberColorChannel Channel)
{
	switch (Channel)
	{
	case EDismemberColorChannel::R_Channel:
	{
		Color.R = Value;
		return;
	}
	case EDismemberColorChannel::G_Channel:
	{
		Color.G = Value;
		return;
	}
	case EDismemberColorChannel::B_Channel:
	{
		Color.B = Value;
		return;
	}
	case EDismemberColorChannel::A_Channel:
	{
		Color.A = Value;
		return;
	}
	}
}

void UDismembermentComponent::MaxColorOfChannel(FLinearColor& Color, float Value, EDismemberColorChannel Channel)
{
	switch (Channel)
	{
	case EDismemberColorChannel::R_Channel:
	{
		Color.R = FMath::Max(Color.R, Value);
		return;
	}
	case EDismemberColorChannel::G_Channel:
	{
		Color.G = FMath::Max(Color.G, Value);
		return;
	}
	case EDismemberColorChannel::B_Channel:
	{
		Color.B = FMath::Max(Color.B, Value);
		return;
	}
	case EDismemberColorChannel::A_Channel:
	{
		Color.A = FMath::Max(Color.A, Value);
		return;
	}
	}
}

void UDismembermentComponent::SetLinearColorChannel(TArray<FLinearColor>& Colors, float Value, EDismemberColorChannel Channel)
{
	for (int32 i = 0; i < Colors.Num(); i++)
	{
		SetColorOfChannel(Colors[i], Value, Channel);
	}
}

void UDismembermentComponent::MaxLinearColorChannel(TArray<FLinearColor>& Colors, TArray<float>& Values, const EDismemberColorChannel Channel)  //执行
{
	for (int32 i = 0; i < Colors.Num(); i++)
	{
		MaxColorOfChannel(Colors[i], Values[i], Channel);
	}
}