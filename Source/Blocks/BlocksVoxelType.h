// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PolyVox/DefaultIsQuadNeeded.h"
#include "PolyVox/DefaultMarchingCubesController.h"
#include "PolyVox/Impl/PlatformDefinitions.h"
#include "PolyVox/Picking.h"
#include "ClassMapping.h"

/**
 * Blocks-specific type including material, density, class and data
 */

namespace PolyVox {

template <typename Type, uint8_t NoOfMaterialBits, uint8_t NoOfDensityBits, uint8_t NoOfDataBits>
class BLOCKS_API BlocksVoxelType
{
public:
	BlocksVoxelType() : m_uMaterial(0), m_uDensity(0), m_uData(0) {}
	BlocksVoxelType(Type uMaterial, Type uDensity, Type uData) : m_uMaterial(uMaterial), m_uDensity(uDensity), m_uData(uData) {}

	bool operator==(const BlocksVoxelType& rhs) const
	{
		return (m_uMaterial == rhs.m_uMaterial) && (m_uDensity == rhs.m_uDensity);
	};

	bool operator!=(const BlocksVoxelType& rhs) const
	{
		return !(*this == rhs);
	}

	Type getDensity() const { return m_uDensity; }
	Type getMaterial() const { return m_uMaterial; }
	Type getData() const { return m_uData;  }

	void setDensity(Type uDensity) { m_uDensity = uDensity; }
	void setMaterial(Type uMaterial) { m_uMaterial = uMaterial; }
	void setData(Type uData) { m_uData = uData; }

	static Type getMaxDensity() { return (0x01 << NoOfDensityBits) - 1; }
	static Type getMinDensity() { return 0; }

private:
	Type m_uMaterial : NoOfMaterialBits;
	Type m_uDensity : NoOfDensityBits;
	Type m_uData : NoOfDataBits;
};

	template<typename Type, uint8_t NoOfMaterialBits, uint8_t NoOfDensityBits, uint8_t NoOfDataBits>
	class DefaultIsQuadNeeded<BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits> >
	{
	public:
		bool operator()(BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits> back, BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits> front, BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits>& materialToUse)
		{
			if ((back.getDensity() > 0) && (front.getDensity() == 0))
			{
				materialToUse = back;
				return true;
			}
			else
			{
				return false;
			}
		}
	};

	template <typename Type, uint8_t NoOfMaterialBits, uint8_t NoOfDensityBits, uint8_t NoOfDataBits>
	class DefaultMarchingCubesController< BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits> >
	{
	public:
		typedef Type DensityType;
		typedef Type MaterialType;

		DefaultMarchingCubesController(void)
		{
			// Default to a threshold value halfway between the min and max possible values.
			m_tThreshold = (BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits>::getMinDensity() + BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits>::getMaxDensity()) / 2;
		}

		DefaultMarchingCubesController(DensityType tThreshold)
		{
			m_tThreshold = tThreshold;
		}

		DensityType convertToDensity(BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits> voxel)
		{
			/*if (ClassMapping::GetClassFromMaterial(voxel.getMaterial()))
			{
				return 0;
			}*/
			return voxel.getDensity();
		}

		MaterialType convertToMaterial(BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits> voxel)
		{
			return voxel.getMaterial();
		}

		BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits> blendMaterials(BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits> a, BlocksVoxelType<Type, NoOfMaterialBits, NoOfDensityBits, NoOfDataBits> b, float /*weight*/)
		{
			if (convertToDensity(a) > convertToDensity(b))
			{
				return a;
			}
			else
			{
				return b;
			}
		}

		DensityType getThreshold(void)
		{
			return m_tThreshold;
		}

		void setThreshold(DensityType tThreshold)
		{
			m_tThreshold = tThreshold;
		}

	private:
		DensityType m_tThreshold;
	};


	}

	template <typename VolumeType>
	class BlocksRaycastPickingFunctor
	{
	public:
		BlocksRaycastPickingFunctor():
			m_result()
		{
		}

		bool operator()(const typename VolumeType::Sampler& sampler)
		{
			auto Voxel = sampler.getVoxel();
			TSubclassOf<AVoxelObject> Subclass;
			if (Voxel.getDensity() > 0 && !ClassMapping::GetClassFromMaterial(Voxel.getMaterial(), Subclass)) //If we've hit something and it's not a class mapped thing
			{
				m_result.didHit = true;
				m_result.hitVoxel = sampler.getPosition();
				return false;
			}

			m_result.hasPreviousVoxel = true;
			m_result.previousVoxel = sampler.getPosition();

			return true;
		}
		PolyVox::PickResult m_result;
	};

typedef PolyVox::BlocksVoxelType<uint8_t, 8, 8, 8> CurBlocksVoxelType;

