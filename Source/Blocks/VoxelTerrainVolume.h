// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "RuntimeMeshComponent.h"
#include "VoxelObject.h"
#include "VoxelTerrainVolume.generated.h"

USTRUCT()
struct FDecodedMesh
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() TArray<FVector> Vertices;
	UPROPERTY() TArray<int32> Indices;
	UPROPERTY() TArray<FVector> Normals;
	UPROPERTY() TArray<FVector2D> UV0;
	UPROPERTY() TArray<FColor> Colors;
	UPROPERTY() TArray<FRuntimeMeshTangent> Tangents;
};

USTRUCT()
struct FObjectVoxel
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() FVector WorldLocation;
	UPROPERTY() TSubclassOf<AVoxelObject> Class;
	UPROPERTY() uint32 Data;
};

USTRUCT(BlueprintType)
struct FSerializedBObj
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere) FTransform Transform;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) UClass *Class;
};


UCLASS(abstract)
class BLOCKS_API UVoxelTerrainVolume : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UVoxelTerrainVolume();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	//Raycast in direction from origin. Direction must contain length.
	virtual bool Raycast(FVector Origin, FVector Direction, FVector& VoxelLocation) {
		return false;
	};

	virtual bool RaycastBlocksOnly(FVector Origin, FVector Direction, FVector& voxelLocation) {
		return false;
	}

	//Raycast in direction from origin and return the voxel before the hit voxel. Direction must contain length.
	virtual bool RaycastPrevious(FVector Origin, FVector Direction, FVector& VoxelLocation) {
		return false;
	};

	//Add block at the given vector (in world coordinates)
	virtual bool AddBlock(FVector BlockPosition, uint8 Material, uint8 DataBits) {
		return false;
	};

	//Remove block from the given vector (in world coordinates)
	virtual bool RemoveBlock(FVector BlockPosition) {
		return false;
	};

	//Find the material of the block at the given vector (in world coordinates)
	virtual int32 GetBlockMaterial(FVector BlockPosition) {
		return 0;
	};

	virtual FDecodedMesh* ExtractMesh(FVector Origin) { return nullptr; };

	virtual TArray<FObjectVoxel> SpawnObjects(FVector Origin) { TArray<FObjectVoxel> Something; return Something; };

	virtual FVector WorldSpaceToVoxelSpace(FVector WorldSpace) {
		return FVector(10,10,10);
	};
	virtual FVector VoxelSpaceToWorldSpace(FVector VoxelSpace) {
		return FVector(10, 10, 10);
	};
	virtual FVector GetWorldChunkLocationFromWorldSpace(FVector WorldSpace) {
		return FVector(10, 10, 10);
	};
	virtual FVector GetVoxelChunkLocationFromVoxelSpace(FVector VoxelSpace) {
		return FVector(10, 10, 10);
	};

	virtual void Test() { };

	UPROPERTY(EditAnywhere) UMaterialInterface* TerrainMaterial;
	
};