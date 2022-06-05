#pragma once
#include<thread>
#include<chrono>
#include<atomic>
#include "QUEUE.h"

namespace pqpoc
{

	class FIFOQueue : public QUEUE
	{
		uint64_t writtenBytes;
		uint64_t readBytes;

		uint32_t bufferSize;
		uint32_t blockSize;
		uint32_t capacity;
		std::atomic<uint32_t> blocks;
		std::vector<byte> buffer;
		uint32_t rp;
		uint32_t wp;
	public:
		FIFOQueue(uint32_t unitSize, uint32_t units) : writtenBytes(0),
			readBytes(0), blockSize(unitSize), capacity(units), rp(0), wp(0)
		{
			blocks.store(0, std::memory_order_release);
			bufferSize = unitSize * units;
			buffer = std::vector<byte>(bufferSize);
		}

		void Write(DataHeader& packet) override
		{
			bool inserted = false;
			while (!inserted)
			{
				uint32_t stored = blocks.load();
				if (stored < capacity)
				{
					uint32_t wpnext = wp + blockSize;
					if (wpnext >= bufferSize)
					{
						wpnext = 0;
					}

					std::copy(reinterpret_cast<const byte*>(std::addressof(packet.prio)),
						reinterpret_cast<const byte*>(std::addressof(packet.prio)) +
						sizeof(packet.prio), buffer.data() + wp);

					wp += sizeof(packet.prio);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packet.sentTime)),
						reinterpret_cast<const byte*>(std::addressof(packet.sentTime)) +
						sizeof(packet.sentTime), buffer.data() + wp);

					wp += sizeof(packet.sentTime);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packet.receiveTime)),
						reinterpret_cast<const byte*>(std::addressof(packet.receiveTime)) +
						sizeof(packet.receiveTime), buffer.data() + wp);

					wp += sizeof(packet.receiveTime);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packet.allocated)),
						reinterpret_cast<const byte*>(std::addressof(packet.allocated)) +
						sizeof(packet.allocated), buffer.data() + wp);

					wp += sizeof(packet.allocated);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packet.data)),
						reinterpret_cast<const byte*>(std::addressof(packet.data)) +
						sizeof(packet.data), buffer.data() + wp);

					writtenBytes += packet.GetSize();

					wp = wpnext;

					blocks++;
					inserted = true;
				}
			}
		}

		void Write(std::vector<DataHeader>& packets)
		{
			while (!packets.empty())
			{
				uint32_t stored = blocks.load();
				if (stored < capacity)
				{
					if (packets.back().GetSize() > blockSize)
					{
						throw("Packet too big");
					}

					uint32_t wpnext = wp + blockSize;
					if (wpnext >= bufferSize)
					{
						wpnext = 0;
					}

					std::copy(reinterpret_cast<const byte*>(std::addressof(packets.back().prio)),
						reinterpret_cast<const byte*>(std::addressof(packets.back().prio)) +
						sizeof(packets.back().prio),
						buffer.data() + wp);

					wp += sizeof(packets.back().prio);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packets.back().sentTime)),
						reinterpret_cast<const byte*>(std::addressof(packets.back().sentTime)) +
						sizeof(packets.back().sentTime),
						buffer.data() + wp);

					wp += sizeof(packets.back().sentTime);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packets.back().receiveTime)),
						reinterpret_cast<const byte*>(std::addressof(packets.back().receiveTime)) +
						sizeof(packets.back().receiveTime),
						buffer.data() + wp);

					wp += sizeof(packets.back().receiveTime);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packets.back().allocated)),
						reinterpret_cast<const byte*>(std::addressof(packets.back().allocated)) +
						sizeof(packets.back().allocated),
						buffer.data() + wp);

					wp += sizeof(packets.back().allocated);

					std::copy(reinterpret_cast<const byte*>(std::addressof(packets.back().data)),
						reinterpret_cast<const byte*>(std::addressof(packets.back().data)) +
						sizeof(packets.back().data),
						buffer.data() + wp);

					writtenBytes += packets.back().GetSize();

					wp = wpnext;
					packets.pop_back();
					blocks++;
				}
			}
		}


		void Read(DataHeader& packet) override
		{
			bool read = false;
			while (!read)
			{
				uint32_t stored = blocks.load();
				if (stored > 0)
				{
					uint32_t rpnext = rp + blockSize;
					if (rpnext >= bufferSize)
					{
						rpnext = 0;
					}
					std::copy((byte*)buffer.data() + rp, (byte*)buffer.data() + rp + sizeof(packet.prio),
						reinterpret_cast<byte*>(std::addressof(packet.prio)));

					rp += sizeof(packet.prio);

					std::copy((byte*)buffer.data() + rp, (byte*)buffer.data() + rp + sizeof(packet.sentTime),
						reinterpret_cast<byte*>(std::addressof(packet.sentTime)));

					rp += sizeof(packet.sentTime);

					std::copy((byte*)buffer.data() + rp, (byte*)buffer.data() + rp + sizeof(packet.receiveTime),
						reinterpret_cast<byte*>(std::addressof(packet.receiveTime)));

					rp += sizeof(packet.sentTime);

					std::copy((byte*)buffer.data() + rp, (byte*)buffer.data() + rp + sizeof(packet.allocated),
						reinterpret_cast<byte*>(std::addressof(packet.allocated)));

					rp += sizeof(packet.allocated);

					std::copy((byte*)buffer.data() + rp, (byte*)buffer.data() + rp + sizeof(packet.data),
						reinterpret_cast<byte*>(std::addressof(packet.data)));

					readBytes += packet.GetSize();

					rp = rpnext;
					blocks--;
					read = true;
				}
			}
		}

		void Read(std::vector<DataHeader>& packets, uint32_t howMany)
		{
			while (packets.size() < howMany)
			{
				uint32_t stored = blocks.load();
				if (stored > 0)
				{
					uint32_t rpnext = rp + blockSize;
					if (rpnext >= bufferSize)
					{
						rpnext = 0;
					}
					packets.push_back(DataHeader());

					std::copy((byte*)buffer.data() + rp, (byte*)buffer.data() + rp + sizeof(packets.back().prio),
						reinterpret_cast<byte*>(std::addressof(packets.back().prio)));

					rp += sizeof(packets.back().prio);

					std::copy((byte*)buffer.data() + rp, (byte*)buffer.data() + rp + sizeof(packets.back().sentTime),
						reinterpret_cast<byte*>(std::addressof(packets.back().sentTime)));

					rp += sizeof(packets.back().sentTime);

					std::copy((byte*)buffer.data() + rp, (byte*)buffer.data() + rp + sizeof(packets.back().receiveTime),
						reinterpret_cast<byte*>(std::addressof(packets.back().receiveTime)));

					rp += sizeof(packets.back().sentTime);

					std::copy((byte*)buffer.data() + rp, (byte*)buffer.data() + rp + sizeof(packets.back().allocated),
						reinterpret_cast<byte*>(std::addressof(packets.back().allocated)));

					rp += sizeof(packets.back().allocated);

					std::copy((byte*)buffer.data() + rp, (byte*)buffer.data() + rp + sizeof(packets.back().data),
						reinterpret_cast<byte*>(std::addressof(packets.back().data)));

					readBytes += packets.back().GetSize();

					rp = rpnext;
					blocks--;
				}
			}
		}

		uint64_t GetWrittenBytes() const
		{
			return writtenBytes;
		}

		uint64_t GetReadBytes() const
		{
			return readBytes;
		}
		void reset()
		{
			writtenBytes = 0;
			readBytes = 0;
		}
	};
}