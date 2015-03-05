DotaSkeletonAnim
================
    首先声明，我不是第一个做出类似查看器的首作者，我所知道的首作者是johance，以下是它在cocoschina发布的贴子。<br>

    分享 刀塔查看器Flash導出版本(帶詳細解析說明)和Flash導入導出批處理文件<br>
    http://www.cocoachina.com/bbs/read.php?tid-219173.html<br>

    贴子里面描述了《刀塔传奇》骨骼动画的文件格式，我是参考这个文件格式写出来的。我的实现思路是将*.fca文件转换成DragonBonesCPP所需要的格式，然后就可以通过DragonBonesCPP播放出来了。注意这里没有转换成DragonBonesCPP的skeleton.xml，而是直接转换成了DragonBonesCPP解析skeleton.xml后的数据结构，免去了中间环节。当然你也可以通过我实现的代码接口导出DragonBonesCPP的三个文件sheet.pvr、texture.xml、skeleton.xml，然后像平时使用DragonBonesCPP的那样来加载播放，这样可以避免加载时转换，效率更高。<br>

    *.fca文件是一个zip压缩包，将其用WinRAR解压缩出来后，可以得到cha、plist、sheet.pvr三个文件，其中sheet.pvr是纹量集图片，包含了该角色使用的所有骨骼碎图。plist是纹量集的骨骼碎图描述文件，描述了每个骨骼碎图在sheet.pvr大图中的坐标、大小等区域信息。cha是骨骼动画数据文件，其格式如下所述。<br>

cha格式：<br>

声明：<br>
以下格式来自于johance公布的贴子，只做了少量修饰和补充。<br>

{}表示一个数据结构，带...表示有多个数据结构。<br>

角色名称长度(4B)<br>
角色名称 不包含"\0"<br>
骨骼数量(4B)<br>
｛<br>
	骨骼名称长度(4B)<br>
	骨骼名称<br>
	骨骼图片文件名长度(4B)<br>
	骨骼图片文件名<br>
	骨骼索引(4B)
｝...<br>
动画个数(4B)<br>
｛<br>
	动画名称长度(4B)<br>
	动画名称<br>
	未知数据(4B) 固定为0x41c00000<br>
	动画帧数(4B)<br>
	｛<br>
		帧类型(4B)<br>
		｛<br>
			// 帧类型为1时才需要解析这个结构，否则跳过这个结构<br>
			未知数据1(4B)<br>
			声音文件名称长度(4B)<br>
			声音文件名称<br>
			未知声音数据(32B)<br>
			未知数据2(4B)<br>
		 ｝<br>
		当前帧使用的骨骼数量(4B)<br>
		｛<br>
			骨骼索引(2B)<br>
			透明度(1B)<br>
			a、b、c、d、tx、ty(类似Flash中的Matrix的6个float)<br>
		 ｝...<br>
	}...<br>
｝...<br>

类似Flash中的Matrix的6个float的详细含义请参考以下文章和AS代码。<br>

as3中Matrix详解<br>
http://wgcode.iteye.com/blog/838550<br>

MatrixTransformer.as：<br>

public static function getScaleX(m:Matrix):Number<br>
{<br>
    return Math.sqrt(m.a*m.a + m.b*m.b);<br>
}<br>

public static function getScaleY(m:Matrix):Number<br>
{<br>
    return Math.sqrt(m.c*m.c + m.d*m.d);<br>
}<br>

public static function getSkewX(m:Matrix):Number<br>
{<br>
    return Math.atan2(-m.c, m.d) * (180/Math.PI);<br>
}<br>

public static function getSkewY(m:Matrix):Number<br>
{<br>
    return Math.atan2(m.b, m.a) * (180/Math.PI);<br>
}<br>


界面截图：<br>
![image](https://github.com/zym2014/DotaSkeletonAnim/blob/master/%E7%95%8C%E9%9D%A2%E6%88%AA%E5%9B%BE/%E4%B8%BB%E7%95%8C%E9%9D%A2.png)<br>
![image](https://github.com/zym2014/DotaSkeletonAnim/blob/master/%E7%95%8C%E9%9D%A2%E6%88%AA%E5%9B%BE/DotaDemo_%E4%B8%BB%E7%95%8C%E9%9D%A2.png)<br>
![image](https://github.com/zym2014/DotaSkeletonAnim/blob/master/%E7%95%8C%E9%9D%A2%E6%88%AA%E5%9B%BE/DotaDemo_%E6%88%98%E6%96%97%E7%95%8C%E9%9D%A2.png)<br>

开发环境：<br>
Cocos2d-x-3.3 Final<br>
DragonBonesCPP<br>

Windows安装包下载地址：<br>
http://pan.baidu.com/s/1dDyBu85<br>

Android安装包下载地址：<br>
http://pan.baidu.com/s/1rVOCI<br>

资源下载地址：<br>
http://pan.baidu.com/s/14NgoI<br>

项目地址：<br>
[https://github.com/zym2014/DotaSkeletonAnim](https://github.com/zym2014/DotaSkeletonAnim)<br>

作者Blog：<br>
[http://zym.cnblogs.com](http://zym.cnblogs.com)<br>
[http://blog.csdn.net/zym_123456](http://blog.csdn.net/zym_123456)<br>
