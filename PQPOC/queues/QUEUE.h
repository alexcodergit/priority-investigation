#pragma once
#include "../dataheader/DataHeader.h"

namespace pqpoc
{
	class QUEUE
	{
	public:
		virtual void Write(DataHeader& packet) = 0;
		virtual void Read(DataHeader& packet) = 0;
	};
}