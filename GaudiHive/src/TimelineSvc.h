
#ifndef GAUDIHIVE_TIMELINESVC_H
#define GAUDIHIVE_TIMELINESVC_H

#include "GaudiKernel/ITimelineSvc.h"
#include "GaudiKernel/Service.h"

#include <string>

#include "tbb/concurrent_vector.h"

class TimelineSvc : public extends<Service, ITimelineSvc>
{

public:
  StatusCode initialize() override;
  StatusCode reinitialize() override;
  StatusCode finalize() override;

  void registerTimelineEvent( const TimelineEvent& ) override;
  void getTimelineEvent( TimelineEvent& ) const override;

  bool isEnabled() const override { return m_isEnabled; }

  using extends::extends;

  // Destructor.
  ~TimelineSvc() override = default;

private:
  void outputTimeline();

  Gaudi::Property<std::string> m_timelineFile{this, "TimelineFile", "timeline.csv", ""};
  Gaudi::Property<bool>        m_isEnabled{this, "RecordTimeline", false, "Enable recording of the timeline events"};
  Gaudi::Property<bool>        m_dumpTimeline{this, "DumpTimeline", false, "Enable dumping of the timeline events"};
  Gaudi::Property<bool>        m_partial{this, "Partial", false, ""};

  tbb::concurrent_vector<TimelineEvent> m_events;
};

#endif
