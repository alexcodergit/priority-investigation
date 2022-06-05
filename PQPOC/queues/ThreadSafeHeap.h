#pragma once
#include<mutex>
#include<vector>
#include<algorithm>
#include "../dataheader/DataHeader.h"
#include "QUEUE.h"

namespace pqpoc
{
	class ThreadSafeHeap : public QUEUE
	{
		mutable std::mutex m;
		uint32_t max_size;
		uint32_t cur_size;
		std::vector<uint64_t> writtenBytes;
		std::vector<uint64_t> readBytes;
		std::vector<DataHeader> heap;
	public:
		ThreadSafeHeap(uint32_t size) : max_size(size), cur_size(0)
		{
			writtenBytes = std::vector<uint64_t>(3, 0);
			readBytes = std::vector<uint64_t>(3, 0);
			heap = std::vector<DataHeader>(max_size);
		}
		ThreadSafeHeap& operator=(const ThreadSafeHeap& tsh) = delete;

		void Write(DataHeader& dh) override
		{
			bool written = false;
			while (!written)
			{
				std::lock_guard<std::mutex> lock(m);
				if (cur_size < max_size)
				{
					heap[cur_size] = dh;
					cur_size++;
					std::push_heap(heap.begin(), heap.begin() + cur_size,
						[](DataHeader& dh1, DataHeader& dh2)
						{ return (dh1.prio > dh2.prio) ||
						((dh1.prio == dh2.prio) && (dh1.sentTime > dh2.sentTime)); });
					writtenBytes[dh.prio - 1] += dh.GetSize();
					written = true;
				}
			}
		}

		void Read(DataHeader& dh) override
		{
			bool read = false;
			while (!read)
			{
				std::lock_guard<std::mutex> lock(m);
				if (cur_size > 0)
				{
					dh = heap.front();
					std::pop_heap(heap.begin(), heap.begin() + cur_size,
						[](DataHeader& dh1, DataHeader& dh2)
						{ return (dh1.prio > dh2.prio) ||
						((dh1.prio == dh2.prio) && (dh1.sentTime > dh2.sentTime)); });
					cur_size--;
					readBytes[dh.prio - 1] += dh.GetSize();
					read = true;
				}
			}
		}

		std::vector<uint64_t> GetWrittenBytes() const
		{
			return writtenBytes;
		}

		std::vector<uint64_t> GetReadBytes() const
		{
			return readBytes;
		}
		void reset()
		{
			for (uint32_t i = 0; i < writtenBytes.size(); i++)
			{
				writtenBytes[i] = 0;
				readBytes[i] = 0;
			}
		}
	};
}