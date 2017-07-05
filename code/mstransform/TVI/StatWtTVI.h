//# FreqAxisTVI.h: This file contains the interface definition of the MSTransformManager class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#ifndef STATWTTVI_H_
#define STATWTTVI_H_

#include <msvis/MSVis/TransformingVi2.h>

#include <casacore/ms/MSSel/MSSelection.h>
#include <casacore/scimath/Mathematics/StatisticsAlgorithmFactory.h>

#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <mstransform/TVI/UtilsTVI.h>
#include <stdcasa/variant.h>
#include <stdcasa/StdCasa/CasacSupport.h>

namespace casa {

namespace vi {

class StatWtTVI : public TransformingVi2 {

public:

    static const casacore::String CHANBIN;

    // The following fields are supported in the input configuration record
    // combine           String, if contains "corr", data will be aggregated
    //                   across correlations.
    // value in CHANBIN: Int or Quantity String, describes channel bin widths
    //                   in which to aggregate data within spectral windows
    //                   (spw boundaries are not crossed). If not supplied,
    //                   data for all channels in each spectral window are
    //                   aggregated.
    // minsamp:          Int, minimum number of samples required in an
    //                   aggregated set, if less than that, stats are not
    //                   computed and the data in the sample are flagged. If
    //                   not supplied, 2 is used.
    // statalg           String representing what statistics algorithm to use.
    //                   "cl", "ch", "f", "h".
    // maxiter           Int max number of iterations for Chauvenet algorithm
    // zscore            Double zscore for Chauvenet algorithm
    // center            String center for FitToHalf algorithm, "mean", "median", or "zero"
    // lside             Bool side to use for FitToHalf algorithm, True => <= center side.
    // fence             Double fence value for HingesFences algorithm
    // wtrange           Zero or two element Array<Double>. Specifies the range of "good"
    //                   weight values. Data with weights computed to be outside this
    //                   range will be flagged. Both elements must be non-negative. If
    //                   zero length, all weights are acceptable.
    // excludechans      String. MSSelection string representing channels to exclude from
    //                   weight computation.
    // datacolumn        String. Data column to use for computing weights. Supports
    //                   'data' or 'corrected'. Minimum match, case insensitive. If not
    //                   provided. 'corrected' is used.
    StatWtTVI(ViImplementation2* inputVii, const casacore::Record &configuration);

    virtual ~StatWtTVI();

    void initWeightSpectrum (const casacore::Cube<casacore::Float>& wtspec);

    void next();

    void origin();

    virtual void weightSpectrum(casacore::Cube<casacore::Float>& wtsp) const;

    virtual void weight(casacore::Matrix<casacore::Float> & wtmat) const;
    
    virtual void flag(casacore::Cube<casacore::Bool>& flagCube) const;

    virtual void flagRow (casacore::Vector<casacore::Bool> & flagRow) const;

    void summarizeFlagging() const;

    // Override unimplemented TransformingVi2 version
    void writeBackChanges(VisBuffer2* vb);

protected:

    void originChunks(casacore::Bool forceRewind);

    void nextChunk();
    
private:

    using Baseline = std::pair<casacore::uInt, casacore::uInt>;

    struct ChanBin {
        casacore::uInt start = 0;
        casacore::uInt end = 0;

        bool operator<(const ChanBin& other) const {
            if (start < other.start) {
                return true;
            }
            if (start == other.start && end < other.end) {
                return true;
            }
            return false;
        }
    };

    struct BaselineChanBin {
        Baseline baseline = make_pair(0, 0);
        casacore::uInt spw = 0;
        ChanBin chanBin;

        bool operator<(const BaselineChanBin& other) const {
            if (baseline < other.baseline) {
                return true;
            }
            if (baseline == other.baseline && spw < other.spw) {
                return true;
            }
            if (baseline == other.baseline && spw == other.spw && chanBin < other.chanBin) {
                return true;
            }
            return false;
        };
    };

    mutable casacore::Bool _weightsComputed = false;
    mutable casacore::Bool _wtSpExists = true;
    mutable casacore::Cube<casacore::Float> _newWtSp;
    mutable casacore::Matrix<casacore::Float> _newWt;
    mutable casacore::Cube<casacore::Bool> _newFlag;
    mutable casacore::Vector<casacore::Bool> _newFlagRow;
    mutable casacore::Vector<casacore::uInt> _newRowIDs;
    // the vector represents separate correlations, there will be
    // only one element in the vector if _combineCorr is true
    mutable std::map<BaselineChanBin, std::vector<casacore::Double>> _weights;
    // The key refers to the spw, the value vector refers to the
    // channel numbers within that spw that are the first, last channel pair
    // in their respective bins
    map<casacore::Int, std::vector<ChanBin>> _chanBins;
    casacore::Int _minSamp = 2;
    casacore::Bool _combineCorr = false;
    casacore::CountedPtr<
        casacore::StatisticsAlgorithm<casacore::Double,
        casacore::Array<casacore::Float>::const_iterator,
        casacore::Array<casacore::Bool>::const_iterator>
    > _statAlg;
    std::unique_ptr<std::pair<casacore::Double, casacore::Double>> _wtrange = nullptr;
    std::map<casacore::uInt, casacore::Cube<casacore::Bool>> _chanSelFlags;

    mutable casacore::uInt _nTotalPts = 0;
    mutable casacore::uInt _nNewFlaggedPts = 0;
    mutable casacore::uInt _nOrigFlaggedPts = 0;
    mutable casacore::Bool _useCorrected = true;
    mutable std::map<casacore::uInt, std::pair<casacore::uInt, casacore::uInt>> _samples;

    void _gatherAndComputeWeights() const;

    void _computeWeights(
        const map<BaselineChanBin, casacore::Cube<casacore::Complex>>& data,
        const map<BaselineChanBin, casacore::Cube<casacore::Bool>>& flags
    ) const;

    casacore::Bool _parseConfiguration(const casacore::Record &configuration);
	
    void _initialize();

    // swaps ant1/ant2 if necessary
    static Baseline _baseline(casacore::uInt ant1, casacore::uInt ant2);

    void _setChanBinMap(casacore::Int binWidth);

    void _setChanBinMap(const casacore::Quantity& binWidth);

    void _setDefaultChanBinMap();

    void _clearCache();

    void _configureStatAlg(const casacore::Record& config);

};

}

}

#endif 

