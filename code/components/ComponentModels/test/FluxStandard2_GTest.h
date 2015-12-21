//# FluxStandard2_GTest.h: definition of FluxStandard2 (tests newer standards with coefs stored external to the code )  google test            
//#                                                                            
//# Copyright (C) 2015                                                         
//# Associated Universities, Inc. Washington DC, USA.                          
//#                                                                            
//# This program is free software; you can redistribute it and/or modify it    
//# under the terms of the GNU General Public License as published by the Free 
//# Software Foundation; either version 2 of the License, or (at your option)  
//# any later version.                                                         
//#                                                                            
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for  
//# more details.                                                              
//#                                                                            
//# You should have received a copy of the GNU General Public License along    
//# with this program; if not, write to the Free Software Foundation, Inc.,    
//# 51 Franklin Street, Fifth FloorBoston, MA 02110-1335, USA                  
//#                                                                            

#ifndef COMPONENTS_COMPONENTMODELS_TEST_FLUXSTANDARD2_GTEST_H
#define COMPONENTS_COMPONENTMODELS_TEST_FLUXSTANDARD2_GTEST_H

#include <memory>
#include <gtest/gtest.h>
#include <components/ComponentModels/test/FluxStandardTest.h>

#include <casa/aips.h>
#include <casa/aipstype.h>
#include <casa/BasicSL/String.h>
#include <components/ComponentModels/FluxStandard.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MFrequency.h>

using namespace casa;

namespace test 
{

class NewFluxStandardTest: public FluxStandardTest 
{

public:

    NewFluxStandardTest();
    virtual ~NewFluxStandardTest();
    
protected:
    
    virtual void SetUp();
    virtual void TearDown();
    Bool modelExists(String modeltablename);

    String foundModelPath;
    String flxStdName;          
    String srcName;
    Double freq;
    MFrequency mfreq;
    MEpoch mtime;
    MDirection srcDir;
    String interpMethod;
    FluxStandard::FluxScale flxStdEnum;
    Vector<Double> fluxUsed;
    Flux<Double> returnFlux, returnFluxErr;
    Bool foundStd;
    String coeffsTbName;
    // expected values
    FluxStandard::FluxScale expFlxStdEnum;
    Double expFlxVal;
};

class MatchStandardTest: public NewFluxStandardTest,
       public ::testing::WithParamInterface<std::tr1::tuple<String,FluxStandard::FluxScale>>
{

public:

    MatchStandardTest();
    virtual ~MatchStandardTest();
    
protected:
    
    virtual void SetUp();
    virtual void TearDown();
    Bool matchedStandard;
    String flxStdDesc;

};//matchStandardTest


class AltSrcNameTest: public NewFluxStandardTest,
       public ::testing::WithParamInterface<std::tr1::tuple<String,String,Double,FluxStandard::FluxScale>>
{

public:

    AltSrcNameTest();
    virtual ~AltSrcNameTest();
    
protected:
    
    virtual void SetUp();
    virtual void TearDown();
};

class SetInterpMethodTest: public NewFluxStandardTest,
       public ::testing::WithParamInterface<std::tr1::tuple<String,Double>>
{

public:

    SetInterpMethodTest();
    ~SetInterpMethodTest();

protected:
     
    virtual void SetUp();
    virtual void TearDown();
};

class FluxValueTest: public NewFluxStandardTest, 
       public ::testing::WithParamInterface<std::tr1::tuple<String,String,Double,FluxStandard::FluxScale,Double>>
{

public:

    FluxValueTest();
    virtual ~FluxValueTest();
    
protected:
    
    virtual void SetUp();
    virtual void TearDown();
    //Double expFlxVal;
};

}// end namespace test

#endif /* COMPONENTS_COMPONENTMODELS_TEST_FLUXSTANDARD_GTEST_H */