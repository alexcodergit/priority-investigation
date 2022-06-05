#pragma once
#include "../queues/QUEUE.h"
#include "../dataheader/DataHeader.h"

namespace pqpoc
{
	class PQFIFOConsumerProducer
	{
	public:
		void consume(uint32_t packs, QUEUE* from, QUEUE* to)
		{
			uint32_t transferred = 0;
			while (transferred < packs)
			{
				DataHeader packet;
				from->Read(packet);
				to->Write(packet);
				transferred++;
			}
		}
	};
}