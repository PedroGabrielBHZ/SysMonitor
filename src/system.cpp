#include "system.h"

#include <unistd.h>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

/**
 * @brief
 *
 * @return Processor&
 */
Processor& System::Cpu() { return this->cpu; }

/**
 * @brief
 *
 * @return std::set<Process>&
 */
std::set<Process>& System::Processes() { return this->proc; }

/**
 * @brief
 *
 */
void System::updateProcesses() {
  this->proc.clear();
  for (auto pid : LinuxParser::Pids()) {
    this->proc.emplace(pid);
  }
}

// Return the system's kernel identifier (string)
std::string System::Kernel() { return LinuxParser::Kernel(); }

// Return the system's memory utilization
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

// Return the operating system name
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

// Return the number of processes actively running on the system
long System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

// Return the total number of processes on the system
long System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

// Return the number of seconds since the system started running
long System::UpTime() { return LinuxParser::UpTime(); }