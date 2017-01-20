// Fill out your copyright notice in the Description page of Project Settings.

#include "Blocks.h"
#include "VoxelChunk.h"
#include "VoxelObject.h"
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

void UVoxelChunk::SpawnObjects(UVoxelTerrainVolume* VoxelVolume)
{
	TArray<FObjectVoxel> ObjectsToSpawn = VoxelVolume->SpawnObjects(Origin);
	for (auto Obj : ObjectsToSpawn)
	{
		auto World = GetTypedOuter<AActor>()->GetWorld();
		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(ECollisionChannel::ECC_GameTraceChannel4); // BObj
		bool ObjectExists = World->SweepTestByObjectType(
			Obj.WorldLocation
			, Obj.WorldLocation + 1
			, FQuat()
			, ObjectParams
			, FCollisionShape::MakeBox(FVector(50.f, 50.f, 50.f))
		);
		if (!ObjectExists)
		{
			FTransform Transform(Obj.WorldLocation);
			AVoxelObject* Spawned = World->SpawnActorDeferred<AVoxelObject>(*Obj.Class, Transform);
			Spawned->InitData(Obj.Data);
			Spawned->FinishSpawning(Transform);
		}
		
	}
	ChunkState = EChunkState::ObjectsSpawned;
	
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

bool UVoxelChunk::CheckNeedsObjects()
{
	return ChunkState > EChunkState::Rendered && ChunkState != EChunkState::ObjectsSpawned;
}
