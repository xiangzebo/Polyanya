/*
 * StatisticsUtil.h
 *
 *  Created on: Jan 2, 2019
 *      Author: idm-lab
 */

#ifndef APPS_SUBGOALGRAPH_UTILS_STATISTICS_STATISTICSUTIL_H_
#define APPS_SUBGOALGRAPH_UTILS_STATISTICS_STATISTICSUTIL_H_
#include <cstring>
#include <vector>

template<class S, class V>
inline double CalculateMean(std::vector<S> & vec, V val) {
  if (vec.size() == 0)
    return 0;

  long double sum = 0;
  for (unsigned int i = 0; i < vec.size(); i++) {
    sum+= val(vec[i]);
  }
  return sum/vec.size();
}

template<class S, class V>
inline double CalculateMD(std::vector<S> & vec, V val) {
  if (vec.size() == 0)
    return 0;

  long double mean = CalculateMean(vec, val);
  long double variance = 0;
  for (unsigned int i = 0; i < vec.size(); i++) {
    variance+= fabs(val(vec[i]) - mean);
  }
  return variance / vec.size();
}

template<class S, class V>
inline double CalculateSD(std::vector<S> & vec, V val) {
  if (vec.size() == 0)
    return 0;

  long double mean = CalculateMean(vec, val);
  long double variance = 0;
  for (unsigned int i = 0; i < vec.size(); i++) {
    long double d = val(vec[i]) - mean;
    variance+= d*d;
  }
  return sqrt(variance / vec.size());
}

template<class S, class V>
inline double CalculateMin(std::vector<S> & vec, V val) {
  if (vec.size() == 0)
    return 0;

  long double min = std::numeric_limits<double>::max();
  for (unsigned int i = 0; i < vec.size(); i++)
    if (val(vec[i]) < min)
     min = val(vec[i]);
  return min;
}

template<class S, class V>
inline double CalculateMax(std::vector<S> & vec, V val) {
  if (vec.size() == 0)
    return 0;

  long double max = std::numeric_limits<double>::min();
  for (unsigned int i = 0; i < vec.size(); i++)
    if (val(vec[i]) > max)
     max = val(vec[i]);
  return max;
}


template<class S, class V>
inline void ReportAll(std::string desc, StatisticsFile* sf,
                      std::vector<S> & vec, V val, double multiplier = 1) {
  sf->ReportDouble(desc + " average", CalculateMean(vec, val) * multiplier);
  sf->ReportDouble(desc + " MD",        CalculateMD(vec, val) * multiplier);
  sf->ReportDouble(desc + " SD",        CalculateSD(vec, val) * multiplier);
  sf->ReportDouble(desc + " min",      CalculateMin(vec, val) * multiplier);
  sf->ReportDouble(desc + " max",      CalculateMax(vec, val) * multiplier);
  sf->AddRemark("");
}

template<class S>
inline void ReportAll(std::string desc, StatisticsFile* sf,
                      std::vector<S> & vec, double multiplier = 1) {
  ReportAll(desc, sf, vec, [&](S v)-> double {return (double)v;}, multiplier);
}

template<class S, class V>
inline std::string GetMeanAndSDString(std::vector<S> & vec, V val,
                                      bool include_sd = false, int multiplier =
                                          1) {
  char buffer [1000];
  sprintf(buffer, "%8.2f", CalculateMean(vec,val)*multiplier);
  if (include_sd) {
    sprintf(buffer + strlen(buffer), " (%8.2f)", CalculateSD(vec,val)*multiplier);
  }
  return std::string(buffer);
}

#endif /* APPS_SUBGOALGRAPH_UTILS_STATISTICS_STATISTICSUTIL_H_ */
