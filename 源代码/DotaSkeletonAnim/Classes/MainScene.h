#ifndef __MAINSCENE_SCENE_H__
#define __MAINSCENE_SCENE_H__

#include "cocos2d.h"
#include "extensions/cocos-ext.h"
#include "DBCCRenderHeaders.h"

USING_NS_DB;

class CustomTableViewCell : public cocos2d::extension::TableViewCell
{
public:
	virtual void draw(cocos2d::Renderer *renderer, const cocos2d::Mat4 &transform, uint32_t flags) override
	{
		TableViewCell::draw(renderer, transform, flags);
		// draw bounding box
		// 	auto pos = getPosition();
		// 	auto size = Size(178, 200);
		// 	Vec2 vertices[4]={
		// 		Vec2(pos.x+1, pos.y+1),
		// 		Vec2(pos.x+size.width-1, pos.y+1),
		// 		Vec2(pos.x+size.width-1, pos.y+size.height-1),
		// 		Vec2(pos.x+1, pos.y+size.height-1),
		// 	};
		// 	DrawPrimitives::drawColor4B(0, 0, 255, 255);
		// 	DrawPrimitives::drawPoly(vertices, 4, true);
	}
};

class MainScene : public cocos2d::Layer, 
	public cocos2d::extension::TableViewDataSource, 
	public cocos2d::extension::TableViewDelegate
{
public:
	MainScene();
	~MainScene();

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(MainScene);

private:
	void addEventListener();	// 添加事件监听器
	bool initData(const char * fileName, std::vector<std::string>& nameList);
	void initUI();


#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
	cocos2d::EventListenerKeyboard *_keyboardListener;
	virtual void keyPressHandler(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event *event);
	virtual void keyReleaseHandler(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event *event);
#endif

	void eventHandler(cocos2d::EventCustom *event);
	void delArmatureNode();

	virtual void scrollViewDidScroll(cocos2d::extension::ScrollView* view) {};
	virtual void scrollViewDidZoom(cocos2d::extension::ScrollView* view) {}
	virtual void tableCellTouched(cocos2d::extension::TableView* table, cocos2d::extension::TableViewCell* cell);
	virtual cocos2d::Size tableCellSizeForIndex(cocos2d::extension::TableView *table, ssize_t idx);
	virtual cocos2d::extension::TableViewCell* tableCellAtIndex(cocos2d::extension::TableView *table, ssize_t idx);
	virtual ssize_t numberOfCellsInTableView(cocos2d::extension::TableView *table);

	cocos2d::extension::TableViewCell* newCell(cocos2d::extension::TableView* table, int nIndex);
	void setCell(cocos2d::extension::TableView* table, int nIndex, cocos2d::extension::TableViewCell* cell);

	bool onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event);
	void onTouchMoved(cocos2d::Touch* touch, cocos2d::Event* event);
	void onTouchEnded(cocos2d::Touch* touch, cocos2d::Event* event);
	void onTouchCancelled(cocos2d::Touch* touch, cocos2d::Event* event);

	void showArmatureNode(const std::string& fcaFile, const std::string &armatureName);
	void showHero(int nIndex);		// 显示指定索引的英雄
	void showEffect(int nIndex);	// 显示指定索引的物效
	void showPrev();				// 显示上一个英雄或特效
	void showNext();				// 显示下一个英雄或特效
	void playNextAnim();			// 播放下一个动画

	void exportDBConfigFile();		// 导出DragonBones配置文件演示

private:
	DBCCArmatureNode *_armatureNode;

	cocos2d::extension::TableView* _tableView1, * _tableView2;

	bool _isHero;
	int _curHero, _curEffect;
	std::vector<std::string> _heroList, _effectList;

	int _curAnimIndex;
	std::vector<std::string> _animationList;
};

#endif // __MAINSCENE_SCENE_H__
