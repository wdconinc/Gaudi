#ifndef GAUDIEXAMPLES_INCIDENTLISTENERTESTALG_H_
#define GAUDIEXAMPLES_INCIDENTLISTENERTESTALG_H_

#include "GaudiAlg/GaudiAlgorithm.h"

#include <memory>

class IIncidentSvc;
class IncidentListenerTest;

class IncidentListenerTestAlg: public GaudiAlgorithm
{
public:
	IncidentListenerTestAlg(const std::string& name ,
                          ISvcLocator*       pSvcLocator );
	~IncidentListenerTestAlg() override = default;

	StatusCode initialize();
	StatusCode execute();
	StatusCode finalize();

	static std::string &incident();

private:
  static std::string s_incidentType;
  IIncidentSvc *m_incSvc = nullptr;
  std::array<std::unique_ptr<IncidentListenerTest>,6> m_listener;
};

#endif /*GAUDIEXAMPLES_INCIDENTLISTENERTESTALG_H_*/
