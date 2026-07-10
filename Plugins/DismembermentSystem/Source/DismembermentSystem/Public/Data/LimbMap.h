// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "LimbMap.generated.h"


USTRUCT(BlueprintType)
struct FLimb
{
	GENERATED_BODY()

	FLimb() {}
	FLimb(const FName& InBone, const TArray<FName>& InLimb) : BoneName(InBone), Limb(InLimb) {}
	FLimb(const int32 InIndex, const TArray<int32>& InChildBones) : BoneIndex(InIndex), ChildBones(InChildBones) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LimbMap")
	FName BoneName = NAME_None;  // 主骨骼名（如 arm_r）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LimbMap")
	TArray<FName> Limb;        // 这个肢体包含的所有子骨头

	UPROPERTY(VisibleAnywhere, Category = "LimbMap")
	int32 BoneIndex = -1;       // 骨骼索引

	UPROPERTY(VisibleAnywhere, Category = "LimbMap")
	TArray<int32> ChildBones;        // 骨骼索引

	UPROPERTY(VisibleAnywhere, Category = "LimbMap")
	FTransform ReferenceBoneTransform;

	TArray<FName>& Get() { return Limb; }

};



USTRUCT(BlueprintType)
struct FLimbKey
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "LimbMap")
	TArray<FLimb> Limbs;

	void Add(const FLimb& InLimb) { Limbs.Add(InLimb); }

	void Empty() { Limbs.Empty(); }

	FLimb& Find(const int32& InIndex) {return Limbs[InIndex];}

	FLimb& Find(const FName& InName) 
	{
		for (FLimb& Limb : Limbs)
		{
			if (Limb.BoneName == InName) return Limb;
		}

		// 只要骨骼名称（Bone Name）是有效的，这段代码永远不会执行到。
		UE_LOG(LogTemp, Error, TEXT("Bone Name is Invalid when searching for Limb"))
		return Limbs[0];
	}
};



/*
    Limb Map 是肢解系统中，为每个骨骼网格体（角色模型）所使用的数据结构
 */



UCLASS(BlueprintType, Blueprintable)
class DISMEMBERMENTSYSTEM_API ULimbMap : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Skeleton", BlueprintReadWrite)
	USkeletalMesh* Mesh;

	UPROPERTY(EditAnywhere, Category = "Skeleton", meta = (ToolTip = "Bone containing these Key Words will be excluded from the Limbs. Usually useful for 'ik' or 'twist' bones."))
	TArray<FString> Exclude;

	bool HasExcludedKeyword(const FString& String);  //排除不被肢解的骨骼Bone


/*
	Cached Data
*/

	UPROPERTY(VisibleAnywhere, Category = "LimbMap")
	int32 VertexNum;    // 模型总顶点数

	//因为虚幻引擎会把骨骼映射关系，按材质区段分开处理。
	UPROPERTY(VisibleAnywhere, Category = "LimbMap")
	TMap<int32, int32> VertexBoneMap;   // 顶点 → 对应骨头


/*
 	Limb Map
 */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LimbMap")
	FLimbKey LimbMap;

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "LimbMap")
	void Initialize();


	//这是一个递归函数，用于获取指定骨骼索引下的所有子骨骼。
	void GetAllChildBones(const int32 Index, TArray<int32>& ChildBones);
	void GetDirectChildBones(const FReferenceSkeleton& ReferenceSkeleton, int32 ParentBoneIndex, TArray<int32>& ChildBones) const;


	FTransform GetBoneReferenceTransform(const FName BoneName) const;

	/* 从数组中移除所有包含排除关键词的骨骼 */
	void RemoveExcludedBones(TArray<int32>& ChildBones);
	/* 从数组中移除指定索引的元素，直到该索引不再包含排除关键词为止*/
	void TryRemoveExcludedBone(const int32& Index, TArray<int32>& ChildBones);

	UFUNCTION(BlueprintPure, Category = "LimbMap")
	TArray<FName>& GetAllBonesOfLimb(ULimbMap* InLimbMap, const FName BoneName);

	UFUNCTION(BlueprintPure, Category = "LimbMap")
	FName GetBoneName(const int32 Index);

	UFUNCTION(BlueprintPure, Category = "LimbMap")
	int32 GetBoneIndex(const FName& InName);

	UFUNCTION(BlueprintPure, Category = "LimbMap")
	FLimb& GetLimb(const FName BoneName);

};