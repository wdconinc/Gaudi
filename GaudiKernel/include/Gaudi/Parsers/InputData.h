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
#pragma once

#include <string>

namespace Gaudi {
  namespace Parsers {
    /// Helper class to enable ADL for parsers
    struct InputData : std::string {
      InputData( const std::string& s ) : std::string{ s } {}
      using std::string::string;
      using std::string::operator=;
    };
  } // namespace Parsers
} // namespace Gaudi
