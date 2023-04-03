#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
""" HistoUtils python module
    This module is deprecated use 'GaudiPython.HistoUtils' instead
"""
from GaudiPython.HistoUtils import *  # noqa: F401 F403

from GaudiPython import deprecation

deprecation("Use 'GaudiPython.HistoUtils' module instead of 'HistoUtils'")
