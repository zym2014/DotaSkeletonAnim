#pragma once

#include "cocos2d.h"
#include "DBCCRenderHeaders.h"
#include "DotaAnimParser.h"

class DBCCFactoryEx : public dragonBones::DBCCFactory
{
public:
	DBCCFactoryEx(void);
	virtual ~DBCCFactoryEx(void);

public:
	static DBCCFactoryEx* getInstanceEx();
	static void destroyInstanceEx();

	// 加载刀塔传奇的*.fca文件
	bool loadDotaFcaFile(const std::string &fcaFile);

	// 导出DragonBones的配置文件(sheet.pvr、texture.xml、skeleton.xml)
	bool exportDBConfigFile(const std::string &fcaFile, const std::string &savePath);

private:
	// 加载纹理集数据
	dragonBones::ITextureAtlas* loadDotaTextureAtlas(
		const std::string &fcaFile,
		const std::string &name = "");

	// 加载骨骼数据
	dragonBones::DragonBonesData* loadDotaDragonBonesData(
		Dota_Skeleton_Data& dotaSkeletonData,
		dragonBones::ITextureAtlas& textureAtlas, 
		const std::string &name = "");

private:
	static DBCCFactoryEx *_instanceex;
};

