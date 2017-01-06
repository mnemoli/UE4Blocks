// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "VoxelChunk.h"
#include "VoxelChunkManager.generated.h"

/**
 * 
 */
UCLASS()
class BLOCKS_API UVoxelChunkManager : public UObject
{
	GENERATED_BODY()

public:
	void UpdateChunks(FVector PlayerPos);
	void SetupTest();
	void Tick(float DeltaTime);
	void InitalizeChunkManager();
	void InvalidateChunk(FVector VoxelPosition);

private:
	UPROPERTY() TMap<FString, UVoxelChunk*> ChunkMap;
	UPROPERTY() TArray<bool> MeshSectionQueue;
	UPROPERTY() UVoxelTerrainVolume* VoxelVolume;

	void LoadChunks();
	
};
