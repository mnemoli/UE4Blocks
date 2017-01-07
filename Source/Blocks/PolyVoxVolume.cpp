// Fill out your copyright notice in the Description page of Project Settings.

#include "Blocks.h"
#include "PolyVoxVolume.h"
#include "VoxelTerrainPager.h"

UPolyVoxVolume::UPolyVoxVolume()
{
	// Default values for our noise control variables.
	Seed = 123;
	NoiseOctaves = 3;
	NoiseFrequency = 0.01f;
	NoiseScale = 32.f;
	NoiseOffset = 0.f;
	TerrainHeight = 64.f;
	ChunkSize = 128;
	BlockSize = 100;

	VoxelVolume = MakeShareable(new PolyVox::PagedVolume<PolyVox::MaterialDensityPair44>(new VoxelTerrainPager(Seed, NoiseOctaves, NoiseFrequency, NoiseScale, NoiseOffset, TerrainHeight)));
	ChunkManager = CreateDefaultSubobject<UVoxelChunkManager>(TEXT("Chunk Manager"));

	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
	bAutoRegister = true;
	
}

UPolyVoxVolume::~UPolyVoxVolume()
{
}

FVector UPolyVoxVolume::WorldSpaceToVoxelSpace(FVector WorldSpace)
{
	return WorldSpace / BlockSize;
}

FVector UPolyVoxVolume::VoxelSpaceToWorldSpace(FVector VoxelSpace)
{
	return (VoxelSpace * BlockSize);
}

FVector UPolyVoxVolume::GetWorldChunkLocationFromWorldSpace(FVector WorldSpace)
{
	FVector VoxelSpace = WorldSpaceToVoxelSpace(WorldSpace);
	FVector ChunkLocation = GetVoxelChunkLocationFromVoxelSpace(VoxelSpace);
	FVector BackToWorldLocation = ChunkLocation * BlockSize;
	return BackToWorldLocation;
}

FVector UPolyVoxVolume::GetVoxelChunkLocationFromVoxelSpace(FVector VoxelSpace)
{
	FVector FlooredVector(floorf(VoxelSpace.X / ChunkSize), floorf(VoxelSpace.Y / ChunkSize), floorf(VoxelSpace.Z / TerrainHeight));
	return (FlooredVector * ChunkSize);
}

FDecodedMesh* UPolyVoxVolume::ExtractMesh(FVector Origin)
{
	FDecodedMesh* Mesh = new FDecodedMesh();
	//Convert into voxelspace (i.e. divide by BlockSize). Should go perfectly since origin should be a multiple of the blocksize
	FVector InVoxelSpace = WorldSpaceToVoxelSpace(Origin);

	PolyVox::Vector3DInt32 o = PolyVox::Vector3DInt32(InVoxelSpace.X, InVoxelSpace.Y, InVoxelSpace.Z);
	PolyVox::Vector3DInt32 t = PolyVox::Vector3DInt32(InVoxelSpace.X + ChunkSize - 1, InVoxelSpace.Y + ChunkSize - 1, TerrainHeight);

	PolyVox::Region ToExtract(o, t);
	CriticalSection.Lock();
	auto ExtractedMesh = extractCubicMesh(VoxelVolume.Get(),  ToExtract);
	CriticalSection.Unlock();
	auto DecodedMesh = decodeMesh(ExtractedMesh);

	if (DecodedMesh.getNoOfIndices() == 0) {
		return Mesh;
	}

	// Loop over all of the triangle vertex indices
	for (uint32 i = 0; i < DecodedMesh.getNoOfIndices() - 2; i += 3)
	{
		// We need to add the vertices of each triangle in reverse or the mesh will be upside down
		auto Index = DecodedMesh.getIndex(i + 2);
		auto Vertex2 = DecodedMesh.getVertex(Index);
		auto TriangleMaterial = Vertex2.data.getMaterial();
		FColor color(TriangleMaterial, 0, 0, 255);

		FVector temp(FPolyVoxVector(Vertex2.position));
		Mesh->Indices.Add(Mesh->Vertices.Add(temp * BlockSize + Origin));
		Mesh->Colors.Add(color);

		Index = DecodedMesh.getIndex(i + 1);
		auto Vertex1 = DecodedMesh.getVertex(Index);
		temp = FPolyVoxVector(Vertex1.position);
		Mesh->Indices.Add(Mesh->Vertices.Add(temp * BlockSize + Origin));
		Mesh->Colors.Add(color);

		Index = DecodedMesh.getIndex(i);
		auto Vertex0 = DecodedMesh.getVertex(Index);
		temp = FPolyVoxVector(Vertex0.position);
		Mesh->Indices.Add(Mesh->Vertices.Add(temp * BlockSize + Origin));
		Mesh->Colors.Add(color);

		// Calculate the tangents of our triangle
		const FVector Edge01 = FPolyVoxVector(Vertex1.position - Vertex0.position);
		const FVector Edge02 = FPolyVoxVector(Vertex2.position - Vertex0.position);

		const FVector TangentX = Edge01.GetSafeNormal();
		FVector TangentZ = (Edge01 ^ Edge02).GetSafeNormal();

		for (int32 i = 0; i < 3; i++)
		{
			Mesh->Tangents.Add(FRuntimeMeshTangent(TangentX, false));
			Mesh->Normals.Add(TangentZ);
		}
	}

	return Mesh;
}

void UPolyVoxVolume::Test()
{
	ChunkManager->SetupTest();
}

void UPolyVoxVolume::BeginPlay()
{
	Super::BeginPlay();
	ChunkManager->InitalizeChunkManager();
}

void UPolyVoxVolume::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ChunkManager->Tick(DeltaTime);
}

bool UPolyVoxVolume::Raycast(FVector Origin, FVector Direction, FVector& VoxelLocation)
{
	UE_LOG(LogTemp, Warning, TEXT("Trying to do a raycast"));
	PolyVox::PickResult Result = RaycastInternal(Origin, Direction);
	if (Result.didHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit summat"));
		VoxelLocation = FVector(Result.hitVoxel.getX(), Result.hitVoxel.getY(), Result.hitVoxel.getZ());
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Didn't hit anything"));
		return false;
	}
}

bool UPolyVoxVolume::RaycastPrevious(FVector Origin, FVector Direction, FVector& VoxelLocation)
{

	PolyVox::PickResult Result = RaycastInternal(Origin, Direction);
	if (Result.didHit && Result.hasPreviousVoxel)
	{
		VoxelLocation = FVector(Result.previousVoxel.getX(), Result.previousVoxel.getY(), Result.previousVoxel.getZ());
		return true;
	}
	else
	{
		return false;
	}
}

PolyVox::PickResult UPolyVoxVolume::RaycastInternal(FVector Origin, FVector Direction)
{
	const PolyVox::MaterialDensityPair44 EmptyVoxel = PolyVox::MaterialDensityPair44(0, 0);
	const FVector OriginVoxelSpace = WorldSpaceToVoxelSpace(Origin);
	const FVector DirectionVoxelSpace = WorldSpaceToVoxelSpace(Direction);
	const PolyVox::Vector3DFloat O = PolyVox::Vector3DFloat(OriginVoxelSpace.X, OriginVoxelSpace.Y, OriginVoxelSpace.Z);
	const PolyVox::Vector3DFloat D = PolyVox::Vector3DFloat(DirectionVoxelSpace.X, DirectionVoxelSpace.Y, DirectionVoxelSpace.Z);
	PolyVox::PickResult Result = PolyVox::pickVoxel(VoxelVolume.Get(), O, D, EmptyVoxel);

	return Result;
}

bool UPolyVoxVolume::AddBlock(FVector BlockPosition, uint8 Material)
{
	const PolyVox::Vector3DInt32 Pos(BlockPosition.X, BlockPosition.Y, BlockPosition.Z);
	auto VoxelToChange = VoxelVolume->getVoxel(Pos);
	VoxelToChange.setDensity(1);
	VoxelToChange.setMaterial(Material);
	VoxelVolume->setVoxel(Pos, VoxelToChange);

	ChunkManager->InvalidateChunk(BlockPosition);
	
	return true;
}

bool UPolyVoxVolume::RemoveBlock(FVector BlockPosition)
{
	const auto VoxelSpace = WorldSpaceToVoxelSpace(BlockPosition);
	const PolyVox::Vector3DInt32 Pos(VoxelSpace.X, VoxelSpace.Y, VoxelSpace.Z);
	auto VoxelToChange = VoxelVolume->getVoxel(Pos);
	VoxelToChange.setDensity(0);
	VoxelToChange.setMaterial(0);
	VoxelVolume->setVoxel(Pos, VoxelToChange);

	ChunkManager->InvalidateChunk(VoxelSpace);

	return true;
}

int32 UPolyVoxVolume::GetBlockMaterial(FVector BlockPosition)
{
	const auto VoxelSpace = WorldSpaceToVoxelSpace(BlockPosition);
	const PolyVox::Vector3DInt32 Pos(VoxelSpace.X, VoxelSpace.Y, VoxelSpace.Z);
	auto Voxel = VoxelVolume->getVoxel(Pos);
	return Voxel.getMaterial();
}