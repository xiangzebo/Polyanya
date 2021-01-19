/*
 * StatisticsIO.h
 *
 *  Created on: Jan 1, 2019
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_STATISTICSFILE_H_
#define APPS_SUBGOALGRAPH_UTILS_STATISTICSFILE_H_
#include <iostream>
#include <string>
#include "DefaultParams.h"

class StatisticsFile {
 public:
  StatisticsFile(std::string statistics_file, bool echo = kDefaultEchoStatistics) {
    Open(statistics_file);
    echo_ = echo;
  }

  void Open(std::string statistics_file) {
    if (statistics_file != "") {
      s_file_.open(statistics_file.c_str());
      s_file_ << std::fixed;
    }
  }

  void Close() {
    if (s_file_.is_open())
      s_file_.close();
  }

  // Prints the value with the given description to the 'detailed' file and prints only the value to he 'brief' file.
  void ReportInt(std::string description, long val) {
//  void ReportInt(const char * description, long val) {
    if (s_file_.is_open())
      s_file_ << description << ": " << val << std::endl;
    if (!s_file_.is_open() || echo_)
      std::cout << description << ": " << val << std::endl;
  }
  void ReportDouble(std::string description, double val) {
//  void ReportDouble(const char * description, double val) {
    if (s_file_.is_open())
      s_file_ << description << ": " << val << std::endl;
    if (!s_file_.is_open() || echo_)
      std::cout << description << ": " << val << std::endl;
  }

  // Prints a sentence to the 'detailed' file, without any associated values.
  void AddRemark(std::string remark) {
    if (s_file_.is_open())
      s_file_ << remark << std::endl;
    if (!s_file_.is_open() || echo_)
      std::cout << remark << std::endl;
  }

 private:
  std::ofstream s_file_;
  bool echo_;
};


#endif /* APPS_SUBGOALGRAPH_UTILS_STATISTICSFILE_H_ */
