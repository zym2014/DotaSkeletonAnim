#include "Utils.h"

USING_NS_CC;
USING_NS_CC_EXT;

void log(const char* str)
{
	if (nullptr == str || '\0' == *str)
		return;

	// SD卡路径现在是写死的，红米手机
	FILE* fp = fopen("/storage/sdcard0/test.txt", "ab+");
	if (fp != nullptr)
	{
		fwrite(str, strlen(str), 1, fp);
		fclose(fp);
	}
}

unsigned char* getFileDataFromZip(const char* zipFile, const char* fileName, ssize_t* size)
{
	if (nullptr == zipFile || nullptr == fileName || nullptr == size)
		return nullptr;

	Data zipData = FileUtils::getInstance()->getDataFromFile(zipFile);
	if (zipData.isNull())
		return nullptr;

	ZipFile * zip = ZipFile::createWithBuffer(zipData.getBytes(), zipData.getSize());
	if (nullptr == zip)
		return nullptr;

	*size = 0;
	unsigned char * data = zip->getFileData(fileName, size);
	delete zip;
	if (nullptr == data || size <= 0)
	{
		*size = 0;
		free(data);
		return nullptr;
	}

	return data;
}

Texture2D* openImage(const char* jpgFile, const char* maskFile)
{
	if (nullptr == jpgFile || '\0' == *jpgFile 
		|| nullptr == maskFile || '\0' == *maskFile)
		return nullptr;

	Texture2D* ret = Director::getInstance()->getTextureCache()->getTextureForKey(jpgFile);
	if (ret)
		return ret;

	Image* img = new Image();
	Image* jpgImg = new Image();
	Image* pngImg = new Image();
	do 
	{
		if (!jpgImg->initWithImageFile(jpgFile))
			break;

		if (!pngImg->initWithImageFile(maskFile))
			break;

		unsigned char* jpgData = jpgImg->getData();
		unsigned char* pngData = pngImg->getData();

		int jpgStep = jpgImg->getBitPerPixel() / 8;
		int pngStep = pngImg->getBitPerPixel() / 8;

		int width = jpgImg->getWidth();
		int height = jpgImg->getHeight();

		ssize_t outDataLen = width*height*4;
		unsigned char * outData = new unsigned char[outDataLen];
		if (!outData)
			break;

		int pngIndex = 0;
		int jpgIndex = 0;
		unsigned int *tmpData = (unsigned int *)outData;

		for (int i = 0; i < width*height; i++)
		{
			*tmpData++ = CC_RGB_PREMULTIPLY_ALPHA(
				jpgData[jpgIndex],		// R
				jpgData[jpgIndex + 1],	// G
				jpgData[jpgIndex + 2],	// B
				pngData[pngIndex]);		//A

			jpgIndex += jpgStep;
			pngIndex += pngStep;
		}
		img->initWithRawData(outData, outDataLen, width, height, 4, true);
		ret = Director::getInstance()->getTextureCache()->addImage(img, jpgFile);
		delete []outData;
		break;
	} while (1);

	CC_SAFE_DELETE(img);
	CC_SAFE_DELETE(jpgImg);
	CC_SAFE_DELETE(pngImg);
	return ret;
}

Texture2D* openDotaImage(const char* jpgFile)
{
	// 目前Android和IOS平台处理jpg+mask还有点问题，故暂时屏蔽相关平台的处理代码

	if (nullptr == jpgFile || '\0' == *jpgFile)
		return nullptr;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
	std::string basePath = "", fileName = jpgFile;

	size_t pos = fileName.find_last_of(".");
	if (pos != std::string::npos)
		fileName = fileName.substr(0, pos);

	pos = fileName.find_last_of("/");
	if (pos != std::string::npos)
	{
		basePath = fileName.substr(0, pos+1);
		fileName = fileName.substr(pos+1);
	}

	std::string maskFile = basePath + fileName + "_alpha_mask";
	if (FileUtils::getInstance()->isFileExist(maskFile))
		return openImage(jpgFile, maskFile.c_str());
	else
		return Director::getInstance()->getTextureCache()->addImage(jpgFile);
#else
	return Director::getInstance()->getTextureCache()->addImage(jpgFile);
#endif
}

// 判断指定文件名是不是以".jpg"扩展名结尾
bool isJpgExtName(const char* jpgFile)
{
	if (nullptr == jpgFile || '\0' == *jpgFile)
		return false;

	std::string fileName = jpgFile, extName;

	size_t pos = fileName.find_last_of(".");
	if (pos != std::string::npos)
		extName = fileName.substr(pos);

	return (extName == ".jpg");
}

Sprite* createDotaSprite(const char* jpgFile)
{
	if (nullptr == jpgFile || '\0' == *jpgFile)
		return nullptr;

	if (isJpgExtName(jpgFile))
	{
		Texture2D* texture = openDotaImage(jpgFile);
		if (texture != nullptr)
			return Sprite::createWithTexture(texture);
		else
			return nullptr;
	}
	else
		return Sprite::create(jpgFile);
}

Scale9Sprite* createDotaScale9Sprite(const char* jpgFile)
{
	if (nullptr == jpgFile || '\0' == *jpgFile)
		return nullptr;

	if (isJpgExtName(jpgFile))
	{
		Texture2D* texture = openDotaImage(jpgFile);
		if (texture != nullptr)
		{
			Sprite* sprite = Sprite::createWithTexture(texture);
			Scale9Sprite* scale9Sprite = Scale9Sprite::create();
			scale9Sprite->updateWithSprite(sprite, Rect::ZERO, false, Rect::ZERO);
			return scale9Sprite;
		}
		else
			return nullptr;
	}
	else
		return Scale9Sprite::create(jpgFile);
}

ControlButton * createBtn(float x, float y, 
						  const char * normalBg, const char * pressBg, 
						  Ref* target, Control::Handler action)
{
	Scale9Sprite * spNormalBg = createDotaScale9Sprite(normalBg);
	Scale9Sprite * spPressBg = createDotaScale9Sprite(pressBg);

	ControlButton *btn = ControlButton::create(spNormalBg);
	if (spPressBg)
		btn->setBackgroundSpriteForState(spPressBg, Control::State::HIGH_LIGHTED);
	btn->setAnchorPoint(Vec2(0, 0));
	btn->setPosition(x, y);
	btn->setZoomOnTouchDown(false);
	btn->setPreferredSize(spNormalBg->getOriginalSize());
	if (target && action)
		btn->addTargetWithActionForControlEvents(target, action, 
			Control::EventType::TOUCH_UP_INSIDE);

	return btn;
}