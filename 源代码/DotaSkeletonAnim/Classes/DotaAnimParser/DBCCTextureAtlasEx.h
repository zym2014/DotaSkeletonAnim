#pragma once

#include "cocos2d.h"
#include "DragonBonesHeaders.h"
#include "DBCCTextureAtlas.h"
#include "Utils.h"

USING_NS_DB;

class DBCCTextureAtlasEx : public DBCCTextureAtlas
{
public:
	DBCCTextureAtlasEx(void)
	{
	}
	virtual ~DBCCTextureAtlasEx(void)
	{
	}

public:
	virtual void dispose() override
	{
		if (textureAtlasData)
		{
			std::string key = _fcaFile + "/" + textureAtlasData->imagePath;
			auto textureCache = cocos2d::Director::getInstance()->getTextureCache();
			textureCache->removeTextureForKey(key);
			textureAtlasData->dispose();
			textureAtlasData = nullptr;
		}
	}

	virtual cocos2d::Texture2D* getTexture()
	{
		if (!textureAtlasData) return nullptr;

		std::string key = _fcaFile + "/" + textureAtlasData->imagePath;

		auto textureCache = cocos2d::Director::getInstance()->getTextureCache();
		auto texture = textureCache->getTextureForKey(key);
		if (!texture)
		{
			ssize_t size = 0;
			unsigned char * data = getFileDataFromZip(_fcaFile.c_str(), 
				textureAtlasData->imagePath.c_str(), &size);
			if (nullptr == data || size <= 0)
			{
				free(data);
				return nullptr;
			}

			cocos2d::Image * image = new (std::nothrow) cocos2d::Image();
			if (nullptr == image)
			{
				free(data);
				return nullptr;
			}

			bool bRet = image->initWithImageData(data, size);
			free(data);
			if (!bRet)
			{
				return nullptr;
			}

			texture = textureCache->addImage(image, key);
		}
		return texture;
	}

	virtual cocos2d::Texture2D* reloadTexture()
	{
		return getTexture();
	}

public:
	std::string _fcaFile;

private:
	DRAGON_BONES_DISALLOW_COPY_AND_ASSIGN(DBCCTextureAtlasEx);
};

