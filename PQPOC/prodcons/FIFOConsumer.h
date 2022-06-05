#pragma once
#include<string>
#include<fstream>
#include<chrono>
#include "../dataheader/DataHeader.h"
#include "../queues/FIFOQueue.h"

namespace pqpoc
{
	class FIFOConsumer
	{
		uint64_t deallocated;
		std::string fileName;
	public:
		FIFOConsumer(const std::string& fn) : deallocated(0), fileName(fn) {}

		void consume(uint32_t packs, QUEUE* fifo)
		{
			std::ofstream ofs(fileName);
			if (ofs.is_open())
			{
				uint32_t consumed = 0;
				while (consumed < packs)
				{
					DataHeader packet;
					fifo->Read(packet);
					uint64_t dealloc = packet.deallocate();
					deallocated += dealloc;
					packet.receiveTime = std::chrono::system_clock::now().time_since_epoch().count();
					ofs << packet.prio << " " << packet.sentTime << " " << packet.receiveTime << " " << dealloc << '\n';
					consumed++;
				}
				ofs.close();
			}
		}

		uint64_t getDeallocated() const
		{
			return deallocated;
		}

		/*
void consume(uint32_t packs, FIFOQueue& rb)
{
	std::ofstream ofs(fileName);
	if (ofs.is_open())
	{
		uint32_t received = 0;
		while (received < packs)
		{
			std::vector<DataHeader> packets;
			uint32_t lim = packs - received < 10 ? packs - received : 10;
			rb.ReadFromBuffer(packets, lim);
			uint64_t t = std::chrono::system_clock::now().time_since_epoch().count();
			for (auto& pa : packets)
			{
				pa.receiveTime = t;
				ofs << pa.prio << " " << pa.sentTime << " " << pa.receiveTime << " ";

				for (uint32_t i = 0; i < pa.allocated; i++)
				{
					ofs << (int)pa.getValue(i) << " ";
				}
				ofs << '\n';
				deallocated += pa.deallocate();
			}
			received += lim;
		}
		ofs.close();
	}
}
*/
	};
}