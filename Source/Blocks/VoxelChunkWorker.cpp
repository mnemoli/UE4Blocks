// Fill out your copyright notice in the Description page of Project Settings.

#include "Blocks.h"
#include "VoxelChunkWorker.h"

void FVoxelChunkWorker::DoWork()
{
	Chunk->ExtractMesh(VoxelVolume);
}