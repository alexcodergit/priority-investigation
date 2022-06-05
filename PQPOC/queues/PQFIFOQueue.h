#pragma once
#include<vector>
#include<thread>
#include<atomic>
#include "QUEUE.h"

namespace pqpoc
{
	class PQFIFOQueue : public QUEUE
	{
		std::vector<uint64_t> writtenBytes;
		std::vector<uint64_t> readBytes;

		std::vector<uint32_t> bufferSizes;
		uint32_t blockSize;
		uint32_t blockIdx;
		std::vector<uint32_t> capacities;
		std::vector<std::vector<uint32_t>> processingPattern;
		std::vector<std::atomic<uint32_t>> blocks;
		std::vector<std::vector<byte>> buffers;
		std::vector<uint32_t> rp;
		std::vector<uint32_t> wp;
		std::vector<uint32_t> maxBlocks;

	public:
		PQFIFOQueue(uint32_t unitSize, uint32_t units) : blockSize(unitSize), blockIdx(0)
		{
			if (units < 50 || units > 1000)
			{
				throw "Not supported units size";
			}
			writtenBytes = std::vector<uint64_t>(3, 0);
			readBytes = std::vector<uint64_t>(3, 0);

			bufferSizes = std::vector<uint32_t>(3);
			for (int i = 0; i < bufferSizes.size(); i++)
			{
				bufferSizes[i] = units * blockSize;
			}
			processingPattern = std::vector<std::vector<uint32_t>>(3);
			processingPattern = { {8,8}, {3,3}, {2,2} };

			blocks = std::vector<std::atomic<uint32_t>>(3);
			for (auto& b : blocks)
			{
				b.store(0, std::memory_order_release);
			}
			buffers = std::vector<std::vector<byte>>(3);
			for (int i = 0; i < buffers.size(); i++)
			{
				buffers[i] = std::vector<byte>((uint64_t)blockSize * units);
			}
			capacities = std::vector<uint32_t>(3, units);
			rp = std::vector<uint32_t>(3, 0);
			wp = std::vector<uint32_t>(3, 0);
			maxBlocks = std::vector<uint32_t>(3, 0);
		}

		void Read(DataHeader& packet) override
		{
			bool read = false;
			uint32_t pidx;
			while (!read)
			{
				for (; blockIdx < blocks.size(); )
				{
					if (processingPattern[blockIdx][0] > 0)
					{
						uint32_t stored = blocks[blockIdx].load();
						if (stored > 0)
						{
							processingPattern[blockIdx][0]--;
							pidx = blockIdx;
							read = true;
							break;
						}
						else
						{
							blockIdx++;
							if (blockIdx >= blocks.size())
							{
								blockIdx = 0;
							}
						}
					}
					else
					{
						processingPattern[blockIdx][0] = processingPattern[blockIdx][1];
						blockIdx++;
						if (blockIdx >= blocks.size())
						{
							blockIdx = 0;
						}
					}
				}
			}

			uint32_t rpnext = rp[pidx] + blockSize;
			if (rpnext >= bufferSizes[pidx])
			{
				rpnext = 0;
			}
			std::copy((byte*)buffers[pidx].data() + rp[pidx], (byte*)buffers[pidx].data() +
				rp[pidx] + sizeof(packet.prio), reinterpret_cast<byte*>(std::addressof(packet.prio)));

			rp[pidx] += sizeof(packet.prio);

			std::copy((byte*)buffers[pidx].data() + rp[pidx],
				(byte*)buffers[pidx].data() + rp[pidx] + sizeof(packet.sentTime),
				reinterpret_cast<byte*>(std::addressof(packet.sentTime)));

			rp[pidx] += sizeof(packet.sentTime);

			std::copy((byte*)buffers[pidx].data() + rp[pidx],
				(byte*)buffers[pidx].data() + rp[pidx] + sizeof(packet.receiveTime),
				reinterpret_cast<byte*>(std::addressof(packet.receiveTime)));

			rp[pidx] += sizeof(packet.sentTime);

			std::copy((byte*)buffers[pidx].data() + rp[pidx],
				(byte*)buffers[pidx].data() + rp[pidx] + sizeof(packet.allocated),
				reinterpret_cast<byte*>(std::addressof(packet.allocated)));

			rp[pidx] += sizeof(packet.allocated);

			std::copy((byte*)buffers[pidx].data() + rp[pidx],
				(byte*)buffers[pidx].data() + rp[pidx] + sizeof(packet.data),
				reinterpret_cast<byte*>(std::addressof(packet.data)));

			readBytes[pidx] += packet.GetSize();

			rp[pidx] = rpnext;
			blocks[pidx]--;
		}

		void Write(DataHeader& packet) override
		{
			bool written = false;
			const uint32_t pidx = packet.prio - 1;
			while (!written)
			{
				uint32_t stored = blocks[pidx].load();
				if (stored < capacities[pidx])
				{
					uint32_t wpnext = wp[pidx] + blockSize;
					if (wpnext >= bufferSizes[pidx])
					{
						wpnext = 0;
					}

					std::copy(reinterpret_cast<const byte*>(std::addressof(packet.prio)),
						reinterpret_cast<const byte*>(std::addressof(packet.prio)) +
						sizeof(packet.prio), buffers[pidx].data() + wp[pidx]);

					wp[pidx] += sizeof(packet.prio);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packet.sentTime)),
						reinterpret_cast<const byte*>(std::addressof(packet.sentTime)) +
						sizeof(packet.sentTime), buffers[pidx].data() + wp[pidx]);

					wp[pidx] += sizeof(packet.sentTime);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packet.receiveTime)),
						reinterpret_cast<const byte*>(std::addressof(packet.receiveTime)) +
						sizeof(packet.receiveTime), buffers[pidx].data() + wp[pidx]);

					wp[pidx] += sizeof(packet.receiveTime);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packet.allocated)),
						reinterpret_cast<const byte*>(std::addressof(packet.allocated)) +
						sizeof(packet.allocated), buffers[pidx].data() + wp[pidx]);

					wp[pidx] += sizeof(packet.allocated);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packet.data)),
						reinterpret_cast<const byte*>(std::addressof(packet.data)) +
						sizeof(packet.data), buffers[pidx].data() + wp[pidx]);

					writtenBytes[pidx] += packet.GetSize();

					wp[pidx] = wpnext;

					blocks[pidx]++;
					written = true;
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
			for (auto& wb : writtenBytes)
			{
				wb = 0;
			}
			for (auto& rb : readBytes)
			{
				rb = 0;
			}
		}
	};
}