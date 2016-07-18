//# VisCalGlobals.h: Declaration/Implementation of VisCalGlobals
//# Copyright (C) 1996,1997,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#

#ifndef SYNTHESIS_VISCALGLOBALS_H
#define SYNTHESIS_VISCALGLOBALS_H

#include <casa/aips.h>
#include <synthesis/MeasurementComponents/VisCal.h>
#include <synthesis/MeasurementComponents/SolvableVisCal.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Forward declaration:
class VisCal;
class VisSet;

// Create a specialized VisCal from VisSet
VisCal* createVisCal(const String& type, VisSet& vs);

// Create a specialized VisCal from msname, nAnt, nSpw
VisCal* createVisCal(const String& type, String msname, Int MSnAnt, Int MSnSpw);

// Create a specialized VisCal from nAnt
VisCal* createVisCal(const String& type, const Int& nAnt);

// Create a specialized SolvableVisCal from VisSet
SolvableVisCal* createSolvableVisCal(const String& type, VisSet& vs);

// Create a specialized VisCal from msname, nAnt, nSpw
SolvableVisCal* createSolvableVisCal(const String& type, String msname, Int MSnAnt, Int MSnSpw);

// Create a specialized SolvableVisCal from nAnt
SolvableVisCal* createSolvableVisCal(const String& type, const Int& nAnt);

// Parameter axis slicer for caltables
//   (experimental)

 namespace viscal {
   Slice calParSlice(String caltabname, String what, String pol);
   Slice calParSliceByType(String type, String what, String pol);
 }

} //# NAMESPACE CASA - END

#endif

