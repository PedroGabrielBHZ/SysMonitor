#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  Process(long pid);
  std::string User() const;
  std::string Command() const;
  std::string Ram() const;
  float CpuUtilization() const;
  long Pid() const;
  long int UpTime() const;
  bool operator<(Process const& a) const;

 private:
  long pid;
};

#endif