#ifndef GAUDIKERNEL_ITIMELINESVC_H
#define GAUDIKERNEL_ITIMELINESVC_H

#include "GaudiKernel/IService.h"

#include <string>

#include <chrono>
typedef std::chrono::high_resolution_clock Clock;
typedef Clock::time_point time_point;

struct TimelineEvent{
	uint thread;
	uint slot;
	uint event;

	std::string algorithm;

	time_point start;
	time_point end;
};

class GAUDI_API ITimelineSvc: virtual public IService {

public:
  /// InterfaceID
  DeclareInterfaceID(ITimelineSvc,1,0);

  virtual void registerTimelineEvent(const TimelineEvent & e) = 0;
  virtual bool isEnabled() const = 0;

};

#endif
