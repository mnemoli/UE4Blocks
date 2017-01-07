// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
// Polyvox Includes
#include "RuntimeMeshComponent.h"
#include "PolyVoxVolume.h"
#include "GameFramework/Actor.h"
#include "AVoxelTerrainActor.generated.h"

UCLASS()
class BLOCKS_API AVoxelTerrainActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVoxelTerrainActor();

	// Called after the C++ constructor and after the properties have been initialized.
	virtual void PostInitializeComponents() override;

	// Called when the actor has begun playing in the level
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	//Raycast in direction from origin. Direction must contain length.
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain") bool Raycast(FVector Origin, FVector Direction, FVector& VoxelLocation) {
		return VoxelVolume->Raycast(Origin, Direction, VoxelLocation);
	};

	//Raycast in direction from origin and return the voxel before the hit voxel. Direction must contain length.
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain") bool RaycastPrevious(FVector Origin, FVector Direction, FVector& VoxelLocation) {
		return VoxelVolume->RaycastPrevious(Origin, Direction, VoxelLocation);
	};

	//Add block at the given vector (in world coordinates)
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain") bool AddBlock(FVector BlockPosition, uint8 Material) {
		return VoxelVolume->AddBlock(BlockPosition, Material);
	};

	//Remove block from the given vector (in world coordinates)
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain") bool RemoveBlock(FVector BlockPosition) {
		return VoxelVolume->RemoveBlock(BlockPosition);
	};

	//Find the material of the block at the given vector (in world coordinates)
	UFUNCTION(BlueprintCallable, Category = "Voxel Terrain") int32 GetBlockMaterial(FVector BlockPosition) {
		return VoxelVolume->GetBlockMaterial(BlockPosition);
	};

	// The procedurally generated mesh that represents our voxels
	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, VisibleAnywhere) class URuntimeMeshComponent* TerrainMesh;

	//The voxel volume
	UPROPERTY(Category = "Voxel Terrain", VisibleAnywhere) class UPolyVoxVolume* VoxelVolume;
};