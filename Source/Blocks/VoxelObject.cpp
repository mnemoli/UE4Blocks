// Fill out your copyright notice in the Description page of Project Settings.

#include "Blocks.h"
#include "VoxelObject.h"


// Sets default values
AVoxelObject::AVoxelObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AVoxelObject::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVoxelObject::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

