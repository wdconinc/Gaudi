#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency
from Configurables import ReadAlg, ReadTES, FileRecordDataSvc

# I/O
GaudiPersistency()
FileRecordDataSvc(IncidentName="NEW_FILE_RECORD")
FileCatalog(Catalogs=["xmlcatalog_file:NEWROOTIO.xml"])
esel = EventSelector(OutputLevel=DEBUG, PrintFreq=1, FirstEvent=1)
esel.Input = [
    "DATAFILE='PFN:NEWROOTIO.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'",
]

# Algorithms
evtAlgs = GaudiSequencer(
    "EventAlgs",
    Members=[
        ReadAlg(
            OutputLevel=VERBOSE, IncidentName=FileRecordDataSvc().IncidentName)
    ],
    VetoObjects=["FSR"])
fsrAlgs = GaudiSequencer(
    "FSRAlgs", Members=[ReadTES(Locations=["FSR"])], RequireObjects=["FSR"])

# Application setup
app = ApplicationMgr()
# - Algorithms
app.TopAlg = [evtAlgs, fsrAlgs]
# - Events
app.EvtMax = -1
app.HistogramPersistency = "NONE"

RootCnvSvc(OutputLevel=INFO)
SequencerTimerTool(OutputLevel=WARNING)
