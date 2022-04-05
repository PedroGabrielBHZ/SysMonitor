#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <fstream>
#include <regex>
#include <set>
#include <string>

namespace LinuxParser {
// Paths
const std::string kProcDirectory{"/proc/"};
const std::string kCmdlineFilename{"/cmdline"};
const std::string kCpuinfoFilename{"/cpuinfo"};
const std::string kStatusFilename{"/status"};
const std::string kStatFilename{"/stat"};
const std::string kUptimeFilename{"/uptime"};
const std::string kMeminfoFilename{"/meminfo"};
const std::string kVersionFilename{"/version"};
const std::string kOSPath{"/etc/os-release"};
const std::string kPasswordPath{"/etc/passwd"};

// Filters
const std::string fProcesses("processes");
const std::string fRunningProcesses("procs_running");
const std::string fMemTotal("MemTotal:");
const std::string fMemFree("MemAvailable:");
const std::string fCpu("cpu");
const std::string fUID("Uid:");
const std::string fProcMem("VmRSS:");

// System
float MemoryUtilization();
long UpTime();
long TotalProcesses();
long RunningProcesses();
long Jiffies();
long ActiveJiffies();
long ActiveJiffies(long pid);
long IdleJiffies();
std::set<long> Pids();
std::string OperatingSystem();
std::string Kernel();
std::vector<std::string> CpuUtilization();

// CPU
enum CPUStates {
  kUser_ = 0,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_
};

// Processes
std::string Command(long pid);
std::string Ram(long pid);
std::string Uid(long pid);
std::string User(long pid);
long UpTime(long pid);
};  // namespace LinuxParser

#endif