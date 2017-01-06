// Fill out your copyright notice in the Description page of Project Settings.

#include "Blocks.h"
#include "VoxelTerrainPager.h"
using namespace PolyVox;
#include "VM/kernel.h"
using namespace anl;

VoxelTerrainPager::VoxelTerrainPager(uint32 NoiseSeed, uint32 Octaves, float Frequency, float Scale, float Offset, float Height)
	: PagedVolume<MaterialDensityPair44>::Pager(), Seed(NoiseSeed), NoiseOctaves(Octaves), NoiseFrequency(Frequency), NoiseScale(Scale), NoiseOffset(Offset), TerrainHeight(Height)
{

}

// Called when a new chunk is paged in
// This function will automatically generate our voxel-based terrain from simplex noise
void VoxelTerrainPager::pageIn(const PolyVox::Region& region, PagedVolume<MaterialDensityPair44>::Chunk* Chunk)
{
	auto RegionCentre = region.getCentre();
	FString RegionString;

	RegionString.AppendInt(RegionCentre.getX());
	RegionString.AppendChar(' ');
	RegionString.AppendInt(RegionCentre.getY());
	RegionString.AppendChar(' ');
	RegionString.AppendInt(RegionCentre.getZ());

	auto HashedRegionString = FMD5::HashAnsiString(*RegionString);

	FString SaveDirectory = FString("F:/Jade/Documents/Unreal Projects/Blocks/Chunks");

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	FString AbsoluteFilePath = SaveDirectory + "/" + HashedRegionString;

	TArray<uint8> VoxelArray;

	if (PlatformFile.FileExists(*AbsoluteFilePath))
	{
		FFileHelper::LoadFileToArray(VoxelArray, *AbsoluteFilePath);
		int i = 0;

		auto It = VoxelArray.CreateConstIterator();

		for (int x = region.getLowerX(); x <= region.getUpperX(); x++)
		{
			for (int y = region.getLowerY(); y <= region.getUpperY(); y++)
			{
				for (int z = region.getLowerZ(); z <= region.getUpperZ(); z++) {
					MaterialDensityPair44 Voxel;
					Voxel.setMaterial(*It);
					It++;
					auto density = *It;
					if (density == 0)
					{
						density = 255;
					}
					Voxel.setDensity(density);
					It++;
					Chunk->setVoxel(x - region.getLowerX(), y - region.getLowerY(), z - region.getLowerZ(), Voxel);
					
				}
			}
		}

	}
	else //evaluate region new with noise
	{
		// This is our kernel. It is responsible for generating our noise.
		CKernel NoiseKernel;

		// Commonly used constants
		auto Zero = NoiseKernel.constant(0);
		auto One = NoiseKernel.constant(1);
		auto VerticalHeight = NoiseKernel.constant(TerrainHeight);

		// Create a gradient on the vertical axis to form our ground plane.
		auto VerticalGradient = NoiseKernel.divide(NoiseKernel.clamp(NoiseKernel.subtract(VerticalHeight, NoiseKernel.z()), Zero, VerticalHeight), VerticalHeight);

		// Turn our gradient into two solids that represent the ground and air. This prevents floating terrain from forming later.
		auto VerticalSelect = NoiseKernel.select(Zero, One, VerticalGradient, NoiseKernel.constant(0.5), Zero);

		// This is the actual noise generator we'll be using.
		// In this case I've gone with a simple fBm generator, which will create terrain that looks like smooth, rolling hills.
		auto TerrainFractal = NoiseKernel.simplefBm(BasisTypes::BASIS_SIMPLEX, InterpolationTypes::INTERP_LINEAR, NoiseOctaves, NoiseFrequency, Seed);

		// Scale and offset the generated noise value. 
		// Scaling the noise makes the features bigger or smaller, and offsetting it will move the terrain up and down.
		auto TerrainScale = NoiseKernel.scaleOffset(TerrainFractal, NoiseScale, NoiseOffset);

		// Setting the Z scale of the fractal to 0 will effectively turn the fractal into a heightmap.
		auto TerrainZScale = NoiseKernel.scaleZ(TerrainScale, Zero);

		// Finally, apply the Z offset we just calculated from the fractal to our ground plane.
		auto PerturbGradient = NoiseKernel.translateZ(VerticalSelect, TerrainZScale);

		CNoiseExecutor TerrainExecutor(NoiseKernel);

		// Now that we have our noise setup, let's loop over our chunk and apply it.
		for (int x = region.getLowerX(); x <= region.getUpperX(); x++)
		{
			for (int y = region.getLowerY(); y <= region.getUpperY(); y++)
			{
				for (int z = region.getLowerZ(); z <= region.getUpperZ(); z++) { // Evaluate the noise

					auto EvaluatedNoise = TerrainExecutor.evaluateScalar(x, y, z, PerturbGradient);
					MaterialDensityPair44 Voxel;
					bool bSolid = EvaluatedNoise > 0.5;

					Voxel.setDensity(bSolid ? 255 : 0);
					Voxel.setMaterial(bSolid ? 1 : 0);

					// Voxel position within a chunk always start from zero. So if a chunk represents region (4, 8, 12) to (11, 19, 15)
					// then the valid chunk voxels are from (0, 0, 0) to (7, 11, 3). Hence we subtract the lower corner position of the
					// region from the volume space position in order to get the chunk space position.
					Chunk->setVoxel(x - region.getLowerX(), y - region.getLowerY(), z - region.getLowerZ(), Voxel);
				}
			}
		}

	}
	
}

// Called when a chunk is paged out
void VoxelTerrainPager::pageOut(const PolyVox::Region& region, PagedVolume<MaterialDensityPair44>::Chunk* Chunk)
{
	auto RegionCentre = region.getCentre();
	FString RegionString;

	RegionString.AppendInt(RegionCentre.getX());
	RegionString.AppendChar(' ');
	RegionString.AppendInt(RegionCentre.getY());
	RegionString.AppendChar(' ');
	RegionString.AppendInt(RegionCentre.getZ());

	auto HashedRegionString = FMD5::HashAnsiString(*RegionString);

	FString SaveDirectory = FString("F:/Jade/Documents/Unreal Projects/Blocks/Chunks");

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
	{
		
		FString AbsoluteFilePath = SaveDirectory + "/" + HashedRegionString;

		FString RegionData;
		// write basic region data to file
		for (int x = region.getLowerX(); x <= region.getUpperX(); x++)
		{
			for (int y = region.getLowerY(); y <= region.getUpperY(); y++)
			{
				for (int z = region.getLowerZ(); z <= region.getUpperZ(); z++) {
					auto Voxel = Chunk->getVoxel(x - region.getLowerX(), y - region.getLowerY(), z - region.getLowerZ());
					RegionData.AppendInt(Voxel.getMaterial());
					auto density = Voxel.getDensity();
					if (density > 1)
					{
						density = 1;
					}
					RegionData.AppendInt(density);
				}
			}
		}

		FFileHelper::SaveStringToFile(RegionData, *AbsoluteFilePath);
	}
}