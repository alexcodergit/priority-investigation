#pragma once
#include<iostream>
#include "DataHeader.h"

int SeralizeIntoSegment(byte* segmentBeg, const DataHeader& dh)
{
	int pos = 0;
	std::copy(reinterpret_cast<const byte*>(std::addressof(dh.prio)),
		reinterpret_cast<const byte*>(std::addressof(dh.prio)) + sizeof(dh.prio),
		segmentBeg);

	pos += sizeof(dh.prio);

	std::copy(reinterpret_cast<const byte*>(std::addressof(dh.sentTime)),
		reinterpret_cast<const byte*>(std::addressof(dh.sentTime)) + sizeof(dh.sentTime),
		segmentBeg + pos);
	pos += sizeof(dh.sentTime);

	std::copy(reinterpret_cast<const byte*>(std::addressof(dh.receiveTime)),
		reinterpret_cast<const byte*>(std::addressof(dh.receiveTime)) + sizeof(dh.receiveTime),
		segmentBeg + pos);
	pos += sizeof(dh.receiveTime);

	std::copy(reinterpret_cast<const byte*>(dh.data),
		reinterpret_cast<const byte*>(dh.data.data + sizeof(byte*)),
		segmentBeg + pos);

	pos += sizeof(int) * dh.data.size();

	return pos;
}

int DeserializeFromSegment(byte* segmentBeg, DataHeader& ds, int size)
{
	int pos = 0;
	std::copy(segmentBeg + pos, segmentBeg + pos + sizeof(ds.prio),
		reinterpret_cast<byte*>(std::addressof(ds.prio)));
	pos += sizeof(ds.prio);

	std::copy(segmentBeg + pos, segmentBeg + pos + sizeof(ds.sentTime),
		reinterpret_cast<byte*>(std::addressof(ds.sentTime)));
	pos += sizeof(ds.sentTime);

	std::copy(segmentBeg + pos, segmentBeg + pos + sizeof(ds.receiveTime),
		reinterpret_cast<byte*>(std::addressof(ds.receiveTime)));
	pos += sizeof(ds.receiveTime);

	int rep = (size - pos) / (int) sizeof(int);
	ds.data = std::vector<int>(rep);

	std::copy(segmentBeg + pos, segmentBeg + size, reinterpret_cast<byte*>(ds.data.data()));

	return size;
}