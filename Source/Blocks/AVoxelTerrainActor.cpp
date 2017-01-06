// Fill out your copyright notice in the Description page of Project Settings.

#include "Blocks.h"
#include "AVoxelTerrainActor.h"
// PolyVox
using namespace PolyVox;

// ANL
#include "VM/kernel.h"
using namespace anl;


// Sets default values
AVoxelTerrainActor::AVoxelTerrainActor()
{
	// Initialize our mesh component
	
	TerrainMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("Terrain Mesh"));
	VoxelVolume = CreateDefaultSubobject<UPolyVoxVolume>(TEXT("PolyVox Volume"));

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bAllowTickOnDedicatedServer = true;

}

// Called after the C++ constructor and after the properties have been initialized.

void AVoxelTerrainActor::PostInitializeComponents()
{
	// Call the base class's function.
	Super::PostInitializeComponents();
}

// Called when the actor has begun playing in the level
void AVoxelTerrainActor::BeginPlay()
{
	Super::BeginPlay();
	//VoxelVolume->Test();
}

void AVoxelTerrainActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}