// Fill out your copyright notice in the Description page of Project Settings.

#include "Blocks.h"
#include "VoxelChunk.h"
#include "PolyVox/PagedVolume.h"

void UVoxelChunk::ExtractMesh(UVoxelTerrainVolume* VoxelVolume)
{
	auto x = VoxelVolume->ExtractMesh(Origin);
	if (x) {
		Mesh = TUniquePtr<FDecodedMesh>(x);
	}
	else {
		Mesh = nullptr;
	}
	ChunkState = EChunkState::Generated;
	
}

void UVoxelChunk::RenderMesh(URuntimeMeshComponent* TerrainMesh, UMaterialInterface* Material)
{
	// Render the mesh
	if (Mesh == nullptr || Mesh->Vertices.Num() == 0) {
		ChunkState = EChunkState::Rendered;
		return;
	}
	if (TerrainMesh->DoesSectionExist(MeshSection))
	{
		UE_LOG(LogTemp, Warning, TEXT("This section %d already exists"), MeshSection);
		TerrainMesh->UpdateMeshSection(MeshSection, Mesh->Vertices, Mesh->Indices, Mesh->Normals, Mesh->UV0, Mesh->Colors, Mesh->Tangents);
		//TerrainMesh->SetMeshCollisionSection(MeshSection, Mesh.Vertices, Mesh.Indices);
	}
	else
	{
		TerrainMesh->CreateMeshSection(MeshSection, Mesh->Vertices, Mesh->Indices, Mesh->Normals, Mesh->UV0, Mesh->Colors, Mesh->Tangents, false, EUpdateFrequency::Frequent);
		//TerrainMesh->SetMeshCollisionSection(MeshSection, Mesh.Vertices, Mesh.Indices);
		//TerrainMesh->CookCollisionNow();
		TerrainMesh->SetMaterial(MeshSection, Material);
	}
	ChunkState = EChunkState::Rendered;
	UrgentUpdate = false;
}

void UVoxelChunk::GenerateMeshCollision(URuntimeMeshComponent* TerrainMesh)
{
	if (ChunkState != EChunkState::Rendered)
	{
		return;
	}
	TerrainMesh->SetMeshCollisionSection(MeshSection, Mesh->Vertices, Mesh->Indices);

	ChunkState = EChunkState::HasCollision;
}

bool UVoxelChunk::IsReadyToRender()
{
	return ChunkState == EChunkState::Generated;
}

uint8 UVoxelChunk::GetMeshSection()
{
	return MeshSection;
}

void UVoxelChunk::SetRendering()
{
	ChunkState = EChunkState::Generating;
}

bool UVoxelChunk::CheckNeedsToRender()
{
	return ChunkState == EChunkState::Uninitted;
}

bool UVoxelChunk::CheckNeedsCollision()
{
	return ChunkState == EChunkState::Rendered;
}