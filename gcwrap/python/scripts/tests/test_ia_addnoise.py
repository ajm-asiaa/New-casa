##########################################################################
# imfit_test.py
#
# Copyright (C) 2008, 2009
# Associated Universities, Inc. Washington DC, USA.
#
# This script is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be adressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# <author>
# Dave Mehringer
# </author>
#
# <summary>
# Test suite for the CASA tool method ia.addnoise()
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the ia.addnoise() tool method
# </etymology>
#
# <synopsis>
# Test for the ia.addnoise() tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casa | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_addnoise[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.addnoise() tool method to ensure
# coding changes do not break the associated bits 
# </motivation>
#

###########################################################################
import shutil
import casac
from tasks import *
from taskinit import *
from __main__ import *
import unittest

class ia_addnoise_test(unittest.TestCase):
    
    def setUp(self):
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()
        self.assertTrue(len(tb.showcache()) == 0)
        
    def test_history(self):
        """Test history is added to image"""
        myia = self._myia
        for mytype in ['f', 'd', 'c', 'cd']:
            myia.fromshape("",[10,10], type=mytype)
            myia.addnoise()
            msgs = myia.history();
            self.assertTrue("addnoise" in msgs[-1])
            myia.done()

    def test_stats(self):
        myia = self._myia
        for mytype in ['f', 'd', 'c', 'cd']:
            myia.fromshape("",[100,100], type=mytype)
            bb = myia.getchunk()
            bb[:] = 1
            if mytype == 'c' or mytype == 'cd':
                bb[:] = 1 + 1j
            myia.putchunk(bb)
            myia.addnoise()
            bb = myia.getchunk()
            myia.done()
            mean = np.mean(bb)
            sigma = np.std(bb)
            expmean = 1
            expsigma = 1
            if mytype == 'c' or mytype == 'cd':
                expmean = 1 + 1j
                expsigma = sqrt(2.0)
            self.assertTrue(
                abs(mean - expmean) <  0.1,
                    "mean " + str(mean) + " is wrong for type " + mytype
            )
            self.assertTrue(
                abs(sigma - expsigma) <  0.03,
                "sigma " + str(sigma) + " is wrong for type " + mytype
            )

def suite():
    return [ia_addnoise_test]
