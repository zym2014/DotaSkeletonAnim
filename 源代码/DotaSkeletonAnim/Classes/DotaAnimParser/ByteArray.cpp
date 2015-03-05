#include "ByteArray.h"
#include "Utils.h"

ByteArray::ByteArray(void)
{
	_data = nullptr;
	_size = 0;
	_position = 0;
}

ByteArray::~ByteArray(void)
{
	close();
}

bool ByteArray::loadFromFile(const char* fileName)
{
	cocos2d::Data data = cocos2d::FileUtils::getInstance()->getDataFromFile(fileName);
	_data = data.getBytes();
	_size = data.getSize();
	_position = 0;
	data.fastSet(nullptr, 0);
	return (_data != nullptr && _size > 0);
}

bool ByteArray::loadFromData(unsigned char* data, ssize_t size)
{
	_data = data;
	_size = size;
	_position = 0;
	return (_data != nullptr && _size > 0);
}

bool ByteArray::loadFromZipFile(const char* zipFile, const char* fileName)
{
	if (nullptr == zipFile || nullptr == fileName)
		return false;

	ssize_t size = 0;
	unsigned char* data = getFileDataFromZip(zipFile, fileName, &size);
	if (nullptr == data || size <= 0)
	{
		free(data);
		return false;
	}

	_data = data;
	_size = size;
	_position = 0;
	return (_data != nullptr && _size > 0);
}

void ByteArray::close()
{
	free(_data);
	_data = nullptr;
	_size = 0;
	_position = 0;
}

ssize_t ByteArray::getSize()
{
	return _size;
}

ssize_t ByteArray::getPosition()
{
	return _position;
}

char ByteArray::readByte()
{
	if (nullptr == _data || _size <= 0 || _position+sizeof(char) > _size)
		return 0;

	char data = *(char*)(_data+_position);
	_position += sizeof(char);
	return data;
}

bool ByteArray::readBool()
{
	if (nullptr == _data || _size <= 0 || _position+sizeof(bool) > _size)
		return false;

	bool data = *(bool*)(_data+_position);
	_position += sizeof(bool);
	return data;
}

short ByteArray::readShort()
{
	if (nullptr == _data || _size <= 0 || _position+sizeof(short) > _size)
		return 0;

	short data = *(short*)(_data+_position);
	_position += sizeof(short);
	return data;
}

int ByteArray::readInt()
{
	if (nullptr == _data || _size <= 0 || _position+sizeof(int) > _size)
		return 0;

	int data = *(int*)(_data+_position);
	_position += sizeof(int);
	return data;
}

float ByteArray::readFloat()
{
	if (nullptr == _data || _size <= 0 || _position+sizeof(float) > _size)
		return 0;

	float data = *(float*)(_data+_position);
	_position += sizeof(float);
	return data;
}

double ByteArray::readDouble()
{
	if (nullptr == _data || _size <= 0 || _position+sizeof(double) > _size)
		return 0;

	double data = *(double*)(_data+_position);
	_position += sizeof(double);
	return data;
}

std::string ByteArray::readString(ssize_t size)
{
	if (size <= 0 || nullptr == _data || _size <= 0 || _position+size > _size)
		return "";

	unsigned char* buf = (unsigned char*)malloc(sizeof(unsigned char) * (size + 1));
	if (nullptr == buf)
		return "";
	memcpy(buf, _data+_position, size);
	buf[size] = '\0';
	std::string data = (char*)buf;
	free(buf);
	_position += size;
	return data;
}

unsigned char * ByteArray::readBytes(unsigned char* buf, ssize_t size)
{
	if (nullptr == buf || size <= 0 
		|| nullptr == _data || _size <= 0 
		|| _position+size > _size)
		return nullptr;

	memcpy(buf, _data+_position, size);
	_position += size;
	return buf;
}