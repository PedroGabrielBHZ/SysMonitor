#include "format.h"

#include <chrono>
#include <iomanip>
#include <string>

/**
 * @brief Format seconds count to hour-minute-seconds.
 *
 * @param s seconds count
 * @return hour-minute-seconds
 */
std::string Format::ElapsedTime(long s) {
  // cast seconds count to chrono seconds object
  std::chrono::seconds seconds{s};

  // get hours count from total seconds
  std::chrono::hours hours =
      std::chrono::duration_cast<std::chrono::hours>(seconds);

  // subtract hours count in seconds from total seconds
  seconds -= std::chrono::duration_cast<std::chrono::seconds>(hours);

  // get minutes count from total remaining seconds
  std::chrono::minutes minutes =
      std::chrono::duration_cast<std::chrono::minutes>(seconds);

  // set seconds as remaining seconds
  seconds -= std::chrono::duration_cast<std::chrono::seconds>(minutes);

  // build time string stream
  std::stringstream ss{};
  ss << std::setw(2) << std::setfill('0') << hours.count()     // HH
     << std::setw(1) << ":"                                    // :
     << std::setw(2) << std::setfill('0') << minutes.count()   // MM
     << std::setw(1) << ":"                                    // :
     << std::setw(2) << std::setfill('0') << seconds.count();  // SS

  // return formated time
  return ss.str();
}