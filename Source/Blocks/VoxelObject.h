// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "VoxelObject.generated.h"

UCLASS()
class BLOCKS_API AVoxelObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVoxelObject();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	void InitData(uint32 pData) { Data = pData; };

	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 Data;
	
private:
};
