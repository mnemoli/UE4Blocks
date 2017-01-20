// Fill out your copyright notice in the Description page of Project Settings.

#include "Blocks.h"
#include "ClassMapping.h"

ClassMapping::ClassMapping()
{
	//read in the config file
	FString FileString;
	FString FileName = FString(TEXT("MaterialConfig.json"));
	FString FilePath = FPaths::Combine(*FPaths::GameConfigDir(), *FileName);
	FFileHelper::LoadFileToString(FileString, *FilePath);
	auto Reader = TJsonReaderFactory<>::Create(FileString);
	TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject());

	if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
	{	
		for (auto Value : JsonObj->Values)
		{
			auto Class = (StaticLoadClass(AVoxelObject::StaticClass() , nullptr, *Value.Value->AsString()));
			TSubclassOf<AVoxelObject> blah = Class;
			ClassMap.Add(FCString::Atoi(*Value.Key), blah);
		}
	}
}

 bool ClassMapping::GetClassFromMaterial(uint32 Material, TSubclassOf<AVoxelObject>& Subclass)
{
	static ClassMapping Instance;
	if (Material == 0)
		return false;
	if (Instance.ClassMap.Contains(Material))
	{
		Subclass = Instance.ClassMap[Material];
		UE_LOG(LogTemp, Warning, TEXT("Material %d has a mapping"), Material);
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Material %d has no mapping"), Material);
		return false;
	}

}

ClassMapping::~ClassMapping()
{
}
