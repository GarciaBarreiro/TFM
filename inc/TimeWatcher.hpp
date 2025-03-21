//
// Created by miguelyermo on 16/11/20.
//

/*
* FILENAME :  TimeWatcher.h  
* PROJECT  :  rule-based-classifier-cpp
* DESCRIPTION :
*  
*
*
*
*
* AUTHOR :    Miguel Yermo        START DATE : 11:45 16/11/20
*
*/

#ifndef RULE_BASED_CLASSIFIER_CPP_TIMEWATCHER_H
#define RULE_BASED_CLASSIFIER_CPP_TIMEWATCHER_H

#pragma once

#include <chrono>
#include <memory>
#include <ostream>
#include <string>

using std::ostream;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::chrono::high_resolution_clock;

/**
 * @author Alberto M. Esmoris Pena
 * @version 1.0
 *
 * @brief A time watcher can be used to perform and report time measures
 */
class TimeWatcher
{
	protected:
	// ***  ATTRIBUTES  *** //
	// ******************** //
	/**
   * @brief Start time point
   *
   * @see TimeWatcher::start
   */
	unique_ptr<high_resolution_clock::time_point> tStart;
	/**
   * @brief End time point
   *
   * @see TimeWatcher::end
   */
	unique_ptr<high_resolution_clock::time_point> tEnd;

	public:
	// ***  CONSTRUCTION / DESTRUCTION  *** //
	// ************************************ //
	/**
   * @brief Instantiate a time watcher
   */

	// ***  PUBLIC METHODS  *** //
	// ************************ //
	/**
   * @brief Start the time watcher, which sets the starting point for a time
   *  measure
   */
	void start();
	/**
   * @brief Stop the time watcher, which sets the ending point for a time
   *  measure
   */
	void stop();

	/**
   * @brief Obtain the elapsed time as the difference between the last
   *  start() and the last stop() invocations.
   * @return Elapsed time
   * @see TimeWatcher::start
   * @see TimeWatcher::stop
   */
	shared_ptr<high_resolution_clock::duration> getElapsedTime();

	/**
   * @brief Obtain the elapsed time as the real number of seconds
   * @return Elapsed time in real seconds
   */
	double getElapsedDecimalSeconds();
	/**
   * @brief Obtain the elapsed time as the integer number of seconds
   * @return Elapsed time in integer seconds
   */
	long getElapsedSeconds();
	/**
   * @brief Obtain the elapsed time as the integer number of milliseconds
   * @return Elapsed time in integer milliseconds
   */
	long getElapsedMillis();
  /**
   * @brief Obtain the elapsed time as the integer number of microseconds
   * @return Elapsed time in integer nanoseconds
   */
  long getElapsedMicros();
  /**
   * @brief Obtain the elapsed time as the integer number of nanoseconds
   * @return Elapsed time in integer nanoseconds
   */
	long getElapsedNanos();
	/**
   * @brief Obtain the elapsed time as a string with format "HH:MM:SS"
   * @return Elapsed time as "HH:MM:SS" string
   */
	string getElapsedFormat();

	/**
   * @brief Report elapsed seconds through specified output stream
   * @param os Output stream for the report
   * @param msg Message to be shown by the report. By default
   *  "Total elapsed seconds: "
   */
	void reportSeconds(ostream& os, const string& msg = "Total elapsed seconds: ");
	/**
   * @brief Report elapsed milliseconds through specified output stream
   * @param os Output stream for the report
   * @param msg Message to be shown by the report. By default
   *  "Total elapsed milliseconds: "
   */
	void reportMillis(ostream& os, const string& msg = "Total elapsed milliseconds: ");
	/**
   * @brief Report elapsed time through specified output stream using
   *  "HH:MM:SS" format
   * @param os Output stream for the report
   * @param msg Message to be shown by the report. By default
   *  "Total elapsed time: "
   */
	void reportFormat(ostream& os, const string& msg = "Total elapsed time: ");

	protected:
	// ***  INNER METHODS  *** //
	// *********************** //
	/**
   * @brief Check if the time watcher has null start or end time points
   *
   * @return FALSE if the time watcher does not have neither a null start
   *  nor end time point. TRUE otherwise
   */
	bool hasNulls();
};


#endif //RULE_BASED_CLASSIFIER_CPP_TIMEWATCHER_H
