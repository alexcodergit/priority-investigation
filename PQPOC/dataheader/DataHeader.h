#pragma once
#include<vector>

namespace pqpoc
{
	using byte = uint8_t;

	struct DataHeader
	{
		uint32_t prio;
		uint64_t sentTime;
		uint64_t receiveTime;
		uint64_t allocated;
		byte* data;

		DataHeader() : prio(1), sentTime(0), receiveTime(0), allocated(0), data(nullptr) {}

		uint32_t GetSize() const
		{
			return sizeof(prio) +
				sizeof(sentTime) +
				sizeof(receiveTime) +
				sizeof(allocated) +
				sizeof(data);
		}

		uint64_t allocate(uint64_t bytes)
		{
			if (bytes < 1)
			{
				return 0;
			}

			if (data != nullptr)
			{
				delete[] data;
				allocated = 0;
			}
			data = new byte[bytes];
			if (data)
			{
				allocated = bytes;
				return allocated;
			}
			return 0;
		}

		uint64_t deallocate()
		{
			if (data)
			{
				uint64_t ret = allocated;
				delete[] data;
				allocated = 0;
				return ret;
			}
			return 0;
		}

		byte getValue(uint32_t idx) const
		{
			return data[idx];
		}

		void setValue(uint32_t idx, byte val)
		{
			data[idx] = val;
		}

		void setAll(byte val)
		{
			for (uint32_t i = 0; i < allocated; i++)
			{
				data[i] = val;
			}
		}
	};
}