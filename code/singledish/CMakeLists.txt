#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2014-2015 by NAOJ (in the framework of the ALMA collaboration)
#
# This file is part of CASA.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

casa_add_library( singledish
    SingleDish/BaselineTable.cc
    SingleDish/BLParameterParser.cc
    SingleDish/LineFindingUtils.cc
    SingleDish/LineFinder.cc
    SingleDish/SDMSManager.cc
    SingleDish/SingleDishMS.cc
)

install (FILES
    SingleDish/BaselineTable.h
    SingleDish/BLParameterParser.h
    SingleDish/LineFindingUtils.h
    SingleDish/LineFinder.h
    SingleDish/SDMSManager.h
    SingleDish/SingleDishMS.h
    DESTINATION include/casacode/singledish/SingleDish
)

### traditional c++ codes for testing ###
#casa_add_assay( singledish
#    SingleDish/test/tLineFinder.cc
#    )
### google tests ###
casa_add_google_test (NOT_ON_APPLE MODULES singledish SOURCES SingleDish/test/LineFindingUtils_GTest.cc)
casa_add_google_test (NOT_ON_APPLE MODULES singledish SOURCES SingleDish/test/LineFinder_GTest.cc)
casa_add_google_test (MODULES singledish SOURCES SingleDish/test/SingleDishMS_GTest.cc)
