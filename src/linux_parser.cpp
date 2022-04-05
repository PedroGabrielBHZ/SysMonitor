#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <filesystem>
#include <iostream>
#include <set>
#include <string>
#include <vector>

/**
 * @brief Fetch a value by key in system's file.
 *
 * @tparam T
 * @param keyFilter key to filter
 * @param filename file to be searched
 * @return T
 */
template <typename T>
T findValueByKey(std::string const& keyFilter, std::string const& filename) {
  T value;
  std::string line;
  std::string key;
  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    // loop through file lines
    while (std::getline(stream, line)) {
      std::istringstream lstream(line);
      while (lstream >> key >> value) {
        // key found: return key's value
        if (key == keyFilter) {
          return value;
        }
      }
    }
  }
  stream.close();

  // key not found
  return value;
};

// Read OS data
std::string LinuxParser::OperatingSystem() {
  std::string line;
  std::string key;
  std::string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream lstream(line);
      while (lstream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  filestream.close();
  return value;
}

/**
 * @brief Kernel info
 *
 * @return std::string
 */
std::string LinuxParser::Kernel() {
  std::string os;
  std::string version;
  std::string kernel;
  std::string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream lstream(line);
    lstream >> os >> version >> kernel;
  }
  stream.close();
  return kernel;
}

/**
 * @brief Get set of pids.
 *
 * @return std::vector<long>
 */
std::set<long> LinuxParser::Pids() {
  std::set<long> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    if (file->d_type == DT_DIR) {
      std::string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        pids.emplace(std::stoi(filename));
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
  return 1.0 - (findValueByKey<float>(fMemFree, kMeminfoFilename) /
                findValueByKey<float>(fMemTotal, kMeminfoFilename));
}

/**
 * @brief Read and return the system uptime.
 *
 * @return system uptime in seconds
 */
long LinuxParser::UpTime() {
  std::string line;
  long uptime;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream lstream(line);
    lstream >> uptime;
  }
  stream.close();
  return uptime;
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
long LinuxParser::ActiveJiffies(long pid) {
  std::string line;
  std::string value;
  std::vector<std::string> values;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream lstream(line);
    while (lstream >> value) {
      values.push_back(value);
    }
  }
  stream.close();

  // values cannot be accessed by index: return dummy value
  if (values.size() < 17) {
    return 1;
  }

  // CPU time spent in user code, measured in clock ticks
  float utime(1);
  if (std::all_of(values[13].begin(), values[13].end(), isdigit))
    utime = std::stof(values[13]);

  // CPU time spent in kernel code, measured in clock ticks
  float stime(1);
  if (std::all_of(values[14].begin(), values[14].end(), isdigit))
    stime = std::stof(values[14]);

  // waited-for children's CPU time spent in user code (in clock ticks)
  float cutime(1);
  if (std::all_of(values[15].begin(), values[15].end(), isdigit))
    cutime = std::stof(values[15]);

  // waited-for children's CPU time spent in kernel code (in clock ticks)
  float cstime(1);
  if (std::all_of(values[16].begin(), values[16].end(), isdigit))
    cstime = std::stof(values[16]);

  return static_cast<long>((utime + stime + cutime + cstime) /
                           static_cast<float>(sysconf(_SC_CLK_TCK)));
}

/**
 * @brief Read and return the number of active jiffies for the system
 *
 * @return system's active jiffies
 */
long LinuxParser::ActiveJiffies() {
  auto jiffies = CpuUtilization();

  // values cannot be accessed by index: return dummy value
  if (jiffies.size() < 9) {
    return 1;
  }

  return std::stol(jiffies[CPUStates::kUser_]) +
         std::stol(jiffies[CPUStates::kNice_]) +
         std::stol(jiffies[CPUStates::kSystem_]) +
         std::stol(jiffies[CPUStates::kIRQ_]) +
         std::stol(jiffies[CPUStates::kSoftIRQ_]) +
         std::stol(jiffies[CPUStates::kSteal_]);
}

/**
 * @brief Read and return the number of idle jiffies for the system
 *
 * @return idle jiffies for the system
 */
long LinuxParser::IdleJiffies() {
  auto jiffies = CpuUtilization();

  // values cannot be accessed by index: return dummy value
  if (jiffies.size() < 9) {
    return 1;
  }

  return std::stol(jiffies[CPUStates::kIdle_]) +
         std::stol(jiffies[CPUStates::kIOwait_]);
}

/**
 * @brief Read and return CPU utilization
 *
 * @return std::vector<std::string>
 */
std::vector<std::string> LinuxParser::CpuUtilization() {
  std::string line;
  std::string value;
  std::string cpu;
  std::vector<std::string> cpuJiffies;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream lstream(line);
    lstream >> cpu;
    while (lstream >> value) {
      cpuJiffies.push_back(value);
    }
  }
  stream.close();
  return cpuJiffies;
}

/**
 * @brief Read and return the total number of processes
 *
 * @return total number of processes
 */
long LinuxParser::TotalProcesses() {
  return findValueByKey<long>(fProcesses, kStatFilename);
}

/**
 * @brief Read and return the number of running processes
 *
 * @return number of running processes
 */
long LinuxParser::RunningProcesses() {
  return findValueByKey<long>(fRunningProcesses, kStatFilename);
}

/**
 * @brief Read and return the command associated with a process
 *
 * @param pid process PID
 * @return command associated with a process
 */
std::string LinuxParser::Command(long pid) {
  std::string command;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, command);
  }
  stream.close();
  return command;
}

/**
 * @brief Read and return the memory used by a process
 *
 * @param pid process PID
 * @return memory used by a process
 */
std::string LinuxParser::Ram(long pid) {
  std::string line;
  std::string key;
  double memory(0);
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream lstream(line);
      lstream >> key;
      if (key == fProcMem) {
        lstream >> memory;
        memory /= 1024.0;
        break;
      }
    }
  }
  stream.close();
  return std::to_string(memory).substr(0, 7);
}

/**
 * @brief Read and return the user ID associated with a process
 *
 * @param pid process PID
 * @return user ID associated with a process
 */
std::string LinuxParser::Uid(long pid) {
  return findValueByKey<std::string>(fUID, std::to_string(pid) + kStatFilename);
}

/**
 * @brief Read and return the user associated with a process
 *
 * @param pid process PID
 * @return user associated with a process
 */
std::string LinuxParser::User(long pid) {
  std::string id;
  std::string x;
  std::string user;
  std::string line;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream lstream(line);
      lstream >> user >> x >> id;
      if (id == Uid(pid)) {
        break;
      }
    }
  }
  stream.close();
  return user;
}

/**
 * @brief Read and return the uptime of a process
 *
 * @param pid process PID
 * @return uptime of a process
 */
long LinuxParser::UpTime(long pid) {
  std::string line;
  std::string value;
  std::vector<std::string> values;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream lstream(line);
    while (lstream >> value) {
      values.push_back(value);
    }
  }
  stream.close();

  // values cannot be accessed by index: return dummy value
  if (values.size() < 22) {
    return 1.0;
  }

  return std::stof(values[21]) / static_cast<float>(sysconf(_SC_CLK_TCK));
}
