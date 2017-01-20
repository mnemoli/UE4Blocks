// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
// Polyvox Includes
#include "PolyVox/PagedVolume.h"
#include "BlocksVoxelType.h"
/**
 * 
 */
class VoxelTerrainPager : public PolyVox::PagedVolume<CurBlocksVoxelType>::Pager
{
public:
	// Constructor
	VoxelTerrainPager(uint32 NoiseSeed = 123, uint32 Octaves = 3, float Frequency = 0.01, float Scale = 32, float Offset = 0, float Height = 64);

	// Destructor
	virtual ~VoxelTerrainPager() {};

	// PagedVolume::Pager functions
	virtual void pageIn(const PolyVox::Region& region, PolyVox::PagedVolume<CurBlocksVoxelType>::Chunk* pChunk);
	virtual void pageOut(const PolyVox::Region& region, PolyVox::PagedVolume<CurBlocksVoxelType>::Chunk* pChunk);

private:
	// Some variables to control our terrain generator
	// The seed of our fractal
	uint32 Seed = 123;

	// The number of octaves that the noise generator will use
	uint32 NoiseOctaves = 3;

	// The frequency of the noise
	float NoiseFrequency = 0.01;

	// The scale of the noise. The output of the TerrainFractal is multiplied by this.
	float NoiseScale = 32;

	// The offset of the noise. This value is added to the output of the TerrainFractal.
	float NoiseOffset = 0;

	// The maximum height of the generated terrain in voxels. NOTE: Changing this will affect where the ground begins!
	float TerrainHeight = 64;
};