// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "VoxelChunk.h"
#include "VoxelTerrainVolume.h"
#include "RuntimeMeshComponent.h"

/**
 * 
 */
class BLOCKS_API FVoxelChunkWorker : public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FVoxelChunkWorker>;
public:
	FVoxelChunkWorker(UVoxelChunk* TheChunk, UVoxelTerrainVolume* TheVoxelVolume)
		: Chunk(TheChunk)
		, VoxelVolume(TheVoxelVolume)
	{}
protected:

	UPROPERTY() UVoxelChunk* Chunk;
	UPROPERTY() UVoxelTerrainVolume* VoxelVolume;

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FVoxelChunkWorker, STATGROUP_ThreadPoolAsyncTasks);
	}

	void DoWork();

};
