// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/Message.cpp,v 1.9 2008/02/20 19:16:23 hmd Exp $
#include <string>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <algorithm>
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/Message.h"
#include "GaudiKernel/Timing.h"
#include "GaudiKernel/Time.h"

using namespace MSG;

namespace {
  // get the current time from the system and format it according to the format
  inline std::string formattedTime (const std::string &fmt, bool universal = false )
  {
    return Gaudi::Time::current().format(!universal, fmt);
  }
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: Constructor.
// Purpose:
// ---------------------------------------------------------------------------
//
Message::Message ( const char* src, int type, const char* msg ) :
  m_message( msg ), m_source( src ), m_type( type )
{
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: Constructor.
// Purpose:
// ---------------------------------------------------------------------------
//
Message::Message ( std::string src, int type, std::string msg ) :
  m_message( std::move(msg) ), m_source( std::move(src) ), m_type( type )
{
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: getMessage
// Purpose: Get the message string.
// ---------------------------------------------------------------------------
//
const std::string& Message::getMessage() const
{
  return m_message;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: setMessage
// Purpose: Set the message string.
// ---------------------------------------------------------------------------
//
void Message::setMessage( std::string msg )
{
  m_message = std::move(msg);
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: getType
// Purpose: Get the message type.
// ---------------------------------------------------------------------------
//
int Message::getType() const
{
  return m_type;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: setType
// Purpose: Set the message type.
// ---------------------------------------------------------------------------
//
void Message::setType( int msg_type )
{
  m_type = msg_type;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: getSource
// Purpose: Get the message source.
// ---------------------------------------------------------------------------
//
const std::string& Message::getSource() const
{
  return m_source;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: setSource
// Purpose: Set the message source.
// ---------------------------------------------------------------------------
//
void Message::setSource( std::string src )
{
  m_source = std::move(src);
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: operator <<
// Purpose:Insert the message into a stream.
// ---------------------------------------------------------------------------
//
std::ostream& operator << ( std::ostream& stream, const Message& msg )
{
  msg.makeFormattedMsg( msg.m_format );
  stream << msg.m_formatted_msg;
  return stream;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: operator <
// Purpose: comparison operator needed for maps
// ---------------------------------------------------------------------------
//
bool Message::operator < ( const Message& b )
{
  return m_type   < b.m_type ||
         m_source < b.m_source ||
         m_message < b.m_message;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: operator ==
// Purpose: comparison op.
// ---------------------------------------------------------------------------
//
bool operator == ( const Message& a, const Message& b )
{
  return a.m_source == b.m_source &&
    a.m_type == b.m_type &&
    a.m_message == b.m_message;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine:
// Purpose: Get the format string.
// ---------------------------------------------------------------------------
//
const std::string& Message::getFormat() const
{
  return m_format;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine:
// Purpose: Get the default format string.
// ---------------------------------------------------------------------------
//
const std::string Message::getDefaultFormat()
{
  return DEFAULT_FORMAT;
}


//#############################################################################
// ---------------------------------------------------------------------------
// Routine:
// Purpose: Set the format string -
//          use isFormatted() to check for valid format.
// ---------------------------------------------------------------------------
//
void Message::setFormat( std::string format ) const
{
    if (LIKELY(!format.empty())) {
        m_format = std::move(format);
    } else {
        m_format = DEFAULT_FORMAT;
    }
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine:
// Purpose: Get the time format string.
// ---------------------------------------------------------------------------
//
const std::string& Message::getTimeFormat() const
{
  return m_time_format;
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine:
// Purpose: Get the default time format string.
// ---------------------------------------------------------------------------
//
const std::string Message::getDefaultTimeFormat()
{
  return DEFAULT_TIME_FORMAT ;
}


//#############################################################################
// ---------------------------------------------------------------------------
// Routine:
// Purpose: Set the time format string -
//          use isFormatted() to check for valid format.
// ---------------------------------------------------------------------------
//
void Message::setTimeFormat( std::string timeFormat ) const
{
  m_time_format = ( timeFormat.empty() ? DEFAULT_TIME_FORMAT
                                       : std::move(timeFormat) );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: makeFormattedMsg
// Purpose: This formats the message according to the format string.
// ---------------------------------------------------------------------------
//
void Message::makeFormattedMsg( const std::string& format ) const
{
  m_formatted_msg.clear();
  auto i = format.begin();
  while( i != format.end() ) {

    // Output format string until format statement found.
    while(  i != format.end() && *i != FORMAT_PREFIX )
      m_formatted_msg += *i++;

    // Test for end of format string.
    if ( i == format.end() ) break;
    i++;

    // Find type of formatting.
    std::string this_format;
    while( i != format.end() && *i != FORMAT_PREFIX &&
           *i != MESSAGE && *i != TYPE && *i != SOURCE &&
           *i != FILL && *i != WIDTH && *i != TIME && *i != UTIME &&
           *i != JUSTIFY_LEFT && *i != JUSTIFY_RIGHT ) {
      this_format += *i++;
    }

    // Reached end of string with improper format.
    if ( i == format.end() ) {
      invalidFormat();
      break;
    }

    this_format += *i++;
    decodeFormat( this_format );
  }
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: decodeFormat
// Purpose: This the work horse that checks for a valid format string.
// ---------------------------------------------------------------------------
//
void Message::decodeFormat( const std::string& format ) const
{
  if ( ! format.empty() ) {
    const char FORMAT_TYPE = format[ format.length() - 1 ];
    const std::string FORMAT_PARAM = format.substr( 0, format.length() - 1 );

    // Now test the format.
    std::string level;
    switch( FORMAT_TYPE ) {
    case FILL:
      if ( FORMAT_PARAM.length() == 1 ) {
        m_fill = FORMAT_PARAM[0];
      }
      else
        invalidFormat();
      break;

    case TIME:
      {
        sizeField( formattedTime ( m_time_format ) );
      }
      break;

    case UTIME:
      {
        sizeField( formattedTime ( m_time_format, true ) );
      }
      break;

    case MESSAGE:
      sizeField( m_message );
      break;

    case SOURCE:
      sizeField( m_source );
      break;

    case TYPE:
      switch ( m_type )    {
#define SET(x)  case x:  level=#x;  break
        SET( NIL );
        SET( VERBOSE );
        SET( DEBUG );
        SET( INFO );
        SET( WARNING );
        SET( ERROR );
        SET( FATAL );
        case ALWAYS:       level="SUCCESS"; break;
        default:
              level = "UNKNOWN";
              break;
#undef SET
      }
      sizeField( level );
      break;

    case FORMAT_PREFIX: m_formatted_msg += FORMAT_PREFIX; break;
    case JUSTIFY_RIGHT: m_left = false; break;
    case JUSTIFY_LEFT: m_left = true; break;
    case WIDTH: setWidth( FORMAT_PARAM ); break;
    default: invalidFormat(); break;
    }
  }
  else
    invalidFormat();
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: invalidFormat.
// Purpose: called when invalid format found.
// ---------------------------------------------------------------------------
//

void Message::invalidFormat() const
{
  makeFormattedMsg( DEFAULT_FORMAT );
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: setWidth
// Purpose: Sets the minimum width of a stream field.
// ---------------------------------------------------------------------------
//

namespace {
    // Check that a container only contains digits.
    constexpr struct all_digit_t {
        template <typename C>
        bool operator()(const C& c) const { 
            return std::all_of( std::begin(c), std::end(c), 
                                [](typename C::const_reference i) { 
                                     return isdigit(i); 
            } );
        }
    } all_digits {};
}


void Message::setWidth( const std::string& formatArg ) const
{
  // Convert string to int, if string contains digits only...
  if ( all_digits(formatArg) ) m_width = std::stoi(formatArg);
  else invalidFormat();
}

//#############################################################################
// ---------------------------------------------------------------------------
// Routine: sizeField
// Purpose: Truncates or pads the text to m_width as necessary
// ---------------------------------------------------------------------------
//

void Message::sizeField( const std::string& text ) const
{
  std::string newText;
  if ( m_width == 0 || m_width == static_cast<int>( text.length() ) ) {
    newText = text;
  }
  else {

    // Truncate the text if it is too long.
    if ( m_width < static_cast<int>( text.length() ) ) {
      newText = text.substr( 0, m_width );
      for ( int i = 0, j = newText.length()-1; i < 3 && j >= 0; i++, j-- )
        newText[ j ] = '.';
    }

    // Pad the text.
    else {
      newText = std::string( m_width, m_fill );
      if ( m_left )
        newText.replace( newText.begin(), newText.begin() + text.length(),
                         text.begin(), text.end() );
      else
        newText.replace( newText.end() - text.length(), newText.end(),
                         text.begin(), text.end() );
    }
  }

  m_formatted_msg += newText;
}
