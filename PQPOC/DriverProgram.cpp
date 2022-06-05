#include<vector>
#include<algorithm>
#include<iostream>
#include<cassert>
#include<fstream>
#include<sstream>
#include "stats/DataStats.h"
#include "queues/FIFOQueue.h"
#include "prodcons/FIFOProducer.h"
#include "prodcons/FIFOConsumer.h"
#include "queues/PQFIFOQueue.h"
#include "prodcons/PQFIFOConsumerProducer.h"
#include "queues/ThreadSafeHeap.h"
using namespace std;

void testFIFO(uint32_t packages, uint32_t p1, uint32_t p2, const string& fileName);
void testPQFIFO(uint32_t packages, uint32_t p1, uint32_t p2, const string& fileName);
void testPQHeap(uint32_t packages, uint32_t p1, uint32_t p2, const string& fileName);

int main()
{
	/*
	auto t1 = std::chrono::system_clock::now().time_since_epoch();
	std::this_thread::sleep_for(1ms);
	auto t2 = std::chrono::system_clock::now().time_since_epoch();
	std::cout <<  "miliseconds: " <<  std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << endl;
	std::cout <<  "microseconds: " <<  std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << endl;
	std::cout << "count: " << t2.count() - t1.count() << endl;

	t2 = t1 + std::chrono::microseconds(1);
	std::cout << "microseconds: " << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() << endl;
	std::cout << "count: " << t2.count() - t1.count() << endl;
	*/
	/*
	std::cout << std::chrono::duration_cast<std::chrono::hours>(t1).count() << endl;
	std::cout << std::chrono::duration_cast<std::chrono::minutes>(t1).count() << endl;

	std::cout << std::chrono::duration_cast<std::chrono::seconds>(t1).count() << endl;
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(t1).count() << endl;
	std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t1).count() << endl;
	*/

	string file1 = "output/pqheap";
	string file2 = "output/pqfifo";
	string file3 = "output/fifo";
	uint32_t packages = 1000000;
	testPQHeap(packages, 34, 35, file1);
	testPQHeap(packages, 34, 35, file1);
	cout << "##############################" << endl;
	testPQFIFO(packages, 34, 35, file2);
	cout << "##############################" << endl;
	testFIFO(packages, 34, 35, file3);

	return 0;
}


void testPQHeap(uint32_t packages, uint32_t p1, uint32_t p2, const string& fileName)
{

	std::cout << "Running PQ Heap Simulation" << std::endl;
	pqpoc::DataHeader dh;
	pqpoc::ThreadSafeHeap pqheap(100);
	pqpoc::ThreadSafeHeap& pqheapref(pqheap);
	pqpoc::FIFOQueue fifo(dh.GetSize(), 100);
	pqpoc::FIFOQueue& fiforef(fifo);

	pqpoc::FIFOProducer fifoProducer(p1, p2, 0);
	pqpoc::PQFIFOConsumerProducer pqfifoConsumerProducer;
	pqpoc::FIFOConsumer fifoConsumer(fileName);

	chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
	std::thread fifoProdThread(&pqpoc::FIFOProducer::produce, std::ref(fifoProducer), packages, &pqheapref);
	std::thread fifoProdConsThread(&pqpoc::PQFIFOConsumerProducer::consume,
		std::ref(pqfifoConsumerProducer), packages, &pqheapref, &fiforef);
	std::thread fifoConsThread(&pqpoc::FIFOConsumer::consume, std::ref(fifoConsumer), packages, &fiforef);

	fifoProdThread.join();
	fifoProdConsThread.join();
	fifoConsThread.join();

	chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();

	std::vector<uint64_t> bytesWritten = pqheap.GetWrittenBytes();
	std::vector<uint64_t> bytesRead = pqheap.GetReadBytes();

	ofstream ofs(fileName + "_about.txt");
	if (ofs.is_open())
	{
		ofs << "PQHeap prio1 bytes written: " << bytesWritten[0] << endl;
		ofs << "PQHeap prio2 bytes written: " << bytesWritten[1] << endl;
		ofs << "PQHeap prio3 bytes written: " << bytesWritten[2] << endl;
		ofs << "PQHeap total bytes written: " << bytesWritten[0] + bytesWritten[1] + bytesWritten[2] << endl;
		ofs << "PQHeap prio1 bytes read: " << bytesRead[0] << endl;
		ofs << "PQHeap prio2 bytes read: " << bytesRead[1] << endl;
		ofs << "PQHeap prio3 bytes read: " << bytesRead[2] << endl;
		ofs << "PQHeap total bytes read: " << bytesRead[0] + bytesRead[1] + bytesRead[2] << endl;
		ofs << "FIFO bytes written: " << fifo.GetWrittenBytes() << endl;
		ofs << "FIFO bytes read: " << fifo.GetReadBytes() << endl;

		ofs << "bytes on heap allocated: " << fifoProducer.getAllocated() << endl;
		ofs << "bytes on heap deallocated: " << fifoConsumer.getDeallocated() << endl;
	}

	pqpoc::DataStats ds;
	ds.collectStats(fileName, fileName + "_stats.txt");
	ds.outputStats(fileName + "_summery.txt");

	auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	std::cout << "Duration of simulatin: " << dur << " milliseconds" << endl;
	std::cout << "Data flow rate: " << (fifo.GetReadBytes() + fifoProducer.getAllocated()) / (1048576 * dur) << " mb / millisec" << endl;
	std::cout << "Simulation results stored in " << fileName << std::endl;
	std::cout << "Summery stats stored in " << fileName + "_summery.txt" << std::endl;
	std::cout << "Detailed stats stored in " << fileName + "_analytics.txt" << std::endl;
}

void testPQFIFO(uint32_t packages, uint32_t p1, uint32_t p2, const string& fileName)
{
	std::cout << "Running PQFIFO Simulation" << std::endl;
	pqpoc::DataHeader dh;
	pqpoc::PQFIFOQueue pqfifo(dh.GetSize(), 100);
	pqpoc::PQFIFOQueue& pqfiforef(pqfifo);
	pqpoc::FIFOQueue fifo(dh.GetSize(), 100);
	pqpoc::FIFOQueue& fiforef(fifo);

	pqpoc::FIFOProducer fifoProducer(p1, p2, 0);
	pqpoc::FIFOConsumer fifoConsumer(fileName);
	pqpoc::PQFIFOConsumerProducer pqfifoConsumerProducer;

	chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

	std::thread fifoProdThread(&pqpoc::FIFOProducer::produce, std::ref(fifoProducer), packages, &pqfiforef);
	std::thread fifoProdConsThread(&pqpoc::PQFIFOConsumerProducer::consume,
		std::ref(pqfifoConsumerProducer), packages, &pqfiforef, &fiforef);
	std::thread fifoConsThread(&pqpoc::FIFOConsumer::consume, std::ref(fifoConsumer), packages, &fiforef);

	fifoProdThread.join();
	fifoProdConsThread.join();
	fifoConsThread.join();

	chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();

	std::vector<std::uint64_t> bytesWritten = pqfifo.GetWrittenBytes();
	std::vector<std::uint64_t> bytesRead = pqfifo.GetReadBytes();

	std::ofstream ofs(fileName + "_about.txt");
	if (ofs.is_open())
	{
		ofs << "PQFIFO prio1 bytes written: " << bytesWritten[0] << endl;
		ofs << "PQFIFO prio2 bytes written: " << bytesWritten[1] << endl;
		ofs << "PQFIFO prio3 bytes written: " << bytesWritten[2] << endl;
		ofs << "PQFIFO total bytes written: " << bytesWritten[0] + bytesWritten[1] + bytesWritten[2] << endl;
		ofs << "PQFIFO prio1 bytes read: " << bytesRead[0] << endl;
		ofs << "PQFIFO prio2 bytes read: " << bytesRead[1] << endl;
		ofs << "PQFIFO prio3 bytes read: " << bytesRead[2] << endl;
		ofs << "PQFIFO total bytes read: " << bytesRead[0] + bytesRead[1] + bytesRead[2] << endl;
		ofs << "PQFIFO bytes written: " << fifo.GetWrittenBytes() << endl;
		ofs << "PQFIFO bytes read: " << fifo.GetReadBytes() << endl;

		ofs << "bytes on heap allocated: " << fifoProducer.getAllocated() << endl;
		ofs << "bytes on heap deallocated: " << fifoConsumer.getDeallocated() << endl;
	}
	pqpoc::DataStats ds;
	ds.collectStats(fileName, fileName + "_stats.txt");
	ds.outputStats(fileName + "_summery.txt");

	auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	cout << "Duration of simulatin: " << dur << " milliseconds" << endl;
	cout << "Data flow rate: " << (fifo.GetReadBytes() + fifoProducer.getAllocated()) / (1048576 * dur) << " mb / millisec" << endl;
	std::cout << "Simulation results stored in " << fileName << std::endl;
	std::cout << "Summery stats stored in " << fileName + "_summery.txt" << std::endl;
	std::cout << "Detailed stats stored in " << fileName + "_analytics.txt" << std::endl;
}

void testFIFO(uint32_t packages, uint32_t p1, uint32_t p2, const string& fileName)
{
	std::cout << "Running FIFO Simulation" << std::endl;
	pqpoc::DataHeader dh;
	pqpoc::FIFOQueue fifo(dh.GetSize(), 100);
	pqpoc::FIFOQueue& fiforef(fifo);

	pqpoc::FIFOProducer fifoProducer(p1, p2, 0);
	pqpoc::FIFOConsumer fifoConsumer(fileName);

	chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();

	std::thread fifoProdThread(&pqpoc::FIFOProducer::produce, std::ref(fifoProducer), packages, &fiforef);
	std::thread fifoConsThread(&pqpoc::FIFOConsumer::consume, std::ref(fifoConsumer), packages, &fiforef);

	fifoProdThread.join();
	fifoConsThread.join();

	chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
	std::ofstream ofs(fileName + "_about.txt");
	if (ofs.is_open())
	{
		ofs << "FIFO bytes written: " << fifo.GetWrittenBytes() << endl;
		ofs << "FIFO bytes read: " << fifo.GetReadBytes() << endl;

		ofs << "bytes on heap allocated: " << fifoProducer.getAllocated() << endl;
		ofs << "bytes on heap deallocated: " << fifoConsumer.getDeallocated() << endl;
		ofs.close();
	}


	pqpoc::DataStats ds;
	ds.collectStats(fileName, fileName + "_stats.txt");
	ds.outputStats(fileName + "_summery.txt");

	auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
	std::cout << "Duration of simulatin: " << dur << " milliseconds" << endl;
	std::cout << "Data flow rate: " << (fifo.GetReadBytes() + fifoProducer.getAllocated()) / (1048576 * dur) << " mb / millisec" << endl;
	std::cout << "Simulation results stored in " << fileName << std::endl;
	std::cout << "Summery stats stored in " << fileName + "_summery.txt" << std::endl;
	std::cout << "Detailed stats stored in " << fileName + "_analytics.txt" << std::endl;
}