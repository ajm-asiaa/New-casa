//# CalSolVi2Organizer.cc: Definition of CalSolVi2Organizer
//# Copyright (C) 2016
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


//#include <casacore/ms/MeasurementSets/MeasurementSet.h>
#include <synthesis/MeasurementComponents/CalSolVi2Organizer.h>
#include <msvis/MSVis/SimpleSimVi2.h>
#include <msvis/MSVis/AveragingTvi2.h>
#include <casa/aips.h>
#include <casa/iostream.h>


namespace casa { //# NAMESPACE CASA - BEGIN

using namespace vi;
  
// Constructor
CalSolVi2Organizer::CalSolVi2Organizer() : 
  ss_(NULL),
  cal_(NULL),
  chanave_(NULL),
  timeave_(NULL),
  factories_(),
  vi_(NULL)
{
  // nothing else
}

// Destructor
CalSolVi2Organizer::~CalSolVi2Organizer()
{
  cleanUp();
}

vi::VisibilityIterator2& CalSolVi2Organizer::makeFullVI() {

  AlwaysAssert(factories_.nelements()>0, AipsError);

  if (vi_) delete vi_;
  vi_=new VisibilityIterator2(factories_);

  return *vi_;
}

Int CalSolVi2Organizer::countSolutions(Vector<Int>& nChunkPerSolve) {

  // TBD:  Make this smart w.r.t. combine options...

  AlwaysAssert(factories_.nelements()>0, AipsError);

  // A local (temporary) VI2 using just the bottom layer
  //  This assumes each solutions constains data
  //  from one or more chunk in the bottom layer
  //  (i.e., one chunk contains data for at most
  //   one distict solution)
  // Using this VI2 (rather than the full one) ensures
  //  that we do not invoke the averaging layers
  //  in non-trivial ways
  //  (NB:the loop below will need to vi.origin() to
  //   examine times and indices to handle 
  //   non-trivial solution-counting cases, e.g.,
  //   combine='spw', etc.)
  VisibilityIterator2 vi(factories_(Slice(0,1,1)));

  // Q: is MS sort init cost too high here?

  Int nchunks(0);
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    ++nchunks;
  }
  //  not needed!
  //    for (vi.origin();vi.more();vi.next()) 

  // Trivial, for now
  //  Later, this will be filled in comb*-dependent manner
  nChunkPerSolve.resize(nchunks);
  nChunkPerSolve.set(1);

  return nchunks;

}


void CalSolVi2Organizer::addSimIO() {

  //  Must be first specified layer
  AlwaysAssert(factories_.nelements()==0, AipsError);

  SimpleSimVi2Parameters ss;
  ss_=new SimpleSimVi2LayerFactory(ss);

  factories_.resize(1);
  factories_[0]=ss_;
}

void CalSolVi2Organizer::addSimIO(const SimpleSimVi2Parameters& ss) {

  //  Must be first specified layer
  AlwaysAssert(factories_.nelements()==0, AipsError);

  ss_=new SimpleSimVi2LayerFactory(ss);

  factories_.resize(1);
  factories_[0]=ss_;
}

void CalSolVi2Organizer::addCalForSolving(Float calfactor) {

  // Must not have added one already!
  AlwaysAssert(!cal_, AipsError);

  //  Must be at least one other layer already...
  AlwaysAssert(factories_.nelements()>0, AipsError);

  CalibratingParameters cp(calfactor);
  // Make the layer factory
  cal_= new vi::CalSolvingVi2LayerFactory(cp);

  // Add it to the list...
  this->appendFactory(cal_);

}

//
void CalSolVi2Organizer::addCalForSolving(VisEquation& ve) {

  // Must not have added one already!
  AlwaysAssert(!cal_, AipsError);

  //  Must be at least one other layer already...
  AlwaysAssert(factories_.nelements()>0, AipsError);

  // Make the layer factory
  cal_= new CalSolvingVi2LayerFactoryByVE(&ve);

  // Add it to the list...
  this->appendFactory(cal_);

}


void CalSolVi2Organizer::addChanAve(Int chanbin) {

  // Must not have added one already!
  AlwaysAssert(!chanave_, AipsError);

  //  Must be at least one other layer already...
  AlwaysAssert(factories_.nelements()>0, AipsError);

  Record config;
  config.define("chanbin",chanbin);

  // Make the layer factory
  chanave_= new ChannelAverageTVILayerFactory(config);

  // Add it to the list...
  this->appendFactory(chanave_);

}

// Add time-averaging layer factory
void CalSolVi2Organizer::addTimeAve(Float timebin) {

  // Must not have added one already!
  AlwaysAssert(!timeave_, AipsError);

  //  Must be at least one other layer already...
  AlwaysAssert(factories_.nelements()>0, AipsError);

  AveragingOptions aveopt(AveragingOptions::AverageCorrected|
                          AveragingOptions::CorrectedFlagWeightAvgFromWEIGHT|
                          AveragingOptions::AverageModel|
                          AveragingOptions::ModelPlainAvg);
  AveragingParameters avepar(timebin,0.0,SortColumns(),aveopt);
  timeave_ = new AveragingVi2LayerFactory(avepar);

  // Add it to the list...
  this->appendFactory(timeave_);

}


void CalSolVi2Organizer::appendFactory(ViiLayerFactory* f) {

  Int nf=factories_.nelements();
  factories_.resize(nf+1,True);  // copies exising values
  factories_[nf]=f;

}


void CalSolVi2Organizer::cleanUp() {

  if (vi_) delete vi_; vi_=NULL;
  if (ss_) delete ss_; ss_=NULL;
  if (cal_) delete cal_;  cal_=NULL;
  if (chanave_) delete chanave_;  chanave_=NULL;
  if (timeave_) delete timeave_;  timeave_=NULL;
  factories_.resize(0);

}

void CalSolVi2Organizer::barf() {
  
  cout << "ss_        = " << ss_ << endl;
  cout << "cal_       = " << cal_ << endl;
  cout << "chanave_   = " << chanave_ << endl;
  cout << "timeave_   = " << timeave_ << endl;
  cout << "factories_ = " << factories_ << endl;

  if (vi_)
    cout << "VI Layers: " << vi_->ViiType() << endl;

}

void CalSolVi2Organizer::deriveVI2Sort(Block<Int>& sortcols,  // Double& iterInterval,
				       Bool combobs,Bool combscan,
				       Bool combfld,Bool combspw) 
{

/*  NB: interval is now just the prescribed averaging interval: min(solint,preavg)
     ---> no need to re-interpret it

  // Interpret solution interval for the VI2
  iterInterval=(max(interval(),DBL_MIN));
  if (interval() < 0.0) {   // means no interval (infinite solint)
    iterInterval=0.0;
    interval()=DBL_MAX;
  }
*/  
  Bool verbose(False);
  if (verbose) {
    //cout << "   interval()=" << interval() ;
    cout << boolalpha << "; combobs =" << combobs;
    cout << boolalpha << "; combscan=" << combscan;
    cout << boolalpha << "; combfld =" << combfld ;
    cout << boolalpha << "; combspw =" << combspw ;
    cout << endl;
  }
  
  Int nsortcol(4+(combscan?0:1)+(combobs?0:1) );  // include room for scan,obs
  sortcols.resize(nsortcol);
  Int i(0);
  sortcols[i++]=MS::ARRAY_ID;
  if (!combobs) sortcols[i++]=MS::OBSERVATION_ID;  // force obsid boundaries
  if (!combscan) sortcols[i++]=MS::SCAN_NUMBER;  // force scan boundaries
  if (!combfld) sortcols[i++]=MS::FIELD_ID;      // force field boundaries
  if (!combspw) sortcols[i++]=MS::DATA_DESC_ID;  // force spw boundaries
  sortcols[i++]=MS::TIME;
  //if (combspw() || combfld()) iterInterval=DBL_MIN;  // force per-timestamp chunks
  if (combfld) sortcols[i++]=MS::FIELD_ID;      // effectively ignore field boundaries
  if (combspw) sortcols[i++]=MS::DATA_DESC_ID;  // effectively ignore spw boundaries
  
  if (verbose) {
    cout << " sort sortcols: " << Vector<Int>(sortcols) << endl;
    //    cout << "iterInterval = " << iterInterval << endl;
  }
  
}





} //# NAMESPACE CASA - END
