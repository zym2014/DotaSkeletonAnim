#include "DotaAnimParser.h"

USING_NS_DB;

std::string toString(int value)
{
	char buf[512] = {0};
	sprintf(buf, "%d", value);
	return buf;
}

std::string toString(float value)
{
	char buf[512] = {0};
	sprintf(buf, "%.2f", value);
	return buf;
}

std::string toString(bool value)
{
	return value ? "true" : "false";
}

std::string readName(ByteArray& byteArray)
{
	int nLen = byteArray.readInt();
	return byteArray.readString(nLen);
}

TextureData* getTextureData(ITextureAtlas& textureAtlas, const std::string& textureName)
{
	int size = (int)textureAtlas.textureAtlasData->textureDataList.size();
	for (int i = 0; i < size; i++)
	{
		if (textureAtlas.textureAtlasData->textureDataList[i]->name == textureName)
		{
			return textureAtlas.textureAtlasData->textureDataList[i];
		}
	}
	return nullptr;
}

std::string getBone(std::vector<Dota_Bone_Data *>& boneDataList, int index)
{
	for (int i = 0; i < (int)boneDataList.size(); i++)
	{
		Dota_Bone_Data * boneData = boneDataList[i];
		if (boneData != nullptr && index == boneData->index)
		{
			return boneData->name;
		}
	}
	return "";
}

DotaAnimParser::DotaAnimParser(void)
{
	//_textureScale = 1.f;
	_armatureScale = 1.f;
	_frameRate = 30;
}

DotaAnimParser::~DotaAnimParser(void)
{
}

// 解析出刀塔骨架数据
Dota_Skeleton_Data* DotaAnimParser::parseDotaSkeletonData(
	const std::string &fcaFile)
{
	const float PI = 3.14159265358979323846f;

	ByteArray byteArray;
	bool bRet = byteArray.loadFromZipFile(fcaFile.c_str(), "cha");
	if (!bRet)
		return nullptr;

	Dota_Skeleton_Data * skeletonData = new Dota_Skeleton_Data();
	if (skeletonData == nullptr)
	{
		byteArray.close();
		return nullptr;
	}

	skeletonData->name = readName(byteArray);
	CCLOG("RoleName:%s\n", skeletonData->name.c_str());

	float factor = 0.1f;
	if (skeletonData->name.substr(0, 14) == "effect/eff_UI_")
		factor = 0.5f;

	int boneCount = byteArray.readInt();
	CCLOG("BoneCnt:%d\n", boneCount);

	for (int i = 0; i < boneCount; i++)
	{
		Dota_Bone_Data * boneData = new Dota_Bone_Data();
		boneData->name = readName(byteArray);
		boneData->textureName = readName(byteArray);
		boneData->index = byteArray.readInt();
		skeletonData->boneDataList.push_back(boneData);

		CCLOG("BoneData:\nName:%s\nTextureName:%s\nIndex:%d\n\n", 
			boneData->name.c_str(), boneData->textureName.c_str(), boneData->index);
	}

	CCLOG("BoneCnt:%d\n", (int)skeletonData->boneDataList.size());

	int animCount = byteArray.readInt();
	CCLOG("AnimCnt:%d\n", animCount);

	for (int i = 0; i < animCount; i++)
	{
		Dota_Anim_Data * animData = new Dota_Anim_Data();
		animData->name = readName(byteArray);
		CCLOG("AnimName:%s\n", animData->name.c_str());

		byteArray.readBytes(animData->unknownData, sizeof(animData->unknownData));

		int frameCount = byteArray.readInt();
		CCLOG("FrameCnt:%d\n", frameCount);

		for (int j = 0; j < frameCount; j++)
		{
			Dota_Frame_Data * frameData = new Dota_Frame_Data();
			frameData->type = byteArray.readInt();
			if (frameData->type == 1)
			{
				frameData->unknownData1 = byteArray.readInt();
				CCLOG("UnknownData1:%d\n", frameData->unknownData1);
				frameData->soundName = readName(byteArray);
				byteArray.readBytes(frameData->soundData, sizeof(frameData->soundData));
				frameData->unknownData2 = byteArray.readInt();
				CCLOG("UnknownData2:%d\n", frameData->unknownData2);
			}
			int slotCount = byteArray.readInt();
			for (int k = 0; k < slotCount; k++)
			{
				Dota_Slot_Data * slotData = new Dota_Slot_Data();
				slotData->boneIndex = byteArray.readShort();
				slotData->opacity = byteArray.readByte();
				float a = byteArray.readFloat();
				float b = byteArray.readFloat();
				float c = byteArray.readFloat();
				float d = byteArray.readFloat();
				float tx = byteArray.readFloat();
				float ty = byteArray.readFloat();

				slotData->x = tx * factor;
				slotData->y = ty * factor;
				slotData->skX = atan2(-c, d) * (180/PI);
				slotData->skY = atan2(b, a) * (180/PI);
				slotData->scX = sqrt(a*a + b*b);
				slotData->scY = sqrt(c*c + d*d);
				frameData->slotDataList.push_back(slotData);
			}
			animData->frameDataList.push_back(frameData);
		}
		CCLOG("FrameCnt:%d\n", animData->frameDataList.size());
		skeletonData->animDataList.push_back(animData);
	}

	byteArray.close();

	for (int i = 0; i < (int)skeletonData->animDataList.size(); i++)
	{
		Dota_Anim_Data * animData = skeletonData->animDataList[i];

		Dota_Anim_Data2 * animData2 = new Dota_Anim_Data2();
		for (int j = 0; j < (int)animData->frameDataList.size(); j++)
		{
			Dota_Frame_Data * frameData = animData->frameDataList[j];

			for (int k = 0; k < (int)frameData->slotDataList.size(); k++)
			{
				Dota_Slot_Data * slotData = frameData->slotDataList[k];
				if (frameData->type == 1 && 0 == k)
					slotData->soundName = frameData->soundName;
				slotData->zOrder = k;		//slotData->boneIndex;
				std::string boneName = getBone(skeletonData->boneDataList, slotData->boneIndex);
				Dota_Anim_Data2::iterator iter = animData2->find(boneName);
				if (iter == animData2->end())
				{
					Dota_Timeline_Data * timelineData = new Dota_Timeline_Data();
					timelineData->insert(Dota_Timeline_Data::value_type(toString(j), slotData));
					animData2->insert(Dota_Anim_Data2::value_type(boneName, timelineData));
					skeletonData->firstFrameOfBoneMap.insert(Dota_First_Frame_Data::value_type(boneName, slotData));
				}
				else
				{
					Dota_Timeline_Data * timelineData = iter->second;
					Dota_Timeline_Data::iterator iter2 = timelineData->find(toString(j));
					if (iter2 == timelineData->end())
					{
						timelineData->insert(Dota_Timeline_Data::value_type(toString(j), slotData));
					}
				}
			}
		}
		skeletonData->animDataMap.insert(std::map<std::string, Dota_Anim_Data2 *>::value_type(animData->name, animData2));
	}

	return skeletonData;
}

// 解析出DB纹理集数据
TextureAtlasData* DotaAnimParser::parseTextureAtlasData(
	const std::string &fcaFile, float scale/* = 1.f*/)
{
	ssize_t size = 0;
	char* data = (char *)getFileDataFromZip(fcaFile.c_str(), "plist", &size);
	if (nullptr == data || size <= 0)
	{
		free(data);
		return nullptr;
	}

	TextureAtlasData *textureAtlasData = new TextureAtlasData();
	textureAtlasData->name = "";
	textureAtlasData->imagePath = "sheet.pvr";

	cocos2d::ValueMap dict = cocos2d::FileUtils::getInstance()->getValueMapFromData(data, size);
	free(data);

	cocos2d::ValueMap& framesDict = dict["frames"].asValueMap();
	int format = 0;

	if (dict.find("metadata") != dict.end())
	{
		cocos2d::ValueMap& metadataDict = dict["metadata"].asValueMap();
		format = metadataDict["format"].asInt();
	}

	for (auto iter = framesDict.begin(); iter != framesDict.end(); ++iter)
	{
		TextureData *textureData = new TextureData();
		textureData->name = iter->first;

		cocos2d::ValueMap& frameDict = iter->second.asValueMap();

		if(format == 0) 
		{
			float x = frameDict["x"].asFloat();
			float y = frameDict["y"].asFloat();
			float w = frameDict["width"].asFloat();
			float h = frameDict["height"].asFloat();
			float ox = frameDict["offsetX"].asFloat();
			float oy = frameDict["offsetY"].asFloat();
			int ow = frameDict["originalWidth"].asInt();
			int oh = frameDict["originalHeight"].asInt();
			// check ow/oh
			if(!ow || !oh)
			{
				//CCLOGWARN("cocos2d: WARNING: originalWidth/Height not found on the SpriteFrame. AnchorPoint won't work as expected. Regenrate the .plist");
			}
			// abs ow/oh
			ow = abs(ow);
			oh = abs(oh);

			textureData->rotated = false;
			textureData->region.x = x / scale;
			textureData->region.y = y / scale;
			textureData->region.width = w / scale;
			textureData->region.height = h / scale;
		} 
		else if(format == 1 || format == 2) 
		{
			cocos2d::Rect frame = cocos2d::RectFromString(frameDict["frame"].asString());
			bool rotated = false;

			// rotation
			if (format == 2)
			{
				rotated = frameDict["rotated"].asBool();
			}

			cocos2d::Vec2 offset = cocos2d::PointFromString(frameDict["offset"].asString());
			cocos2d::Size sourceSize = cocos2d::SizeFromString(frameDict["sourceSize"].asString());

			textureData->rotated = rotated;
			textureData->region.x = frame.origin.x / scale;
			textureData->region.y = frame.origin.y / scale;
			
			if (rotated)
			{
				textureData->region.width = frame.size.height / scale;
				textureData->region.height = frame.size.width / scale;
			}
			else
			{
				textureData->region.width = frame.size.width / scale;
				textureData->region.height = frame.size.height / scale;
			}
		} 
		else if (format == 3)
		{
			// get values
			cocos2d::Size spriteSize = cocos2d::SizeFromString(frameDict["spriteSize"].asString());
			cocos2d::Vec2 spriteOffset = cocos2d::PointFromString(frameDict["spriteOffset"].asString());
			cocos2d::Size spriteSourceSize = cocos2d::SizeFromString(frameDict["spriteSourceSize"].asString());
			cocos2d::Rect textureRect = cocos2d::RectFromString(frameDict["textureRect"].asString());
			bool textureRotated = frameDict["textureRotated"].asBool();

			textureData->rotated = textureRotated;
			textureData->region.x = textureRect.origin.x / scale;
			textureData->region.y = textureRect.origin.y / scale;
			if (textureRotated)
			{
				textureData->region.width = spriteSize.height / scale;
				textureData->region.height = spriteSize.width / scale;
			}
			else
			{
				textureData->region.width = spriteSize.width / scale;
				textureData->region.height = spriteSize.height / scale;
			}
		}

		textureAtlasData->textureDataList.push_back(textureData);
	}

	return textureAtlasData;
}

// 解析出DB骨骼数据
DragonBonesData* DotaAnimParser::parseDragonBonesData(
	Dota_Skeleton_Data& dotaSkeletonData,
	ITextureAtlas& textureAtlas, float scale/* = 1.f*/)
{
	_armatureScale = scale;
    _frameRate = 24;

    DragonBonesData *dragonBonesData = new DragonBonesData();
    dragonBonesData->name = dotaSkeletonData.name;
    
	ArmatureData *armatureData = new ArmatureData();
	armatureData->name = dotaSkeletonData.name;

	for (int i = 0; i < (int)dotaSkeletonData.boneDataList.size(); i++)
	{
		Dota_Bone_Data * dotaBoneData = dotaSkeletonData.boneDataList[i];

		BoneData *boneData = new BoneData();

		boneData->name = dotaBoneData->name;

		boneData->length = 0;
		boneData->inheritRotation = true;
		boneData->inheritScale = false;

		Dota_Slot_Data * slotData = nullptr;
		Dota_First_Frame_Data::iterator iter;
		iter = dotaSkeletonData.firstFrameOfBoneMap.find(dotaBoneData->name);
		if (iter != dotaSkeletonData.firstFrameOfBoneMap.end())
			slotData = iter->second;

		if (slotData != nullptr)
			setTransform(boneData->global, slotData->x, slotData->y, 
				slotData->skX, slotData->skY, slotData->scX, slotData->scY);
		else
			setTransform(boneData->global, 0, 0, 0, 0, 1, 1);
		
		boneData->transform = boneData->global;

		armatureData->boneDataList.push_back(boneData);
	}

	SkinData *skinData = new SkinData();
	skinData->name = "default";

	for (int i = 0; i < (int)dotaSkeletonData.boneDataList.size(); i++)
	{
		Dota_Bone_Data * dotaBoneData = dotaSkeletonData.boneDataList[i];

		SlotData *slotData = new SlotData();
		slotData->name = dotaBoneData->name;
		slotData->parent = dotaBoneData->name;
		slotData->zOrder = dotaBoneData->index;

		DisplayData *displayData = new DisplayData();
		displayData->name = dotaBoneData->textureName + ".png";
		displayData->type = DisplayType::DT_IMAGE;
		displayData->scalingGrid = false;
		
		setTransform(displayData->transform, 0, 0, 0, 0, 1, 1);
		
		TextureData * textureData = getTextureData(textureAtlas, displayData->name);
		if (textureData->rotated)
		{
			displayData->pivot.x = textureData->region.height / 2 / _armatureScale;
			displayData->pivot.y = textureData->region.width / 2 / _armatureScale;
		}
		else
		{
			displayData->pivot.x = textureData->region.width / 2 / _armatureScale;
			displayData->pivot.y = textureData->region.height / 2 / _armatureScale;
		}

		slotData->displayDataList.push_back(displayData);
		skinData->slotDataList.push_back(slotData);
	}
	armatureData->skinDataList.push_back(skinData);

	transformArmatureData(armatureData);
	armatureData->sortBoneDataList();

	for (int i = 0; i < (int)dotaSkeletonData.animDataList.size(); i++)
	{
		Dota_Anim_Data * dotaAnimData = dotaSkeletonData.animDataList[i];

		AnimationData *animationData = new AnimationData();
		animationData->name = dotaAnimData->name;
		animationData->frameRate = _frameRate;
		animationData->duration = (int)(round((int)dotaAnimData->frameDataList.size() * 1000.f / _frameRate));
		animationData->playTimes = 0;
		animationData->fadeTime = 0.f;	//0.3f;
		animationData->scale = 1;
		// use frame tweenEase, NaN
		// overwrite frame tweenEase, [-1, 0):ease in, 0:line easing, (0, 1]:ease out, (1, 2]:ease in out
		animationData->tweenEasing = USE_FRAME_TWEEN_EASING;
		animationData->autoTween = true;

		parseTimeline(*animationData);


		std::map<std::string, Dota_Anim_Data2 *>::iterator iter;
		iter = dotaSkeletonData.animDataMap.find(dotaAnimData->name);
		if (iter == dotaSkeletonData.animDataMap.end())
			continue;

		Dota_Anim_Data2 * dotaAnimData2 = iter->second;

		Dota_Anim_Data2::iterator iter2;
		for (iter2 = dotaAnimData2->begin(); iter2 != dotaAnimData2->end(); iter2++)
		{
			Dota_Timeline_Data * dataTimelineData2 = iter2->second;

			TransformTimeline *timeline = new TransformTimeline();
			timeline->name = iter2->first;
			timeline->scale = 1;
			timeline->offset = 0;
			timeline->duration = animationData->duration;

			int nBlankKeyframes = 0;
			for (int j = 0; j < (int)dotaAnimData->frameDataList.size(); j++)
			{	
				Dota_Timeline_Data::iterator iter3 = dataTimelineData2->find(toString(j));
				if (iter3 != dataTimelineData2->end())
				{
					Dota_Slot_Data * slotData = iter3->second;

					if (nBlankKeyframes > 0)
					{
						TransformFrame *frame = newBlankKeyframes(nBlankKeyframes, slotData->soundName);
						timeline->frameList.push_back(frame);
						nBlankKeyframes = 0;
					}

					TransformFrame *frame = newKeyframes(slotData->zOrder, 
						slotData->opacity, slotData->x, slotData->y,
						slotData->skX, slotData->skY, slotData->scX, slotData->scY, 
						slotData->soundName);
					timeline->frameList.push_back(frame);
				}
				else
				{
					nBlankKeyframes++;
				}
			}

			if (nBlankKeyframes > 0)
			{
				TransformFrame *frame = newBlankKeyframes(nBlankKeyframes, "");
				timeline->frameList.push_back(frame);
				nBlankKeyframes = 0;
			}

			parseTimeline(*timeline);

			animationData->timelineList.push_back(timeline);
		}

		addHideTimeline(animationData, armatureData);
		transformAnimationData(animationData, armatureData);

		armatureData->animationDataList.push_back(animationData);
	}

	dragonBonesData->armatureDataList.push_back(armatureData);

    return dragonBonesData;
}

// 导出DragonBones的配置文件(sheet.pvr、texture.xml、skeleton.xml)
bool DotaAnimParser::exportDBConfigFile(const std::string &fcaFile, const std::string &savePath)
{
	Dota_Skeleton_Data* skeletonData = parseDotaSkeletonData(fcaFile);
	if (nullptr == skeletonData)
		return false;

	DBCCTextureAtlasEx* textureAtlas = new DBCCTextureAtlasEx();
	if (nullptr == textureAtlas)
	{
		delete skeletonData;
		return false;
	}
	textureAtlas->_fcaFile = fcaFile;
	textureAtlas->textureAtlasData = parseTextureAtlasData(fcaFile);
	if (nullptr == textureAtlas->textureAtlasData)
	{
		delete skeletonData;
		return false;
	}
	textureAtlas->textureAtlasData->name = skeletonData->name;

	exportDBTexturePic(savePath, fcaFile);
	exportDBTextureData(savePath, *textureAtlas);
	exportDBSkeletonData(savePath, *skeletonData, *textureAtlas);

	delete textureAtlas;
	delete skeletonData;

	return true;
}

void DotaAnimParser::parseTimeline(Timeline &timeline)
{
	int position = 0;
	Frame *frame = nullptr;

	for (size_t i = 0, l = timeline.frameList.size(); i < l; ++i)
	{
		frame = timeline.frameList[i];
		frame->position = position;
		position += frame->duration;
	}

	if (frame)
	{
		frame->duration = timeline.duration - frame->position;
	}
}

void DotaAnimParser::addHideTimeline(AnimationData *animationData, const ArmatureData *armatureData)
{
	for (size_t i = 0, l = armatureData->boneDataList.size(); i < l; ++i)
	{
		const BoneData *boneData = armatureData->boneDataList[i];

		if (!animationData->getTimeline(boneData->name))
		{
			auto iterator = std::find(animationData->hideTimelineList.cbegin(), animationData->hideTimelineList.cend(), boneData->name);

			if (iterator != animationData->hideTimelineList.cend())
			{
				continue;
			}

			animationData->hideTimelineList.push_back(boneData->name);
		}
	}
}

void DotaAnimParser::transformAnimationData(AnimationData *animationData, const ArmatureData *armatureData)
{
	SkinData *skinData = armatureData->getSkinData("");

	for (size_t i = 0, l = armatureData->boneDataList.size(); i < l; ++i)
	{
		const BoneData *boneData = armatureData->boneDataList[i];
		TransformTimeline *timeline = animationData->getTimeline(boneData->name);

		if (!timeline)
		{
			continue;
		}

		SlotData *slotData = nullptr;

		if (skinData)
		{
			for (size_t i = 0, l = skinData->slotDataList.size(); i < l; ++i)
			{
				slotData = skinData->slotDataList[i];

				if (slotData->parent == boneData->name)
				{
					break;
				}
			}
		}

		Transform *originTransform = nullptr;
		Point *originPivot = nullptr;
		TransformFrame *prevFrame = nullptr;

		for (size_t i = 0, l = timeline->frameList.size(); i < l; ++i)
		{
			TransformFrame *frame = static_cast<TransformFrame*>(timeline->frameList[i]);
			setFrameTransform(animationData, armatureData, boneData, frame);
			frame->transform.x -= boneData->transform.x;
			frame->transform.y -= boneData->transform.y;
			frame->transform.skewX -= boneData->transform.skewX;
			frame->transform.skewY -= boneData->transform.skewY;
			frame->transform.scaleX -= boneData->transform.scaleX;
			frame->transform.scaleY -= boneData->transform.scaleY;

			if (!timeline->transformed && slotData)
			{
				frame->zOrder -= slotData->zOrder;
			}

			if (!originTransform)
			{
				// copy
				timeline->originTransform = frame->transform;
				originTransform = &timeline->originTransform;
				originTransform->skewX = formatRadian(originTransform->skewX);
				originTransform->skewY = formatRadian(originTransform->skewY);
				// copy
				timeline->originPivot = frame->pivot;
				originPivot = &timeline->originPivot;
			}

			frame->transform.x -= originTransform->x;
			frame->transform.y -= originTransform->y;
			frame->transform.skewX = formatRadian(frame->transform.skewX - originTransform->skewX);
			frame->transform.skewY = formatRadian(frame->transform.skewY - originTransform->skewY);
			frame->transform.scaleX -= originTransform->scaleX;
			frame->transform.scaleY -= originTransform->scaleY;

			if (!timeline->transformed)
			{
				frame->pivot.x -= originPivot->x;
				frame->pivot.y -= originPivot->y;
			}

			if (prevFrame)
			{
				const float dLX = frame->transform.skewX - prevFrame->transform.skewX;

				if (prevFrame->tweenRotate)
				{
					if (prevFrame->tweenRotate > 0)
					{
						if (dLX < 0)
						{
							frame->transform.skewX += PI * 2;
							frame->transform.skewY += PI * 2;
						}

						if (prevFrame->tweenRotate > 1)
						{
							frame->transform.skewX += PI * 2 * (prevFrame->tweenRotate - 1);
							frame->transform.skewY += PI * 2 * (prevFrame->tweenRotate - 1);
						}
					}
					else
					{
						if (dLX > 0)
						{
							frame->transform.skewX -= PI * 2;
							frame->transform.skewY -= PI * 2;
						}

						if (prevFrame->tweenRotate < 1)
						{
							frame->transform.skewX += PI * 2 * (prevFrame->tweenRotate + 1);
							frame->transform.skewY += PI * 2 * (prevFrame->tweenRotate + 1);
						}
					}
				}
				else
				{
					frame->transform.skewX = prevFrame->transform.skewX + formatRadian(frame->transform.skewX - prevFrame->transform.skewX);
					frame->transform.skewY = prevFrame->transform.skewY + formatRadian(frame->transform.skewY - prevFrame->transform.skewY);
				}
			}

			prevFrame = frame;
		}

		timeline->transformed = true;
	}
}

void DotaAnimParser::transformArmatureData(ArmatureData *armatureData)
{
	for (size_t i = armatureData->boneDataList.size(); i--;)
	{
		BoneData *boneData = armatureData->boneDataList[i];

		if (boneData && !boneData->parent.empty())
		{
			const BoneData *parentBoneData = armatureData->getBoneData(boneData->parent);

			if (parentBoneData)
			{
				boneData->transform = boneData->global;
				boneData->transform.transformWith(parentBoneData->global);
			}
		}
	}
}

void DotaAnimParser::setFrameTransform(AnimationData *animationData, const ArmatureData *armatureData, const BoneData *boneData, TransformFrame *frame)
{
	frame->transform = frame->global;
	BoneData *parentData = armatureData->getBoneData(boneData->parent);

	if (parentData)
	{
		TransformTimeline *parentTimeline = animationData->getTimeline(parentData->name);

		if (parentTimeline)
		{
			std::vector<TransformTimeline*> parentTimelineList;
			std::vector<BoneData*> parentDataList;

			while (parentTimeline)
			{
				parentTimelineList.push_back(parentTimeline);
				parentDataList.push_back(parentData);
				parentData = armatureData->getBoneData(parentData->parent);

				if (parentData)
				{
					parentTimeline = animationData->getTimeline(parentData->name);
				}
				else
				{
					parentTimeline = nullptr;
				}
			}

			Matrix helpMatrix;
			Transform currentTransform;
			Transform *globalTransform = nullptr;

			for (size_t i = parentTimelineList.size(); i--;)
			{
				parentTimeline = parentTimelineList[i];
				parentData = parentDataList[i];
				getTimelineTransform(parentTimeline, frame->position, &currentTransform, !globalTransform);

				if (globalTransform)
				{
					//if(inheritRotation)
					//{
					globalTransform->skewX += currentTransform.skewX + parentTimeline->originTransform.skewX + parentData->transform.skewX;
					globalTransform->skewY += currentTransform.skewY + parentTimeline->originTransform.skewY + parentData->transform.skewY;
					//}
					//if(inheritScale)
					//{
					//  globalTransform.scaleX *= currentTransform.scaleX + parentTimeline.originTransform.scaleX;
					//  globalTransform.scaleY *= currentTransform.scaleY + parentTimeline.originTransform.scaleY;
					//}
					//else
					//{
					globalTransform->scaleX = currentTransform.scaleX + parentTimeline->originTransform.scaleX + parentData->transform.scaleX;
					globalTransform->scaleY = currentTransform.scaleY + parentTimeline->originTransform.scaleY + parentData->transform.scaleY;
					//}
					const float x = currentTransform.x + parentTimeline->originTransform.x + parentData->transform.x;
					const float y = currentTransform.y + parentTimeline->originTransform.y + parentData->transform.y;
					globalTransform->x = helpMatrix.a * x + helpMatrix.c * y + helpMatrix.tx;
					globalTransform->y = helpMatrix.d * y + helpMatrix.b * x + helpMatrix.ty;
				}
				else
				{
					globalTransform = new Transform();
					*globalTransform = currentTransform;
				}

				globalTransform->toMatrix(helpMatrix, true);
			}

			frame->transform.transformWith(*globalTransform);

			if (globalTransform)
			{
				delete globalTransform;
				globalTransform = nullptr;
			}
		}
	}
}

void DotaAnimParser::getTimelineTransform(const TransformTimeline *timeline, int position, Transform *retult, bool isGlobal)
{
	for (size_t i = 0, l = timeline->frameList.size(); i < l; ++i)
	{
		const TransformFrame *currentFrame = static_cast<const TransformFrame*>(timeline->frameList[i]);

		if (currentFrame->position <= position && currentFrame->position + currentFrame->duration > position)
		{
			if (i == timeline->frameList.size() - 1 || position == currentFrame->position)
			{
				//copy
				*retult = isGlobal ? currentFrame->global : currentFrame->transform;
			}
			else
			{
				float progress = (position - currentFrame->position) / (float)(currentFrame->duration);
				const float tweenEasing = currentFrame->tweenEasing;

				if (tweenEasing && tweenEasing != NO_TWEEN_EASING && tweenEasing != AUTO_TWEEN_EASING)
				{
					progress = getEaseValue(progress, tweenEasing);
				}

				const TransformFrame *nextFrame = static_cast<const TransformFrame*>(timeline->frameList[i + 1]);
				const Transform *currentTransform = isGlobal ? &currentFrame->global : &currentFrame->transform;
				const Transform *nextTransform = isGlobal ? &nextFrame->global : &nextFrame->transform;
				retult->x = currentTransform->x + (nextTransform->x - currentTransform->x) * progress;
				retult->y = currentTransform->y + (nextTransform->y - currentTransform->y) * progress;
				retult->skewX = formatRadian(currentTransform->skewX + (nextTransform->skewX - currentTransform->skewX) * progress);
				retult->skewY = formatRadian(currentTransform->skewY + (nextTransform->skewY - currentTransform->skewY) * progress);
				retult->scaleX = currentTransform->scaleX + (nextTransform->scaleX - currentTransform->scaleX) * progress;
				retult->scaleY = currentTransform->scaleY + (nextTransform->scaleY - currentTransform->scaleY) * progress;
			}

			break;
		}
	}
}

TransformFrame * DotaAnimParser::newBlankKeyframes(int duration, const std::string& sound)
{
	TransformFrame *frame = new TransformFrame();
	frame->duration = (int)(round(duration * 1000.f / _frameRate));
	if (!sound.empty())
		frame->sound = sound;

	frame->visible = true;
	// NaN:no tween, 10:auto tween, [-1, 0):ease in, 0:line easing, (0, 1]:ease out, (1, 2]:ease in out
	frame->tweenEasing = AUTO_TWEEN_EASING;
	frame->tweenRotate = 0;
	frame->tweenScale = true;
	frame->displayIndex = -1;
	frame->zOrder = 0;

	// copy
	frame->transform = frame->global;
	frame->scaleOffset.x = 0.f;
	frame->scaleOffset.y = 0.f;

	return frame;
}

TransformFrame * DotaAnimParser::newKeyframes(int zOrder, unsigned char opacity,
											  float x, float y, float skX, 
											  float skY, float scX, float scY, 
											  const std::string& sound)
{
	TransformFrame *frame = new TransformFrame();
	frame->duration = (int)(round(1 * 1000.f / _frameRate));
	if (!sound.empty())
		frame->sound = sound;

	frame->visible = true;
	// NaN:no tween, 10:auto tween, [-1, 0):ease in, 0:line easing, (0, 1]:ease out, (1, 2]:ease in out
	frame->tweenEasing = AUTO_TWEEN_EASING;
	frame->tweenRotate = 0;
	frame->tweenScale = true;
	frame->displayIndex = 0;
	frame->zOrder = zOrder;

	setTransform(frame->global, x, y, skX, skY, scX, scY);
	setPivot(frame->pivot, 0, 0);

	// copy
	frame->transform = frame->global;
	frame->scaleOffset.x = 0.f;
	frame->scaleOffset.y = 0.f;

	if (opacity != 255)
	{
		frame->color = new ColorTransform();
		setColorTransform(*frame->color, opacity);
	}

	return frame;
}

void DotaAnimParser::setTransform(Transform &transform, float x, float y, 
								  float skX, float skY, float scX, float scY)
{
	transform.x = x / _armatureScale;
	transform.y = y / _armatureScale;
	transform.skewX = skX * ANGLE_TO_RADIAN;
	transform.skewY = skY * ANGLE_TO_RADIAN;
	transform.scaleX = scX;
	transform.scaleY = scY;
}

void DotaAnimParser::setPivot(Point &pivot, float pX, float pY)
{
	pivot.x = pX / _armatureScale;
	pivot.y = pY / _armatureScale;
}

void DotaAnimParser::setColorTransform(ColorTransform &colorTransform, unsigned char opacity)
{
	colorTransform.alphaOffset = 0;
	colorTransform.redOffset = 0;
	colorTransform.greenOffset = 0;
	colorTransform.blueOffset = 0;
	colorTransform.alphaMultiplier = ((int)((float)opacity/255*100)) * 0.01f;
	colorTransform.redMultiplier = 100 * 0.01f;
	colorTransform.greenMultiplier = 100 * 0.01f;
	colorTransform.blueMultiplier = 100 * 0.01f;
}

// 导出DragonBones的纹理图片(sheet.pvr)
void DotaAnimParser::exportDBTexturePic(const std::string &savePath, 
										const std::string &fcaFile)
{
	ssize_t size = 0;
	unsigned char * data = getFileDataFromZip(fcaFile.c_str(), "sheet.pvr", &size);
	if (data != nullptr && size > 0)
	{
		std::string fileName = savePath + "/sheet.pvr";
		FILE* fp = fopen(fileName.c_str(), "wb");
		if (fp)
		{
			fwrite(data, size, 1, fp);
			fclose(fp);
		}		
	}

	if (data != nullptr)
		free(data);
}

// 导出DragonBones的纹理数据(texture.xml)
void DotaAnimParser::exportDBTextureData(const std::string &savePath, 
										 ITextureAtlas& textureAtlas)
{
	std::string strXml = "<TextureAtlas name=\"";
	strXml += textureAtlas.textureAtlasData->name;
	strXml += "\" imagePath=\"sheet.pvr\">\n";

	for (int i = 0; i < (int)textureAtlas.textureAtlasData->textureDataList.size(); i++)
	{
		TextureData* textureData = textureAtlas.textureAtlasData->textureDataList[i];

		strXml += "<SubTexture name=\"";
		strXml += textureData->name;
		strXml += "\" x=\"";
		strXml += toString((int)textureData->region.x);
		strXml += "\" y=\"";
		strXml += toString((int)textureData->region.y);
		strXml += "\" width=\"";
		strXml += toString((int)textureData->region.width);
		strXml += "\" height=\"";
		strXml += toString((int)textureData->region.height);
		strXml += "\" rotated=\"";
		strXml += toString(textureData->rotated);
		strXml += "\" />\n";
	}
	strXml += "</TextureAtlas>\n";

	std::string fileName = savePath + "/texture.xml";
	FILE* fp = fopen(fileName.c_str(), "wb");
	if (fp)
	{
		fwrite(strXml.c_str(), strXml.size(), 1, fp);
		fclose(fp);
	}
}

// 导出DragonBones的骨骼数据(skeleton.xml)
void DotaAnimParser::exportDBSkeletonData(const std::string &savePath,
										  Dota_Skeleton_Data& dotaSkeletonData,
										  ITextureAtlas& textureAtlas)
{
	std::string strXml = "<dragonBones name=\"";
	strXml += dotaSkeletonData.name;
	strXml += "\" frameRate=\"24\" version=\"2.3\">\n";
	strXml += " <armature name=\"";
	strXml += dotaSkeletonData.name;
	strXml += "\">\n";

	for (int i = 0; i < (int)dotaSkeletonData.boneDataList.size(); i++)
	{
		Dota_Bone_Data * boneData = dotaSkeletonData.boneDataList[i];

		strXml += "  <bone name=\"";
		strXml += boneData->name;
		strXml += "\">\n";

		Dota_Slot_Data * slotData = nullptr;
		Dota_First_Frame_Data::iterator iter;
		iter = dotaSkeletonData.firstFrameOfBoneMap.find(boneData->name);
		if (iter != dotaSkeletonData.firstFrameOfBoneMap.end())
			slotData = iter->second;

		float x, y, skX, skY, scX, scY;
		if (slotData != nullptr)
		{
			x = slotData->x;
			y = slotData->y;
			skX = slotData->skX;
			skY = slotData->skY;
			scX = slotData->scX;
			scY = slotData->scY;
		}
		else
		{
			x = 0.f;
			y = 0.f;
			skX = 0.f;
			skY = 0.f;
			scX = 1.f;
			scY = 1.f;
		}

		strXml += "   <transform x=\"";
		strXml += toString(x);
		strXml += "\" y=\"";
		strXml += toString(y);
		strXml += "\" skX=\"";
		strXml += toString(skX);
		strXml += "\" skY=\"";
		strXml += toString(skY);
		strXml += "\" scX=\"";
		strXml += toString(scX);
		strXml += "\" scY=\"";
		strXml += toString(scY);
		strXml += "\"/>\n";

		strXml += "  </bone>\n";
	}

	strXml += "  <skin name=\"default\">\n";
	for (int i = 0; i < (int)dotaSkeletonData.boneDataList.size(); i++)
	{
		Dota_Bone_Data * boneData = dotaSkeletonData.boneDataList[i];

		strXml += "   <slot name=\"";
		strXml += boneData->name;
		strXml += "\" parent=\"";
		strXml += boneData->name;
		strXml += "\" z=\"";
		strXml += toString(boneData->index);
		strXml += "\">\n";

		strXml += "    <display name=\"";
		strXml += boneData->textureName+".png";
		strXml += "\" type=\"image\">\n";

		float width = 0, height = 0;
		TextureData * textureData = getTextureData(textureAtlas, boneData->textureName+".png");
		if (textureData != nullptr)
		{
			if (textureData->rotated)
			{
				width = textureData->region.height;
				height = textureData->region.width;
			}
			else
			{
				width = textureData->region.width;
				height = textureData->region.height;
			}
		}
		else
		{
			int xxx = 0;
			xxx++;
		}

		strXml += "     <transform x=\"";
		strXml += toString(0);
		strXml += "\" y=\"";
		strXml += toString(0);
		strXml += "\" skX=\"";
		strXml += toString(0);
		strXml += "\" skY=\"";
		strXml += toString(0);
		strXml += "\" scX=\"";
		strXml += toString(1);
		strXml += "\" scY=\"";
		strXml += toString(1);
		strXml += "\" pX=\"";
		strXml += toString(width/2);
		strXml += "\" pY=\"";
		strXml += toString(height/2);
		strXml += "\"/>\n";

		strXml += "    </display>\n";

		strXml += "   </slot>\n";
	}
	strXml += "  </skin>\n";

	for (int i = 0; i < (int)dotaSkeletonData.animDataList.size(); i++)
	{
		Dota_Anim_Data * animData = dotaSkeletonData.animDataList[i];

		std::map<std::string, Dota_Anim_Data2 *>::iterator iter;
		iter = dotaSkeletonData.animDataMap.find(animData->name);
		if (iter == dotaSkeletonData.animDataMap.end())
			continue;

		Dota_Anim_Data2 * animData2 = iter->second;

		int frameCount = (int)animData->frameDataList.size();

		strXml += "  <animation name=\"";
		strXml += iter->first;
		strXml += "\" duration=\"";
		strXml += toString(frameCount);
		strXml += "\" fadeInTime=\"0.3\" scale=\"1\" loop=\"0\" autoTween=\"1\" tweenEasing=\"NaN\">\n";

		Dota_Anim_Data2::iterator iter2;
		for (iter2 = animData2->begin(); iter2 != animData2->end(); iter2++)
		{
			strXml += "   <timeline name=\"";
			strXml += iter2->first;
			strXml += "\" scale=\"1\" offset=\"0\">\n";

			Dota_Timeline_Data * timelineData = iter2->second;

			int nBlankKeyframes = 0;
			for (int j = 0; j < frameCount; j++)
			{
				Dota_Timeline_Data::iterator iter3 = timelineData->find(toString(j));
				if (iter3 != timelineData->end())
				{
					Dota_Slot_Data * slotData = iter3->second;

					if (nBlankKeyframes > 0)
					{
						strXml += "    <frame duration=\"";
						strXml += toString(nBlankKeyframes);
						if (!slotData->soundName.empty())
						{
							strXml += "\" sound=\"";
							strXml += slotData->soundName;
						}
						strXml += "\" displayIndex=\"-1\"/>\n";
						nBlankKeyframes = 0;
					}

					strXml += "    <frame z=\"";
					strXml += toString(slotData->zOrder);
					if (!slotData->soundName.empty())
					{
						strXml += "\" sound=\"";
						strXml += slotData->soundName;
					}
					strXml += "\" duration=\"1\">\n";

					strXml += "     <transform x=\"";
					strXml += toString(slotData->x);
					strXml += "\" y=\"";
					strXml += toString(slotData->y);
					strXml += "\" skX=\"";
					strXml += toString(slotData->skX);
					strXml += "\" skY=\"";
					strXml += toString(slotData->skY);
					strXml += "\" scX=\"";
					strXml += toString(slotData->scX);
					strXml += "\" scY=\"";
					strXml += toString(slotData->scY);
					strXml += "\" pX=\"";
					strXml += toString(0);
					strXml += "\" pY=\"";
					strXml += toString(0);
					strXml += "\"/>\n";

					if (slotData->opacity != 255)
					{
						strXml += "     <colorTransform aO=\"0\" rO=\"0\" gO=\"0\" bO=\"0\" aM=\"";
						strXml += toString((int)((float)slotData->opacity/255*100));
						strXml += "\" rM=\"100\" gM=\"100\" bM=\"100\"/>\n";
					}

					strXml += "    </frame>\n";
				}
				else
				{
					nBlankKeyframes++;
				}
			}

			if (nBlankKeyframes > 0)
			{
				strXml += "    <frame duration=\"";
				strXml += toString(nBlankKeyframes);
				strXml += "\" displayIndex=\"-1\"/>\n";
				nBlankKeyframes = 0;
			}

			strXml += "   </timeline>\n";
		}

		strXml += "  </animation>\n";
	}

	strXml += " </armature>\n";
	strXml += "</dragonBones>\n";

	std::string fileName = savePath + "/skeleton.xml";
	FILE* fp = fopen(fileName.c_str(), "wb");
	if (fp)
	{
		fwrite(strXml.c_str(), strXml.size(), 1, fp);
		fclose(fp);
	}
}