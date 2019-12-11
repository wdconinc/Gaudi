/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#define GAUDIKERNEL_CHRONOENTITY_CPP 1
// ============================================================================
// include files
// ============================================================================
// STD & STL
// ============================================================================
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/Kernel.h"
#include "GaudiKernel/System.h"
// ============================================================================
// Boost
// ============================================================================
#include "boost/format.hpp"
// ============================================================================
/** @file
 *  implementation file for class ChronoEntity
 *  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
 *  @date:   December 1, 1999
 */
// ============================================================================
namespace {
  /// the unit used by ChronoEntity is microsecond
  constexpr double microsecond = 1; // unit here is microsecond
  constexpr double millisecond = 1000 * microsecond;
  constexpr double second      = 1000 * millisecond;
  constexpr double minute      = 60 * second;
  constexpr double hour        = 60 * minute;
  constexpr double day         = 24 * hour;
  constexpr double week        = 7 * day;
  constexpr double month       = 30 * day;
  constexpr double year        = 365 * day;

  constexpr double nanosecond = 0.001 * microsecond;
} // namespace
// ============================================================================
// start the chrono
// ============================================================================
IChronoSvc::ChronoStatus ChronoEntity::start() {
  if ( IChronoSvc::RUNNING == m_status ) { return m_status; }
  //
  // following lines could be platform dependent!
  //
  // Store in object the measured times
  m_start = System::getProcessTime();

  m_status = IChronoSvc::RUNNING;

  return m_status;
}
// ============================================================================
// stop the chrono
// ============================================================================
IChronoSvc::ChronoStatus ChronoEntity::stop() {
  if ( IChronoSvc::RUNNING != m_status ) { return m_status; }

  // following lines could be platform dependent!
  m_delta = System::getProcessTime() - m_start;

  // update the counters:

  m_user += m_delta.userTime<TimeUnit>();
  m_kernel += m_delta.kernelTime<TimeUnit>();
  m_elapsed += m_delta.elapsedTime<TimeUnit>();

  // set new status

  m_status = IChronoSvc::STOPPED;

  return m_status;
}
// ============================================================================
// print user time
// ============================================================================
std::string ChronoEntity::outputUserTime() const {
  return "Time User   : " +
         format( uTotalTime(), uMinimalTime(), uMeanTime(), uRMSTime(), uMaximalTime(), nOfMeasurements() );
}
// ============================================================================
// print system time
// ============================================================================
std::string ChronoEntity::outputSystemTime() const {
  return "Time System : " +
         format( kTotalTime(), kMinimalTime(), kMeanTime(), kRMSTime(), kMaximalTime(), nOfMeasurements() );
}
// ============================================================================
// print time
std::string ChronoEntity::outputElapsedTime() const {
  return "TimeElapsed: " +
         format( eTotalTime(), eMinimalTime(), eMeanTime(), eRMSTime(), eMaximalTime(), nOfMeasurements() );
}
// ============================================================================
// print the chrono
// ============================================================================
std::string ChronoEntity::format( const double total, const double minimal, const double mean, const double rms,
                                  const double maximal, const unsigned long number ) const {

  /// @todo: cache the format
  boost::format fmt( "Tot=%2$5.3g%1$s %4$43s #=%3$3lu" );

  static const auto tbl = {
      std::tuple{500, microsecond, " [us]"}, std::tuple{500, millisecond, " [ms]"}, std::tuple{500, second, "  [s]"},
      std::tuple{500, minute, "[min]"},      std::tuple{500, hour, "  [h]"},        std::tuple{10, day, "[day]"},
      std::tuple{5, week, "  [w]"},          std::tuple{20, month, "[mon]"},        std::tuple{-1, year, "  [y]"}};

  auto i = std::find_if( begin( tbl ), std::prev( end( tbl ) ), [&]( const std::tuple<int, double, const char*>& i ) {
    return total < std::get<0>( i ) * std::get<1>( i );
  } );
  long double unit = std::get<1>( *i );
  fmt % std::get<2>( *i ) % (double)( total / unit ) % number;

  if ( number > 1 ) {
    /// @todo: cache the format
    boost::format fmt1( "Ave/Min/Max=%2$5.3g(+-%3$5.3g)/%4$5.3g/%5$5.3g%1$s" );
    i = std::find_if(
        std::begin( tbl ), std::prev( std::end( tbl ) ),
        [&]( const std::tuple<int, double, const char*>& i ) { return total < std::get<0>( i ) * std::get<1>( i ); } );
    unit = std::get<1>( *i );
    fmt1 % std::get<2>( *i ) % (double)( mean / unit ) % (double)( rms / unit ) % (double)( minimal / unit ) %
        (double)( maximal / unit );
    fmt % fmt1.str();
  } else {
    fmt % "";
  }

  return fmt.str();
}
// ============================================================================
// compound assignment operator
// ============================================================================
ChronoEntity& ChronoEntity::operator+=( const ChronoEntity& e ) {
  // System::ProcessTime type
  m_delta += e.m_delta;

  // Summing, massaging here does not make too much sense.
  // This is done only for final reductions
  // Keep by convention the original one.
  //	m_start += e.m_start;

  // Tymevaluetypes type
  m_user += e.m_user;
  m_kernel += e.m_kernel;
  m_elapsed += e.m_elapsed;

  return *this;
}

// ============================================================================
/*  print the chrono according the format and units
 *  @param typ  the chrono type
 *  @param fmt  the format string
 *  @param unit the unit
 *  @return the string representations
 *  @see boost::format
 */
// ============================================================================
std::string ChronoEntity::outputTime( IChronoSvc::ChronoType typ, std::string_view fmt, System::TimeType unit ) const {
  boost::format _fmt( std::string{fmt} );
  // allow various number of arguments
  using namespace boost::io;
  _fmt.exceptions( all_error_bits ^ ( too_many_args_bit | too_few_args_bit ) );
  //
  double _unit = microsecond;
  switch ( unit ) {
  case System::Year:
    _unit = year;
    break;
  case System::Month:
    _unit = month;
    break;
  case System::Day:
    _unit = day;
    break;
  case System::Hour:
    _unit = hour;
    break;
  case System::Min:
    _unit = minute;
    break;
  case System::Sec:
    _unit = second;
    break;
  case System::milliSec:
    _unit = millisecond;
    break;
  case System::microSec:
    _unit = microsecond;
    break;
  case System::nanoSec:
    _unit = nanosecond;
    break;
  default:
    _unit = microsecond;
    break;
  }
  //
  const StatEntity* stat = &m_user;
  switch ( typ ) {
  case IChronoSvc::USER:
    stat = &m_user;
    break;
  case IChronoSvc::KERNEL:
    stat = &m_kernel;
    break;
  case IChronoSvc::ELAPSED:
    stat = &m_elapsed;
    break;
  default:
    stat = &m_user;
    break;
  }
  //
  _fmt % ( stat->nEntries() )           // %1 : #entries
      % ( stat->flag() / _unit )        // %2 : total time
      % ( stat->flagMean() / _unit )    // %3 : mean time
      % ( stat->flagRMS() / _unit )     // %4 : RMS  time
      % ( stat->flagMeanErr() / _unit ) // %5 : error in mean time
      % ( stat->flagMin() / _unit )     // %6 : minimal time
      % ( stat->flagMax() / _unit );    // %7 : maximal time
  //
  return _fmt.str();
}
// ==========================================================================

// ============================================================================
// The END
// ============================================================================
