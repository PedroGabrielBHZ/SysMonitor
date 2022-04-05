#ifndef SYSTEM_H
#define SYSTEM_H

#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"

class System {
 public:
  Processor& Cpu();
  std::set<Process>& Processes();
  float MemoryUtilization();
  long UpTime();
  long TotalProcesses();
  long RunningProcesses();
  void updateProcesses();
  std::string Kernel();
  std::string OperatingSystem();

 private:
  Processor cpu = Processor();
  std::set<Process> proc;
};

#endif