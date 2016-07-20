// Include files
#include "QotdAlg.h"

#include "GaudiKernel/MsgStream.h"

using namespace GaudiEx;

DECLARE_COMPONENT(QotdAlg)

//------------------------------------------------------------------------------
QotdAlg::QotdAlg(const std::string& name,
		 ISvcLocator* pSvcLocator) :
  Algorithm( name,     pSvcLocator ),
  m_evtCnt ( 0 )
//------------------------------------------------------------------------------
{}


//------------------------------------------------------------------------------
StatusCode QotdAlg::initialize()
//------------------------------------------------------------------------------
{
  info()
	<< "Initializing " << name() << "..."
	<< endmsg;

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode QotdAlg::execute()
//------------------------------------------------------------------------------
{
  info() << "Event #" << m_evtCnt++ << "\n"
	<< " --- famous quotes ---\n"
	<< " - God does not play dice with the Universe.\n"
	<< " - 640K of memory should be enough for anybody.\n"
	<< " - Always code as if the guy maintaining your code would be a violent psychopath knowing where you live.\n"
	<< " - In a few minutes a computer can make a mistake so great that it would have taken many men many months to equal it.\n"
	<< " - GIGO is not a design pattern.\n"
	<< " --- famous quotes --- [DONE]"
	<< endmsg;

  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode QotdAlg::finalize()
//------------------------------------------------------------------------------
{
  info() << "Finalizing..." << endmsg;
  return StatusCode::SUCCESS;
}

