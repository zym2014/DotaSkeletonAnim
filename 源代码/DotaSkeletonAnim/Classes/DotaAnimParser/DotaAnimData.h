#include <string>
#include <vector>
#include <map>

class Dota_Bone_Data			// 骨头数据
{
public:
	std::string name;			// 骨头名称
	std::string textureName;	// 骨头使用的纹理名称
	int index;					// 骨头索引
};

class Dota_Slot_Data			// 槽数据
{
public:
	short boneIndex;			// 骨头索引
	unsigned char opacity;		// 透明度
	float x, y;					// 坐标点
	float skX, skY;				// 倾斜系数
	float scX, scY;				// 缩放系数
	int zOrder;					// 转换格式过程中用来暂存的骨头所在的z层索引
	std::string soundName;		// 转换格式过程中用来暂存的声音文件名称
};

class Dota_Frame_Data			// 动画帧数据
{
public:
	Dota_Frame_Data(void){}
	~Dota_Frame_Data(void)
	{
		for (int i = 0; i < (int)slotDataList.size(); i++)
		{
			delete slotDataList[i];
		}
		slotDataList.clear();
	}

public:
	int type;									// 动画帧类型
	int unknownData1;							// 未知数据1
	std::string soundName;						// 声音文件名称
	unsigned char soundData[32];				// 未知声音数据
	int unknownData2;							// 未知数据2
	std::vector<Dota_Slot_Data *> slotDataList;	// 槽数据列表
};

class Dota_Anim_Data		// 动画数据
{
public:
	Dota_Anim_Data(void){}
	~Dota_Anim_Data(void)
	{
		for (int i = 0; i < (int)frameDataList.size(); i++)
		{
			delete frameDataList[i];
		}
		frameDataList.clear();
	}

public:
	std::string name;								// 动画名称
	unsigned char unknownData[4];					// 未知数据
	std::vector<Dota_Frame_Data *> frameDataList;	// 动画帧数据列表
};

typedef std::map<std::string, Dota_Slot_Data *> Dota_Timeline_Data;		// 时间轴<第几帧，槽数据>
typedef std::map<std::string, Dota_Timeline_Data *> Dota_Anim_Data2;	// 动画数据2<骨头名，时间轴>
typedef std::map<std::string, Dota_Slot_Data *> Dota_First_Frame_Data;	// 存储每个骨头的第一帧数据<骨头名，槽数据>

class Dota_Skeleton_Data	// 骨架数据
{
public:
	Dota_Skeleton_Data(void){}
	~Dota_Skeleton_Data(void)
	{
		for (int i = 0; i < (int)boneDataList.size(); i++)
		{
			delete boneDataList[i];
		}
		boneDataList.clear();

		for (int i = 0; i < (int)animDataList.size(); i++)
		{
			delete animDataList[i];
		}
		animDataList.clear();

		std::map<std::string, Dota_Anim_Data2 *>::iterator iter;
		for (iter = animDataMap.begin(); iter != animDataMap.end(); iter++)
		{
			Dota_Anim_Data2 * animData2 = iter->second;
			if (nullptr == animData2)
				continue;

			Dota_Anim_Data2::iterator iter2;
			for (iter2 = animData2->begin(); iter2 != animData2->end(); iter2++)
			{
				Dota_Timeline_Data * timelineData = iter2->second;

				// 				Dota_Timeline_Data::iterator iter3;
				// 				for (iter3 = timelineData->begin(); iter3 != timelineData->end(); iter3++)
				// 				{
				// 					delete iter3->second;
				// 				}

				timelineData->clear();
				delete timelineData;
			}
			animData2->clear();
			delete animData2;
		}
		animDataMap.clear();
		firstFrameOfBoneMap.clear();
	}

public:
	std::string name;										// 骨架名称
	std::vector<Dota_Bone_Data *> boneDataList;				// 骨头数据列表
	std::vector<Dota_Anim_Data *> animDataList;				// 动画数据列表
	std::map<std::string, Dota_Anim_Data2 *> animDataMap;	// 动画数据键值对<动画名称，动画数据2>
	Dota_First_Frame_Data firstFrameOfBoneMap;				// 存取每个骨头的第一帧数据<骨头名，骨头的第一帧>
};