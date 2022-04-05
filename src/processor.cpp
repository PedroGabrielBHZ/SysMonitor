#include "processor.h"

#include "linux_parser.h"

/**
 * @brief Return the aggregate CPU utilization
 *
 * @return float
 */
float Processor::Utilization() {
  float total = static_cast<float>(LinuxParser::Jiffies());
  float idle = static_cast<float>(LinuxParser::IdleJiffies());

  float utilization = 1.0 - ((idle - this->idle) / (total - this->total));

  this->total = total;
  this->idle = idle;

  return utilization;
}
