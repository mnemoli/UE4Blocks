// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "VoxelTerrainVolume.h"
#include "RuntimeMeshComponent.h"
#include "VoxelChunk.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EChunkState : uint8
{
	Uninitted UMETA(DisplayName = "Uninitalised"),
	Generating UMETA(DisplayName = "Generating"),
	Generated UMETA(DisplayName = "Generated"),
	Rendered UMETA(DisplayName = "Rendered"),
	HasCollision UMETA(DisplayName = "Colliding")
};

UCLASS()
class BLOCKS_API UVoxelChunk : public UObject
{
	GENERATED_BODY()
	
public:
	void ExtractMesh(UVoxelTerrainVolume* VoxelVolume);
	void RenderMesh(URuntimeMeshComponent* TerrainMesh, UMaterialInterface* Material);
	void GenerateMeshCollision(URuntimeMeshComponent* TerrainMesh);
	void Initialize(FVector InOrigin, uint32 InMeshSection) { Origin = InOrigin; MeshSection = InMeshSection; }
	void Invalidate() { ChunkState = EChunkState::Uninitted; UrgentUpdate = true; }
	bool IsReadyToRender();
	void SetRendering();
	bool CheckNeedsToRender();
	bool CheckNeedsCollision();
	uint8 GetMeshSection();
	UPROPERTY() bool UrgentUpdate;
private:
	UPROPERTY() FVector Origin;
	UPROPERTY() EChunkState ChunkState;
	UPROPERTY() uint32 MeshSection;
	TUniquePtr<FDecodedMesh> Mesh;
};
