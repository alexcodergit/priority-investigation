#pragma once
#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include "../dataheader/DataHeader.h"

namespace pqpoc
{
	class DataStats
	{
		std::vector<uint32_t>dataSets;
		std::vector<uint64_t> totalBytes;
		std::vector<uint64_t> avgDelay;

		void reset()
		{
			for (auto& d : dataSets)
			{
				d = 0;
			}
			for (auto& d : totalBytes)
			{
				d = 0;
			}
			for (auto& d : avgDelay)
			{
				d = 0;
			}
		}

	public:
		DataStats()
		{
			dataSets = std::vector<uint32_t>(3, 0);
			totalBytes = std::vector<uint64_t>(3, 0);
			avgDelay = std::vector<uint64_t>(3, 0);
		}

		void collectStats(const std::string& fileIn, const std::string& fileOut)
		{
			reset();
			std::ifstream ifs(fileIn);
			if (ifs.is_open())
			{
				std::ofstream ofs(fileOut);
				if (ofs.is_open())
				{
					uint64_t prev = 0;
					std::string line;
					ofs << "prio,receive_time,time_delay,time_since_last_packet" << '\n';
					bool firstLine = true;
					while (std::getline(ifs, line))
					{
						uint32_t prio;
						uint64_t t1;
						uint64_t t2;
						uint32_t byteCount;
						std::istringstream iss(line);
						iss >> prio >> t1 >> t2 >> byteCount;
						dataSets[prio - 1]++;
						avgDelay[prio - 1] += (t2 - t1);
						totalBytes[prio - 1] += byteCount;
						if (firstLine)
						{
							//ofs << prio << "," << t2 << "," << t2 - t1 << "," << 0 << '\n';
							ofs << prio << "," << t2 << "," << (static_cast<double>(t2) - static_cast<double>(t1)) / 10.0 << "," << 0 << '\n';
							firstLine = false;
						}
						else
						{
							//ofs << prio << "," << t2 << "," << t2 - t1 << "," << static_cast<double>(t2 - prev) / 10.0 << '\n';
							ofs << prio << "," << t2 << "," << (static_cast<double>(t2) - static_cast<double>(t1)) / 10.0 << "," << static_cast<double>(t2 - prev) / 10.0 << '\n';
						}
						prev = t2;
					}
					ofs.close();
				}
				else
				{
					throw("Can't open file " +  fileIn + "_stats.txt");
				}
				ifs.close();
			}
			else
			{
				throw "Can't open file";
			}
		}

		void outputStats(const std::string &fileName)
		{
			std::ofstream ofs(fileName);
			if (ofs.is_open())
			{
				for (uint32_t p = 0; p < 3; p++)
				{
					ofs << "Total data sets of prio " << p + 1 << ": " << dataSets[p] << " ";
					if (dataSets[p] > 0)
					{
						ofs << "Average Delay: " << avgDelay[p] / dataSets[p] << " ";
					}
					else
					{
						ofs << "Average Delay not available ";
					}
					ofs << "Total bytes of prio " << p + 1 << " received: " << totalBytes[p] << '\n';
				}
				ofs << "Total bytes of all prios received: " << totalBytes[0] + totalBytes[1] + totalBytes[2] << '\n';
				ofs.close();
			}
		}
	};
}