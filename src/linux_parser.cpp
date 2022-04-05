#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

/**
 * @brief Kernel info
 *
 * @return string
 */
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

/**
 * @brief Calculate the system memory utilization.
 *
 * @return system memory utilization
 */
float LinuxParser::MemoryUtilization() {
  float memTotal(1), memFree(1);
  string line, key, value;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          memTotal = std::stof(value);
        }
        if (key == "MemAvailable:") {
          memFree = std::stof(value);
        }
      }
    }
  }
  return (memTotal - memFree) / memTotal;
}

/**
 * @brief Read and return the system uptime
 *
 * @return system uptime in seconds
 */
long LinuxParser::UpTime() {
  string line, uptimeString;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptimeString;
    return std::stol(uptimeString);
  }
  return -1;
}

/**
 * @brief Read and return the number of jiffies for the system
 *
 * @return total jiffies
 */
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

/**
 * @brief Read and return the number of active jiffies for a PID
 *
 * @param pid process PID
 * @return active jiffies for a PID
 */
long LinuxParser::ActiveJiffies(int pid) {
  string line, value;
  vector<string> values;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      values.push_back(value);
    }
  }

  // CPU time spent in user code, measured in clock ticks
  long utime = stol(values[13]);

  // CPU time spent in kernel code, measured in clock ticks
  long stime = stol(values[14]);

  // waited-for children's CPU time spent in user code (in clock ticks)
  long cutime = stol(values[15]);

  // waited-for children's CPU time spent in kernel code (in clock ticks)
  long cstime = stol(values[16]);

  return (utime + stime + cutime + cstime) / sysconf(_SC_CLK_TCK);
}

/**
 * @brief Read and return the number of active jiffies for the system
 *
 * @return system's active jiffies
 */
long LinuxParser::ActiveJiffies() {
  auto jiffies = CpuUtilization();
  return stol(jiffies[CPUStates::kUser_]) + stol(jiffies[CPUStates::kNice_]) +
         stol(jiffies[CPUStates::kSystem_]) + stol(jiffies[CPUStates::kIRQ_]) +
         stol(jiffies[CPUStates::kSoftIRQ_]) +
         stol(jiffies[CPUStates::kSteal_]);
}

/**
 * @brief Read and return the number of idle jiffies for the system
 *
 * @return idle jiffies for the system
 */
long LinuxParser::IdleJiffies() {
  auto jiffies = CpuUtilization();
  long idling = stol(jiffies[CPUStates::kIdle_]);
  long ioAwait = stol(jiffies[CPUStates::kIOwait_]);
  return idling + ioAwait;
}

/**
 * @brief Read and return CPU utilization
 *
 * @return vector<string>
 */
vector<string> LinuxParser::CpuUtilization() {
  string line, value;
  vector<string> cpuJiffies;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      if (value != "cpu") {
        cpuJiffies.push_back(value);
      }
    }
  }
  return cpuJiffies;
}

/**
 * @brief Read and return the total number of processes
 *
 * @return total number of processes
 */
int LinuxParser::TotalProcesses() {
  string line, key, value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") {
          return std::stoi(value);
        }
      }
    }
  }
  return -1;
}

/**
 * @brief Read and return the number of running processes
 *
 * @return number of running processes
 */
int LinuxParser::RunningProcesses() {
  string line, key, value;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") {
          return std::stoi(value);
        }
      }
    }
  }
  return -1;
}

/**
 * @brief Read and return the command associated with a process
 *
 * @param pid process PID
 * @return command associated with a process
 */
string LinuxParser::Command(int pid) {
  string command;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, command);
    return command;
  }
  return "-";
}

/**
 * @brief Read and return the memory used by a process
 *
 * @param pid process PID
 * @return memory used by a process
 */
string LinuxParser::Ram(int pid) {
  string line, key;
  long memory(0);
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize:") {
        linestream >> memory;
        memory /= 1000;
        break;
      }
    }
  }
  return std::to_string(memory);
}

/**
 * @brief Read and return the user ID associated with a process
 *
 * @param pid process PID
 * @return user ID associated with a process
 */
string LinuxParser::Uid(int pid) {
  string line, key, uid;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "Uid:") {
        linestream >> uid;
        return uid;
      }
    }
  }
  return "-";
}

/**
 * @brief Read and return the user associated with a process
 *
 * @param pid process PID
 * @return user associated with a process
 */
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string id, x, temp, line;
  string name = "---";
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);

      linestream >> temp >> x >> id;
      if (id == uid) {
        name = temp;
        break;
      }
    }
  }
  return name;
}

/**
 * @brief Read and return the uptime of a process
 *
 * @param pid process PID
 * @return uptime of a process
 */
long LinuxParser::UpTime(int pid) {
  string line, value;
  vector<string> values;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      values.push_back(value);
    }
  }
  return stol(values[21]) / sysconf(_SC_CLK_TCK);
}
