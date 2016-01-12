from Gaudi.Configuration import *
# ============================================================================
from Configurables import GaudiExamplesCommonConf, CPUCruncher,HiveEventLoopMgr, HiveWhiteBoard

import json

#GaudiExamplesCommonConf()
# ============================================================================     

#-------------------------------------------------------------------------------
# Metaconfig

NUMBEROFEVENTS = 100
NUMBEROFEVENTSINFLIGHT = 10
NUMBEROFALGOSINFLIGHT = 100
NUMBEROFTHREADS = 10
CLONEALGOS = True
DUMPQUEUES = False
SCALE = .1
VERBOSITY = 5


NumberOfEvents = NUMBEROFEVENTS
NumberOfEventsInFlight = NUMBEROFEVENTSINFLIGHT
NumberOfAlgosInFlight = NUMBEROFALGOSINFLIGHT
NumberOfThreads = NUMBEROFTHREADS
CloneAlgos = CLONEALGOS
DumpQueues = DUMPQUEUES
Scale = SCALE
Verbosity = VERBOSITY



#-------------------------------------------------------------------------------


def load_brunel_scenario(filename):
  algs = {}
  timing = {}
  objs = []
  curr = None
  order = 0
  nodes = ('/Event', '/Event/Rec', '/Event/DAQ')
  for l in open(filename).readlines():
    if l.find('StoreTracer') == 0:
      if   l.find('Executing Algorithm') != -1:
        alg = l.split()[-1]
        if alg not in algs.keys() : algs[alg] = (order, set(),set())
        curr = alg
        order += 1
      elif l.find('Done with Algorithm') != -1:
        curr = None
      elif l.find('[EventDataSvc]') != -1 and curr:
        obj = l.split()[-1]
        if obj in nodes : continue
        if obj.find('/Event/') == 0 : obj = obj[7:]
        obj = obj.replace('/','_')
        if obj not in objs : objs.append(obj)
        talg = algs[curr]
        if l.find('RETRIEVE') != -1:
          if obj not in talg[1] : talg[1].add(obj)
        elif l.find('REGOBJ') != -1:
          if obj not in talg[2] : talg[2].add(obj)
    if l.find("TimingAuditor") != -1:
        algo = l.split()[2]#.rstrip("|")
        index = 13
        if algo.endswith("|"): 
           index = 12
           algo = algo.rstrip("|")
        if algo in algs.keys(): 
             timing[algo] = l.split()[index]
        else:
             for name in algs.keys():
                if name.startswith(algo):
                   timing[name] = l.split()[index]
             
  all_inputs = set()
  all_outputs = set()
  all_algos = []
  all_algos_inputs = []
  
  sTiming = json.dumps(timing)
  
  f = open("algTimings.json", 'w')
  print >> f, sTiming
  
  #Scale all algo timings if needed
  if Scale!=-1:
    for alg in timing.keys():
      old_timing = float(timing[alg])
      new_timing = old_timing*Scale;
      #print "Algorithm %s: %f --> %f" %(alg, old_timing, new_timing)
      timing[alg]=new_timing
  
  for i, (alg,deps) in enumerate(algs.items()):
    if alg in ["PatPVOffline","PrsADCs"]: continue
    if deps[1] or deps[2] : 
        inputs = []
        inputs = [item for item in deps[1] if item not in ("DAQ_ODIN","DAQ_RawEvent") and item not in deps[2]]
        outputs = [item for item in deps[2]]
        new_algo = CPUCruncher(alg,
                               avgRuntime=float(timing[alg]),
                               DataInputs=inputs,
                               DataOutputs=outputs,
                               OutputLevel = 6
                              )
        for item in deps[1]: 
            all_inputs.add(item)
        for item in deps[2]: 
            all_outputs.add(item)
        all_algos.append(new_algo)
        all_algos_inputs.append(inputs)
  #look for the objects that haven't been provided within the job. Assume this needs to come via input
  new_algo = CPUCruncher("input",
                         avgRuntime=1,
                         DataInputs=[],
                         DataOutputs=[item for item in all_inputs.difference(all_outputs)],
                         OutputLevel = 6
                         )
  all_algos.append(new_algo)
  all_algos_inputs.append([])
  return all_algos,all_algos_inputs
        

# Set output level threshold 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
ms = MessageSvc() 
ms.OutputLevel     =  Verbosity

crunchers,inputs = load_brunel_scenario("Brunel.TES.trace.log")

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = NumberOfEventsInFlight)
                

# Setup the Event Loop Manager
evtloop = HiveEventLoopMgr()
evtloop.MaxAlgosParallel = NumberOfAlgosInFlight
evtloop.MaxEventsParallel = NumberOfEventsInFlight
evtloop.NumThreads = NumberOfThreads 
evtloop.CloneAlgorithms = CloneAlgos
evtloop.DumpQueues = DumpQueues
evtloop.AlgosDependencies = inputs

# And the Application Manager
app = ApplicationMgr()
app.TopAlg = crunchers
app.EvtSel = "NONE" # do not use any event input
app.EvtMax = NumberOfEvents
app.EventLoop = evtloop
app.ExtSvc =[whiteboard]
#app.MessageSvcType = "TBBMessageSvc"


