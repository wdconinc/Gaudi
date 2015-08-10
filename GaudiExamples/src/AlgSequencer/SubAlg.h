#ifndef GAUDIEXAMPLE_SUBALG_H
#define GAUDIEXAMPLE_SUBALG_H 1

// Include files
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/MsgStream.h"

/** @class SubAlg
    Trivial Algorithm for tutotial purposes
    
    @author nobody
*/
class SubAlg : public Algorithm {
public:
  /// Constructor of this form must be provided
  SubAlg(const std::string& name, ISvcLocator* pSvcLocator); 

  /// Three mandatory member functions of any algorithm
  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();
};

#endif    // GAUDIEXAMPLE_SUBALG_H
