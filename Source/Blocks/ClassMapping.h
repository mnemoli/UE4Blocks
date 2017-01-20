// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Json.h"
#include "VoxelObject.h"

/**
 * 
 */
class BLOCKS_API ClassMapping
{
public:
	static bool GetClassFromMaterial(uint32 Material, TSubclassOf<AVoxelObject>& Subclass);
	~ClassMapping();

	// delete copy and move constructors and assign operators
	ClassMapping(ClassMapping const&) = delete;             // Copy construct
	ClassMapping(ClassMapping&&) = delete;                  // Move construct
	ClassMapping& operator=(ClassMapping const&) = delete;  // Copy assign
	ClassMapping& operator=(ClassMapping &&) = delete;      // Move assign

private:
	ClassMapping();
	TMap<uint32, TSubclassOf<AVoxelObject>> ClassMap;
};
