#ifndef GAUDIHIVE_THREADPOOLSVC_H
#define GAUDIHIVE_THREADPOOLSVC_H

#include "GaudiKernel/Service.h"
#include "GaudiKernel/IThreadPoolSvc.h"
#include "GaudiKernel/IThreadInitTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "tbb/spin_mutex.h"
#include "tbb/task_scheduler_init.h"
#include "boost/thread.hpp"

#include <vector>

class ThreadPoolSvc: public extends1<Service, IThreadPoolSvc> {
public:
  /// Constructor
  ThreadPoolSvc( const std::string& name, ISvcLocator* svc );

  /// Destructor
  ~ThreadPoolSvc();

  /// Initialise
  virtual StatusCode initialize();

  /// Finalise
  virtual StatusCode finalize();


  virtual StatusCode initPool(const int& poolSize);
  virtual int poolSize() const {return m_threadPoolSize;}
  virtual bool isInit() const { return m_init; }

  virtual std::vector<IThreadInitTool*> getThreadInitTools() const;

private:

  bool m_init;
  int m_threadPoolSize;

  ToolHandleArray<IThreadInitTool> m_threadInitTools;

  tbb::spin_mutex m_initMutex;

  tbb::task_scheduler_init* m_tbbSchedInit;

  boost::barrier *m_barrier;


};

#endif
