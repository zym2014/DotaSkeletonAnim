#pragma once

#include "cocos2d.h"

class ByteArray
{
public:
	ByteArray(void);
	~ByteArray(void);

public:
	bool loadFromFile(const char* fileName);
	bool loadFromData(unsigned char* data, ssize_t size);
	bool loadFromZipFile(const char* zipFile, const char* fileName);
	void close();

	ssize_t getSize();
	ssize_t getPosition();

	char readByte();
	bool readBool();
	short readShort();
	int readInt();
	float readFloat();
	double readDouble();
	std::string readString(ssize_t size);
	unsigned char* readBytes(unsigned char* buf, ssize_t size);

private:
	unsigned char* _data;
	ssize_t _size;
	ssize_t _position;
};

