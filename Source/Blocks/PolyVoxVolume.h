// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "VoxelTerrainVolume.h"
#include "PolyVox/PagedVolume.h"
#include "BlocksVoxelType.h"
#include "VoxelChunkManager.h"
#include "PolyVox/CubicSurfaceExtractor.h"
#include "PolyVox/Mesh.h"
#include "PolyVox/Picking.h"
#include "WindowsPlatformProcess.h"
#include "PolyVoxVolume.generated.h"

/**
 * 
 */
UCLASS()
class BLOCKS_API UPolyVoxVolume : public UVoxelTerrainVolume
{
public:
	GENERATED_BODY()
	UPolyVoxVolume();
	~UPolyVoxVolume();

	virtual FDecodedMesh* ExtractMesh(FVector Origin) override;
	virtual TArray<FObjectVoxel> SpawnObjects(FVector Origin) override;

	virtual void Test() override;

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual bool Raycast(FVector Origin, FVector Direction, FVector& VoxelLocation) override;
	virtual bool RaycastBlocksOnly(FVector Origin, FVector Direction, FVector & voxelLocation) override;
	virtual bool RaycastPrevious(FVector Origin, FVector Direction, FVector& VoxelLocation) override;
	virtual bool AddBlock(FVector BlockPosition, uint8 Material, uint8 DataBits) override;
	virtual bool RemoveBlock(FVector BlockPosition) override;
	virtual int32 GetBlockMaterial(FVector BlockPosition) override;

	virtual FVector WorldSpaceToVoxelSpace(FVector WorldSpace) override;
	virtual FVector VoxelSpaceToWorldSpace(FVector VoxelSpace) override;
	virtual FVector GetWorldChunkLocationFromWorldSpace(FVector WorldSpace) override;
	virtual FVector GetVoxelChunkLocationFromVoxelSpace(FVector VoxelSpace) override;

	// The seed of our fractal
	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere) int32 Seed;

	// The number of octaves that the noise generator will use
	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere) int32 NoiseOctaves;

	// The frequency of the noise
	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere) float NoiseFrequency;

	// The scale of the noise. The output of the TerrainFractal is multiplied by this.
	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere) float NoiseScale;

	// The offset of the noise. This value is added to the output of the TerrainFractal.
	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere) float NoiseOffset;

	// The maximum height of the generated terrain in voxels. NOTE: Changing this will affect where the ground begins!
	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere) float TerrainHeight;

	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere) float BlockSize;

	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere) float ChunkSize;

	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere) float ChunkSurrounds;

	UPROPERTY(Category = "Voxel Terrain", BlueprintReadWrite, EditAnywhere) float CollisionSurrounds;

private:
	TSharedPtr<PolyVox::PagedVolume<CurBlocksVoxelType>> VoxelVolume;

	UPROPERTY(Instanced)
		UVoxelChunkManager* ChunkManager;

	FCriticalSection CriticalSection;

	PolyVox::PickResult RaycastInternal(FVector Origin, FVector Direction);

	PolyVox::Region findRegionfromOrigin(FVector Origin);
};

// Bridge between PolyVox Vector3DFloat and Unreal Engine 4 FVector
struct FPolyVoxVector : public FVector
{
	FORCEINLINE FPolyVoxVector()
	{}

	explicit FORCEINLINE FPolyVoxVector(EForceInit E)
		: FVector(E)
	{}

	FORCEINLINE FPolyVoxVector(float InX, float InY, float InZ)
		: FVector(InX, InY, InX)
	{}

	FORCEINLINE FPolyVoxVector(const FVector &InVec)
	{
		FVector::operator=(InVec);
	}

	FORCEINLINE FPolyVoxVector(const PolyVox::Vector3DFloat &InVec)
	{
		FPolyVoxVector::operator=(InVec);
	}

	FORCEINLINE FVector& operator=(const PolyVox::Vector3DFloat& Other)
	{
		this->X = Other.getX();
		this->Y = Other.getY();
		this->Z = Other.getZ();

		DiagnosticCheckNaN();

		return *this;
	}
};