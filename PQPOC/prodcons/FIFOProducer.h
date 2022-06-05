#pragma once
#include "../dataheader/DataHeader.h"
#include <random>

namespace pqpoc
{
	class FIFOProducer
	{
		const uint32_t prioSize;
		unsigned int randSeed;
		std::vector<uint32_t> prios;
		uint32_t pidx;
		uint64_t allocated;
	public:
		FIFOProducer(uint32_t p1, uint32_t p2, unsigned int ts) : prioSize(100),
			randSeed(ts),
			pidx(0),
			allocated(0)
		{
			prios = std::vector<uint32_t>(prioSize);
			uint32_t lim1 = p1 < prioSize ? p1 : prioSize;
			uint32_t lim2 = lim1 + p2 < prioSize ? lim1 + p2 : prioSize;
			uint32_t i = 0;
			for (; i < lim1; i++)
			{
				prios[i] = 1;
			}
			for (; i < lim2; i++)
			{
				prios[i] = 2;
			}
			for (; i < prioSize; i++)
			{
				prios[i] = 3;
			}
			std::random_device rd;
			std::mt19937 gen(rd());
			std::shuffle(prios.begin(), prios.end(), gen);
		}

		uint32_t nextPrio()
		{
			uint32_t p = prios[pidx];
			pidx++;
			if (pidx >= prioSize)
			{
				pidx = 0;
			}
			return p;
		}


		void produce(uint32_t packages, QUEUE* fifo)
		{
			srand(randSeed);
			uint32_t produced = 0;
			while (produced < packages)
			{
				DataHeader packet;
				packet.prio = nextPrio();
				uint64_t bytes = (rand() % 10) * 1000000 + 1000;
				allocated += packet.allocate(bytes);
				//packet.setAll(1);
				packet.sentTime = std::chrono::system_clock::now().time_since_epoch().count();
				fifo->Write(packet);
				produced++;
			}
		}

		uint64_t getAllocated() const
		{
			return allocated;
		}

		/*
void produce(uint32_t packages, FIFOQueue& rb)
{
	srand(randSeed);
	uint32_t sent = 0;
	while (sent < packages)
	{
		std::vector<DataHeader> packets;
		uint32_t lim = packages - sent > 10 ? 10 : packages - sent;
		uint64_t t = std::chrono::system_clock::now().time_since_epoch().count();
		for (uint32_t i = 0; i < lim; i++)
		{
			packets.push_back(DataHeader());
			packets.back().prio = nextPrio();
			packets.back().sentTime = t;
			uint32_t bytes = rand() % 20 + 1;
			allocated += packets.back().allocate(bytes);
			packets.back().setAll(1);
		}
		rb.WriteIntoBuffer(packets);
		sent += lim;
	}
}
*/
	};
}