// Fill out your copyright notice in the Description page of Project Settings.

#include "Blocks.h"
#include "VoxelChunkManager.h"
#include "AVoxelTerrainActor.h"
#include "VoxelChunkWorker.h"

void UVoxelChunkManager::InitalizeChunkManager()
{
	VoxelVolume = GetTypedOuter<AVoxelTerrainActor>()->VoxelVolume;
	UPolyVoxVolume *Volume = Cast<UPolyVoxVolume>(VoxelVolume);
	MeshSectionQueue.Init(false, pow(((Volume->ChunkSurrounds + 1)*2),2)*2 );
}

void UVoxelChunkManager::InvalidateChunk(FVector VoxelPosition)
{
	const FVector ChunkPosInVoxelSpace = VoxelVolume->GetVoxelChunkLocationFromVoxelSpace(VoxelPosition);
	const FVector ChunkPosInWorldSpace = VoxelVolume->VoxelSpaceToWorldSpace(ChunkPosInVoxelSpace);

	if (ChunkMap.Contains(ChunkPosInWorldSpace.ToString()))
	{
		ChunkMap[ChunkPosInWorldSpace.ToString()]->Invalidate();
		UE_LOG(LogTemp, Warning, TEXT("Invalidated chunk %s"), *ChunkPosInWorldSpace.ToString());
	}
	else
	{
		auto x = ChunkPosInWorldSpace.ToString();
		UE_LOG(LogTemp, Warning, TEXT("Looking for %s, didn't find"), *x);
	}

	// Check for surrounding chunks that need to invalidated as well
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			if (x == 0 && y == 0)
				continue;
			const FVector NewChunkPos = VoxelVolume->GetVoxelChunkLocationFromVoxelSpace(VoxelPosition + FVector(x, y, 0));
			if (NewChunkPos != ChunkPosInVoxelSpace)
			{
				const FVector NewChunkPosWorldSpace = VoxelVolume->VoxelSpaceToWorldSpace(NewChunkPos);
				if (ChunkMap.Contains(NewChunkPosWorldSpace.ToString()))
				{
					ChunkMap[NewChunkPosWorldSpace.ToString()]->Invalidate();
					UE_LOG(LogTemp, Warning, TEXT("Invalidated chunk %s"), *NewChunkPosWorldSpace.ToString());
				}
				else
				{
					auto x = NewChunkPosWorldSpace.ToString();
					UE_LOG(LogTemp, Warning, TEXT("Looking for %s, didn't find"), *x);
				}
			}
		}
	}

	

}

void UVoxelChunkManager::UpdateChunks(FVector PlayerPos)
{
	UPolyVoxVolume *Volume = Cast<UPolyVoxVolume>(VoxelVolume);
	FVector ZNullPlayerPos(PlayerPos.X, PlayerPos.Y, 0);
	FVector PlayerVoxelLoc = Volume->WorldSpaceToVoxelSpace(ZNullPlayerPos);
	AVoxelTerrainActor* Outer = GetTypedOuter<AVoxelTerrainActor>();

	auto sortFunc = [ZNullPlayerPos](FString a, FString b)
	{
		FVector ChunkVectorA;
		ChunkVectorA.InitFromString(a);
		FVector ChunkVectorB;
		ChunkVectorB.InitFromString(b);

		float DistanceA = FVector::DistSquared(ChunkVectorA, ZNullPlayerPos);
		float DistanceB = FVector::DistSquared(ChunkVectorB, ZNullPlayerPos);

		return  DistanceA < DistanceB;
	};

	ChunkMap.KeySort(sortFunc);

	// Check for chunks that are now out of range and remove them
	// Load or render or collide chunks that need it
	for (auto Chunk : ChunkMap)
	{
		FVector ChunkVector;
		ChunkVector.InitFromString(Chunk.Key);
		FVector ChunkVoxelLoc = Volume->WorldSpaceToVoxelSpace(ChunkVector);
		//FVector Distance = (PlayerVoxelLoc - ChunkVoxelLoc).GetAbs() / Volume->ChunkSize;
		//if (Distance.GetMax() > Volume->ChunkSurrounds + 2)
		if (!FVector::PointsAreNear(ChunkVector, ZNullPlayerPos, (Volume->ChunkSurrounds+1) * Volume->ChunkSize * Volume->BlockSize ))
		{
			UE_LOG(LogTemp, Warning, TEXT("Unloading chunk %s from %d"), *Chunk.Key, Chunk.Value->GetMeshSection())
			uint8 MeshSection = Chunk.Value->GetMeshSection();
			if (MeshSection <= MeshSectionQueue.Num())
			{
				MeshSectionQueue[MeshSection] = false;
				ChunkMap.Remove(Chunk.Key);
				Outer->TerrainMesh->ClearMeshSection(MeshSection);
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Something horrible happened"));
			}
		}
		else if (Chunk.Value->CheckNeedsToRender())
		{
			Chunk.Value->SetRendering();
			auto C = Chunk.Value;
			auto Task = [C, Volume]() { C->ExtractMesh(Volume); };
			if (C->UrgentUpdate) // If we just invalidated/updated this chunk via player action and it needs to be done right now,
								 // get a single thread to do it rather than pooling
			{
				Async<void>(EAsyncExecution::Thread, Task);
			}
			else
			{
				Async<void>(EAsyncExecution::ThreadPool, Task);
			}
		}
		else if (Chunk.Value->IsReadyToRender())
		{
			Chunk.Value->RenderMesh(Outer->TerrainMesh, Volume->TerrainMaterial);
		}
		else if (FVector::PointsAreNear(ChunkVector, ZNullPlayerPos, Volume->CollisionSurrounds * Volume->ChunkSize * Volume->BlockSize)
			//Distance.GetMax() < Volume->CollisionSurrounds 
			&& Chunk.Value->CheckNeedsCollision() )
		{
			Chunk.Value->GenerateMeshCollision(Outer->TerrainMesh);
		}
	}

	// Check for new chunks which need to be loaded in
	
	for (int x = -Volume->ChunkSurrounds; x <= Volume->ChunkSurrounds; x++)
	{
		for (int y = -Volume->ChunkSurrounds; y <= Volume->ChunkSurrounds; y++)
		{
			FVector PlayerChunkOrigin = Volume->GetWorldChunkLocationFromWorldSpace(ZNullPlayerPos);

			// Check whether the chunk at the loop position offset to the player's position is loaded

			FVector LoopChunkOrigin(x, y, 0);
			LoopChunkOrigin *= Volume->ChunkSize;
			FVector LoopChunkOriginVoxelSpace = Volume->VoxelSpaceToWorldSpace(LoopChunkOrigin);

			FVector ChunkToLoadOrigin(PlayerChunkOrigin + LoopChunkOriginVoxelSpace);
			if (!ChunkMap.Contains(ChunkToLoadOrigin.ToString()))
			{
				int32 Section;
				bool HasFreeMeshSection = MeshSectionQueue.Find(false, Section);
				if (HasFreeMeshSection) {
					MeshSectionQueue[Section] = true;
					UE_LOG(LogTemp, Warning, TEXT("Loading chunk %s into %d"), *ChunkToLoadOrigin.ToString(), Section);
					UVoxelChunk* theChunk = NewObject<UVoxelChunk>();
					theChunk->Initialize(ChunkToLoadOrigin, Section);
					ChunkMap.Emplace(ChunkToLoadOrigin.ToString(), theChunk);
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Oops, we loaded too many chunks, out of sections"));
				}
			}
		}
	}
}

void UVoxelChunkManager::SetupTest()
{
	UE_LOG(LogTemp, Warning, TEXT("TESTING"));

	auto Volume = GetTypedOuter<AVoxelTerrainActor>()->VoxelVolume;
	auto ChunkSize = Volume->ChunkSize;
	auto Surrounds = Volume->ChunkSurrounds;
	int section = 0;

	FVector origin(6400, 4800, 0);

	for (int i = 0; i < Surrounds; i++) {
		for (int j = 0; j < Surrounds; j++) {
			FVector theVector(origin.X + i*ChunkSize*Volume->BlockSize, origin.Y + j*ChunkSize*Volume->BlockSize, 0);
			UVoxelChunk* theChunk = NewObject<UVoxelChunk>();
			theChunk->Initialize(theVector, section);
			ChunkMap.Emplace(theVector.ToString(), theChunk);
			section++;
		}
	}
	//LoadChunks();
}

void UVoxelChunkManager::LoadChunks()
{
	for (auto Chunk : ChunkMap)
	{
		if (Chunk.Value->CheckNeedsToRender())
		{
			AVoxelTerrainActor* Outer = GetTypedOuter<AVoxelTerrainActor>();
			auto Volume = Outer->VoxelVolume;
			auto ChunkSize = Volume->ChunkSize;
			Chunk.Value->SetRendering();
			auto C = Chunk.Value;
			auto Task = [C, Volume]() { C->ExtractMesh(Volume); };
			if (Chunk.Value->UrgentUpdate)
			{
				Async<void>(EAsyncExecution::Thread, Task);
			}
			else
			{
				Async<void>(EAsyncExecution::ThreadPool, Task);
			}
		}
		
	}
}

void UVoxelChunkManager::Tick(float DeltaTime)
{
	AVoxelTerrainActor* Outer = GetTypedOuter<AVoxelTerrainActor>();

	// Update the chunks before rendering
	UWorld *World = GetTypedOuter<AVoxelTerrainActor>()->GetWorld();
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (Player)
	{
		FVector PlayerPos = Player->GetActorLocation();
		UpdateChunks(PlayerPos);
	}
}