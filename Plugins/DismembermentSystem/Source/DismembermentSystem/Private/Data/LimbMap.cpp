// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/LimbMap.h"

#include "Rendering/SkeletalMeshRenderData.h"

bool ULimbMap::HasExcludedKeyword(const FString& String)
{
	for (FString& Key : Exclude)
	{
		if (String.Contains(Key)) return true;
	}

	return false;
}

void ULimbMap::Initialize()
{
	Modify();
	LimbMap.Empty();

	if (!Mesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot Initialize Limb Map. Skeletal Mesh is Invalid!"))
		return;
	}

	// 先构建一个临时数组，然后将它重新映射回骨骼网格体
	// 因为在获取子骨骼时，骨骼网格体本身会出现异常/奇怪的行为
	const FReferenceSkeleton& RefSkel = Mesh->GetSkeleton()->GetReferenceSkeleton();
	const TArray<FVirtualBone>& VirtualBones = Mesh->GetSkeleton()->GetVirtualBones();

	TArray<FName> VirtualBoneNames;
	for (const FVirtualBone& VirtualBone : VirtualBones) VirtualBoneNames.Add(VirtualBone.VirtualBoneName);

	// 为每一根骨头都创建一个 FLimb（肢体 / 骨骼数据）
	for (int32 i = 0; i < RefSkel.GetNum(); i++)
	{
		if (VirtualBoneNames.Contains(RefSkel.GetBoneName(i))) continue;

		TArray<int32> ChildBones;
		GetAllChildBones(i, ChildBones);
		RemoveExcludedBones(ChildBones);

		FLimb Limb(i, ChildBones);

		Limb.BoneName = RefSkel.GetBoneName(i);
		Limb.Limb.Add(Limb.BoneName);

		for (const int32& Child : ChildBones)
		{
			Limb.Limb.Add(RefSkel.GetBoneName(Child));
		}

		Limb.ReferenceBoneTransform = GetBoneReferenceTransform(Limb.BoneName);

		LimbMap.Add(Limb);

	}

	// 把 FLimb 肢体数据里的骨骼索引，重新映射对齐到 骨骼网格体（Skeletal Mesh）的真实骨骼索引。

	for (FLimb& Limb : LimbMap.Limbs)
	{
		TArray<int32> Bones;
		Limb.BoneIndex = GetBoneIndex(Limb.BoneName);
		for (FName Name : Limb.Limb)
		{
			Bones.Add(GetBoneIndex(Name));
		}

		Limb.ChildBones = Bones;
	}

	// 设置顶点的数量
	VertexNum = Mesh->GetResourceForRendering()->LODRenderData[0].GetNumVertices();

	// 创建完整的骨骼映射表
	VertexBoneMap.Empty();
	FSkeletalMeshLODRenderData& LodModel = Mesh->GetResourceForRendering()->LODRenderData[0];

	for (int32 i = 0; i < LodModel.RenderSections.Num(); i++)
	{
		FSkelMeshRenderSection& Section = LodModel.RenderSections[i];

		for (int32 j = 0; j < Section.BoneMap.Num(); j++)
		{
			VertexBoneMap.Add(j, Section.BoneMap[j]);
		}
	}

}

void ULimbMap::GetAllChildBones(const int32 Index, TArray<int32>& ChildBones)
{
	TArray<int32> CurrentChildBones;

	const FReferenceSkeleton& RefSkeleton = Mesh->GetSkeleton()->GetReferenceSkeleton();
	GetDirectChildBones(RefSkeleton, Index, CurrentChildBones);

	if (CurrentChildBones.Num() <= 0) return;

	ChildBones.Append(CurrentChildBones);

	for (const int32& Child : CurrentChildBones)
	{
		GetAllChildBones(Child, ChildBones);
	}
}

void ULimbMap::GetDirectChildBones(const FReferenceSkeleton& ReferenceSkeleton, int32 ParentBoneIndex, TArray<int32>& ChildBones) const
{
	ChildBones.Reset();

	const int32 NumBones = ReferenceSkeleton.GetNum();
	for (int32 ChildIndex = ParentBoneIndex + 1; ChildIndex < NumBones; ChildIndex++)
	{
		if (ParentBoneIndex == ReferenceSkeleton.GetParentIndex(ChildIndex))
		{
			ChildBones.Add(ChildIndex);
		}
	}

}

FTransform ULimbMap::GetBoneReferenceTransform(const FName BoneName) const
{
	return FTransform(Mesh->GetComposedRefPoseMatrix(BoneName));
}

void ULimbMap::RemoveExcludedBones(TArray<int32>& ChildBones)
{
	for (int32 i = 0; i < ChildBones.Num(); i++)
	{
		TryRemoveExcludedBone(i, ChildBones);
	}
}

void ULimbMap::TryRemoveExcludedBone(const int32& Index, TArray<int32>& ChildBones)
{
	if (!ChildBones.IsValidIndex(Index)) return;

	const FString BoneName = Mesh->GetSkeleton()->GetReferenceSkeleton().GetBoneName(ChildBones[Index]).ToString();

	if (HasExcludedKeyword(BoneName))
	{
		ChildBones.RemoveAt(Index);
		TryRemoveExcludedBone(Index, ChildBones);
	}
}

TArray<FName>& ULimbMap::GetAllBonesOfLimb(ULimbMap* InLimbMap, const FName BoneName)
{
	return InLimbMap->LimbMap.Find(BoneName).Get();
}

FName ULimbMap::GetBoneName(const int32 Index)
{
	return Mesh->GetRefSkeleton().GetBoneName(Index);
}

int32 ULimbMap::GetBoneIndex(const FName& InName)
{
	return Mesh->GetRefSkeleton().FindBoneIndex(InName);
}

FLimb& ULimbMap::GetLimb(const FName BoneName)
{
	return LimbMap.Find(BoneName);
}
