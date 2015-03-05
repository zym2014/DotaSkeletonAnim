#pragma once

#include "cocos2d.h"
#include "cocos-ext.h"

void log(const char* str);
unsigned char* getFileDataFromZip(const char* zipFile, 
								  const char* fileName, 
								  ssize_t* size);
cocos2d::Texture2D* openImage(const char* jpgFile, const char* maskFile);
cocos2d::Texture2D* openDotaImage(const char* jpgFile);
cocos2d::Sprite* createDotaSprite(const char* jpgFile);
bool isJpgExtName(const char* jpgFile);		// 判断指定文件名是不是以".jpg"扩展名结尾
cocos2d::extension::Scale9Sprite* createDotaScale9Sprite(const char* jpgFile);
cocos2d::extension::ControlButton * createBtn(float x, float y, 
											  const char * normalBg, const char * pressBg, 
											  cocos2d::Ref* target, cocos2d::extension::Control::Handler action);