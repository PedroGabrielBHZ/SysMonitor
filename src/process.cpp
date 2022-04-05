#include "process.h"

#include <unistd.h>

#include <string>

#include "linux_parser.h"

/**
 * @brief Construct a new Process:: Process object
 *
 * @param pid
 */
Process::Process(long pid) : pid(pid) {}

/**
 * @brief Return this process's ID
 *
 * @return int
 */
long Process::Pid() const { return this->pid; }

/**
 * @brief Return this process's CPU utilization
 *
 * @return float
 */
float Process::CpuUtilization() const {
  try {
    return static_cast<float>(LinuxParser::ActiveJiffies(this->pid)) /
           static_cast<float>(this->UpTime());
  } catch (...) {
    return 1.0;
  }
}

/**
 * @brief Return the command that generated this process
 *
 * @return std::string
 */
std::string Process::Command() const { return LinuxParser::Command(this->pid); }

/**
 * @brief Return this process's memory utilization
 *
 * @return std::string
 */
std::string Process::Ram() const { return LinuxParser::Ram(this->pid); }

/**
 * @brief Return the user (name) that generated this process
 *
 * @return std::string
 */
std::string Process::User() const { return LinuxParser::User(this->pid); }

/**
 * @brief Return the age of this process (in seconds)
 *
 * @return long
 */
long Process::UpTime() const {
  return LinuxParser::UpTime() - LinuxParser::UpTime(this->pid);
}

/**
 * @brief Overload the "less than" comparison operator for Process objects
 *
 * @param a
 * @return true
 * @return false
 */
bool Process::operator<(Process const& o) const {
  return CpuUtilization() > o.CpuUtilization();
}
