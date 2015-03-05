#include "MainScene.h"
#include "Utils.h"
#include "SimpleAudioEngine.h"
#include "external/tinyxml2/tinyxml2.h"
#include "DotaAnimParser/DBCCFactoryEx.h"

USING_NS_CC;
USING_NS_CC_EXT;
using namespace CocosDenshion;

MainScene::MainScene()
	: _armatureNode(nullptr)
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
	, _keyboardListener(nullptr)
#endif
	, _tableView1(nullptr)
	, _tableView2(nullptr)
	, _isHero(true)
	, _curHero(0)
	, _curEffect(0)
	, _curAnimIndex(0)
{
}

MainScene::~MainScene()
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
	this->getEventDispatcher()->removeEventListener(_keyboardListener);
	_keyboardListener = nullptr;
#endif
	_armatureNode = nullptr;
}

Scene* MainScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = MainScene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool MainScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }

	addEventListener();

	initData("Hero.xml", _heroList);
	initData("Effect.xml", _effectList);

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(MainScene::menuCloseCallback, this));
    
	closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
    
	initUI();
 	
 	DBCCEventDispatcher * pEventDispatcher = new DBCCEventDispatcher();
 	pEventDispatcher->eventDispatcher = new cocos2d::EventDispatcher();
 	pEventDispatcher->eventDispatcher->setEnabled(true);
 	pEventDispatcher->eventDispatcher->addCustomEventListener(
 		EventData::SOUND, CC_CALLBACK_1(MainScene::eventHandler, this));
 	Armature::soundEventDispatcher = pEventDispatcher;	

	showHero(13);

	//exportDBConfigFile();	// 导出DragonBones配置文件演示

    return true;
}

void MainScene::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

// 添加事件监听器
void MainScene::addEventListener()
{
	// 添加触摸事件监听器
	EventListenerTouchOneByOne* touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(MainScene::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(MainScene::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(MainScene::onTouchEnded, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(MainScene::onTouchCancelled, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(touchListener, this);

	// 添加按键事件监听器
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
	_keyboardListener = cocos2d::EventListenerKeyboard::create();
	_keyboardListener->onKeyPressed = CC_CALLBACK_2(MainScene::keyPressHandler, this);
	_keyboardListener->onKeyReleased = CC_CALLBACK_2(MainScene::keyReleaseHandler, this);
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(_keyboardListener, this);
#endif
}

bool MainScene::initData(const char * fileName, std::vector<std::string>& nameList)
{
	bool ret = false;
	Data data = cocos2d::FileUtils::getInstance()->getDataFromFile(fileName);
	if (data.isNull())
	{
		return false;
	}

	tinyxml2::XMLDocument xmlDoc;
	xmlDoc.Parse((const char*)data.getBytes(), data.getSize());

	tinyxml2::XMLElement *rootNode = xmlDoc.RootElement();

	tinyxml2::XMLElement *node = rootNode->FirstChildElement("Item");
	while (node)
	{
		std::string name = node->Attribute("name");
		nameList.push_back(name);
		node = node->NextSiblingElement("Item");
		CCLOG("%s", name.c_str());
	}	

	return true;
}

void MainScene::initUI()
{
	const Size& szWin = Director::getInstance()->getWinSize();

	Configuration *cfg = Configuration::getInstance();
	cfg->loadConfigFile("strings.plist");

	// 背景图片
	Sprite* spBg = Sprite::create("bbg_blood_elf_door.jpg");
	spBg->setPosition(szWin.width/2, szWin.height/2);
	const Size& szBg = spBg->getTextureRect().size;
	spBg->setScaleX(szWin.width/szBg.width);
	spBg->setScaleY(szWin.height/szBg.height);
	this->addChild(spBg, 0);

	_tableView1 = TableView::create(this, Size(szWin.width, 104));
	_tableView1->setDirection(ScrollView::Direction::HORIZONTAL);
	_tableView1->setPosition(0, szWin.height-104);
	_tableView1->setDelegate(this);
	this->addChild(_tableView1);
	_tableView1->reloadData();

	_tableView2 = TableView::create(this, Size(szWin.width, 104));
	_tableView2->setDirection(ScrollView::Direction::HORIZONTAL);
	_tableView2->setPosition(0, 0);
	_tableView2->setDelegate(this);
	this->addChild(_tableView2);
	_tableView2->reloadData();

	Value defFontName("Arial"), fontName;
	fontName = cfg->getValue("fontname", defFontName);

	Value defValue("unknown"), value;
	value = cfg->getValue("tips", defValue);

	auto label = Label::createWithSystemFont(value.asString(), fontName.asString(), 20.0);
	label->setPosition(Vec2(40, szWin.height-150));
	label->setAnchorPoint(Vec2::ZERO);
	label->setTextColor(Color4B(255,0,0,255));
	this->addChild(label, 1);

	value = cfg->getValue("cur_effect", defValue);
	label = Label::createWithSystemFont(value.asString(), fontName.asString(), 20.0);
	label->setPosition(Vec2(40, szWin.height-180));
	label->setAnchorPoint(Vec2::ZERO);
	label->setTextColor(Color4B(255,0,0,255));
	this->addChild(label, 1);

	value = cfg->getValue("cur_anim", defValue);
	label = Label::createWithSystemFont(value.asString(), fontName.asString(), 20.0);
	label->setPosition(Vec2(40, szWin.height-210));
	label->setAnchorPoint(Vec2::ZERO);
	label->setTextColor(Color4B(255,0,0,255));
	this->addChild(label, 1);

	label = Label::createWithSystemFont("CW", fontName.asString(), 20.0);
	label->setPosition(Vec2(140, szWin.height-180));
	label->setAnchorPoint(Vec2::ZERO);
	label->setTextColor(Color4B(255,0,0,255));
	label->setTag(1001);
	this->addChild(label, 1);

	label = Label::createWithSystemFont("Idle", fontName.asString(), 20.0);
	label->setPosition(Vec2(140, szWin.height-210));
	label->setAnchorPoint(Vec2::ZERO);
	label->setTextColor(Color4B(255,0,0,255));
	label->setTag(1002);
	this->addChild(label, 1);

	value = cfg->getValue("author", defValue);
	label = Label::createWithSystemFont(value.asString(), fontName.asString(), 20.0);
	label->setPosition(Vec2(40, 260));
	label->setAnchorPoint(Vec2::ZERO);
	label->setTextColor(Color4B(255,0,0,255));
	this->addChild(label, 1);

	value = cfg->getValue("email", defValue);
	label = Label::createWithSystemFont(value.asString(), fontName.asString(), 20.0);
	label->setPosition(Vec2(40, 230));
	label->setAnchorPoint(Vec2::ZERO);
	label->setTextColor(Color4B(255,0,0,255));
	this->addChild(label, 1);

	value = cfg->getValue("blog", defValue);
	label = Label::createWithSystemFont(value.asString(), fontName.asString(), 20.0);
	label->setPosition(Vec2(40, 200));
	label->setAnchorPoint(Vec2::ZERO);
	label->setTextColor(Color4B(255,0,0,255));
	this->addChild(label, 1);
}



#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
void MainScene::keyPressHandler(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event *event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_W:
	case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
		break;

	case cocos2d::EventKeyboard::KeyCode::KEY_S:
	case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
		break;

	case cocos2d::EventKeyboard::KeyCode::KEY_A:
	case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
		showPrev();
		break;

	case cocos2d::EventKeyboard::KeyCode::KEY_D:
	case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
		showNext();
		break;

	case cocos2d::EventKeyboard::KeyCode::KEY_SPACE:
		playNextAnim();
		break;
	}
}

void MainScene::keyReleaseHandler(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event *event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_W:
	case cocos2d::EventKeyboard::KeyCode::KEY_UP_ARROW:
		break;

	case cocos2d::EventKeyboard::KeyCode::KEY_DOWN_ARROW:
	case cocos2d::EventKeyboard::KeyCode::KEY_S:
		break;

	case cocos2d::EventKeyboard::KeyCode::KEY_A:
	case cocos2d::EventKeyboard::KeyCode::KEY_LEFT_ARROW:
		break;

	case cocos2d::EventKeyboard::KeyCode::KEY_D:
	case cocos2d::EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
		break;
	}
}
#endif

void MainScene::eventHandler(cocos2d::EventCustom *event)
{
	EventData *eventData = static_cast<EventData*>(event->getUserData());
	if (eventData && eventData->animationState)
	{
		CCLOG("Animation name: %s, Event type: %s",
			eventData->animationState->name.c_str(),
			eventData->getStringType().c_str()
			);

		if (eventData->getType() == EventData::EventType::SOUND)
		{
			std::string path = "sound/" + eventData->sound;
			SimpleAudioEngine::getInstance()->playEffect(path.c_str());
		}
	}
}

void MainScene::delArmatureNode()
{
	if (nullptr == _armatureNode)
		return;

	_armatureNode->removeFromParent();
	_armatureNode = nullptr;

	if (_isHero)
	{
		DBCCFactoryEx::getInstanceEx()->removeTextureAtlas(_heroList[_curHero]);
		DBCCFactoryEx::getInstanceEx()->removeDragonBonesData(_heroList[_curHero]);
	}
	else
	{
		DBCCFactoryEx::getInstanceEx()->removeTextureAtlas(_effectList[_curEffect]);
		DBCCFactoryEx::getInstanceEx()->removeDragonBonesData(_effectList[_curEffect]);
	}
	cocos2d::Director::getInstance()->getTextureCache()->removeUnusedTextures();
}

void MainScene::tableCellTouched(TableView* table, TableViewCell* cell)
{
	CCLOG("cell touched at index: %ld", cell->getIdx());

	if (table == _tableView1)
	{
		showHero(cell->getIdx());
	}
	else
	{
		showEffect(cell->getIdx());
	}
}

Size MainScene::tableCellSizeForIndex(TableView *table, ssize_t idx)
{
	return Size(104, 104);
}

TableViewCell* MainScene::tableCellAtIndex(TableView *table, ssize_t idx)
{
	TableViewCell *cell = table->dequeueCell();
	if (!cell) 
	{
		cell = newCell(table, idx);
	}
	else
	{
		setCell(table, idx, cell);
	}

	return cell;
}

ssize_t MainScene::numberOfCellsInTableView(TableView *table)
{
	if (table == _tableView1)
		return _heroList.size();
	else
		return _effectList.size();
}

TableViewCell* MainScene::newCell(TableView* table, int nIndex)
{
	TableViewCell* cell = new (std::nothrow) CustomTableViewCell();
	cell->autorelease();

	Sprite* sprite = nullptr;
	if (table == _tableView1)
	{
		std::string jpgFile = "UI/DOTA-HERO/" + _heroList[nIndex] + ".jpg";
		sprite = createDotaSprite(jpgFile.c_str());
		if (nullptr == sprite)
		{
			jpgFile = "UI/DOTA-HERO/wca_3.jpg";
			sprite = createDotaSprite(jpgFile.c_str());
		}
	}
	else
	{
		std::string jpgFile = "UI/DOTA-HERO/wca_3.jpg";
		sprite = createDotaSprite(jpgFile.c_str());
	}

	if (sprite != nullptr)
	{
		sprite->setAnchorPoint(Vec2::ZERO);
		sprite->setPosition(0, 0);
		sprite->setTag(456);
		cell->addChild(sprite);
	}

	String* string = String::createWithFormat("%ld", nIndex);
	Label* label = Label::createWithSystemFont(string->getCString(), "Helvetica", 20.0);
	label->setPosition(Vec2::ZERO);
	label->setAnchorPoint(Vec2::ZERO);
	label->setTag(123);
	cell->addChild(label);

	return cell;
}

void MainScene::setCell(TableView* table, int nIndex, TableViewCell* cell)
{
	if (table == _tableView1)
	{
		Sprite* sprite = (Sprite*)cell->getChildByTag(456);
		if (sprite != nullptr)
		{
			std::string jpgFile = "UI/DOTA-HERO/" + _heroList[nIndex] + ".jpg";
			Texture2D * texture = openDotaImage(jpgFile.c_str());
			if (nullptr == texture)
			{
				jpgFile = "UI/DOTA-HERO/wca_3.jpg";
				texture = openDotaImage(jpgFile.c_str());
			}
			sprite->setTexture(texture);
		}
	}

	Label* label = (Label*)cell->getChildByTag(123);
	if (label != nullptr)
	{
		String* string = String::createWithFormat("%ld", nIndex);
		label->setString(string->getCString());
	}
}

bool MainScene::onTouchBegan(Touch *touch, Event *event)
{
	if (_tableView1 && _tableView2 && _tableView1->getParent())
	{
		Vec2 point = _tableView1->getParent()->convertTouchToNodeSpace(touch);

		Rect rect1 = _tableView1->getBoundingBox();
		Rect rect2 = _tableView2->getBoundingBox();
		if (!rect1.containsPoint(point) && !rect2.containsPoint(point))
			playNextAnim();
	}
	return true;
}

void MainScene::onTouchMoved(Touch* touch, Event* event)
{
}

void MainScene::onTouchEnded(Touch* touch, Event* event)
{
}

void MainScene::onTouchCancelled(Touch* touch, Event* event)
{
}

void MainScene::showArmatureNode(const std::string& fcaFile, const std::string &armatureName)
{
	const Size& szWin = Director::getInstance()->getWinSize();

	delArmatureNode();

	DBCCFactoryEx::getInstanceEx()->loadDotaFcaFile(fcaFile);
	_armatureNode = DBCCFactoryEx::getInstanceEx()->buildArmatureNode(armatureName);
	_armatureNode->setPosition(szWin.width/2, szWin.height/2);
	addChild(_armatureNode);

	_animationList.clear();
	_animationList = _armatureNode->getAnimation()->getAnimationList();
	_curAnimIndex = 0;
	_armatureNode->getAnimation()->gotoAndPlay(_animationList[_curAnimIndex]);

	Label* label = (Label*)getChildByTag(1001);
	if (label)
		label->setString(armatureName);

	label = (Label*)getChildByTag(1002);
	if (label)
		label->setString(_animationList[_curAnimIndex]);
}

void MainScene::showHero(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)_heroList.size())
		return;

	_isHero = true;
	_curHero = nIndex;
	std::string name = _heroList[_curHero];
	std::string fcaFile = "pfca/" + name + ".fca";
	showArmatureNode(fcaFile, name);
}

void MainScene::showEffect(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)_effectList.size())
		return;

	_isHero = false;
	_curEffect = nIndex;
	std::string name = _effectList[_curEffect];
	std::string fcaFile = "pfca/" + name + ".fca";
	showArmatureNode(fcaFile, name);
}

// 显示上一个英雄或特效
void MainScene::showPrev()
{
	if (_isHero)
	{
		_curHero--;
		if (_curHero < 0)
		{
			_curHero = _heroList.size()-1;
		}
		showHero(_curHero);
	}
	else
	{
		_curEffect--;
		if (_curEffect < 0)
		{
			_curEffect = _effectList.size()-1;
		}
		showEffect(_curEffect);
	}	
}

// 显示下一个英雄或特效
void MainScene::showNext()
{
	if (_isHero)
	{
		_curHero++;
		if (_curHero >= (int)(_heroList.size()))
		{
			_curHero = 0;
		}
		showHero(_curHero);
	}
	else
	{
		_curEffect++;
		if (_curEffect >= (int)(_effectList.size()))
		{
			_curEffect = 0;
		}
		showEffect(_curEffect);
	}
}

// 播放下一个动画
void MainScene::playNextAnim()
{
	_curAnimIndex++;
	if (_curAnimIndex >= (int)(_animationList.size()))
	{
		_curAnimIndex = 0;
	}
	_armatureNode->getAnimation()->gotoAndPlay(_animationList[_curAnimIndex]);

	auto label = (Label *)getChildByTag(1002);
	if (label)
		label->setString(_animationList[_curAnimIndex]);
}

// 导出DragonBones配置文件演示
void MainScene::exportDBConfigFile()
{
	const Size& szWin = Director::getInstance()->getWinSize();

	std::string savePath = "D:/";
	DBCCFactoryEx::getInstanceEx()->exportDBConfigFile("pfca/CW.fca", savePath);

	DBCCFactory::getInstance()->loadTextureAtlas(savePath+"texture.xml");
	DBCCFactory::getInstance()->loadDragonBonesData(savePath+"skeleton.xml");
	DBCCArmatureNode* armatureNode = DBCCFactory::getInstance()->buildArmatureNode("CW");
	armatureNode->setPosition(szWin.width/2, szWin.height/2);
	addChild(armatureNode);

	armatureNode->getAnimation()->gotoAndPlay("Idle");
}