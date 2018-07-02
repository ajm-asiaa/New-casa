//# ImageFFT.h: FFT an image
//# Copyright (C) 1996,1997,1998,1999,2000,2001
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: ImageFFT.h 20291 2008-03-21 07:19:34Z gervandiepen $

#ifndef IMAGES_IMAGEFFT_H
#define IMAGES_IMAGEFFT_H

#include <casa/aips.h>
#include <images/Images/ImageInterface.h>
#include <imageanalysis/ImageTypedefs.h>

namespace casacore{

class CoordinateSystem;
class IPosition;
template<class T> class Vector;
}

namespace casa { //# NAMESPACE CASA - BEGIN


// <summary>
// FFT an image
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> <linkto class=casacore::LatticeFFT>LatticeFFT</linkto> 
//   <li> <linkto class=casacore::ImageInterface>ImageInterface</linkto> 
//   <li> <linkto class=casacore::TempImage>TempImage</linkto> 
// </prerequisite>

// <etymology>
// Take the fast Fourier Transform of an image.
// </etymology>

// <synopsis>
// This class takes the FFT of an image.  It can
// take the FFT of just the sky plane(s) of an image or
// the specified axes.
//
// When you specify axes, if any of them are a sky axis (casacore::DirectionCoordinate)
// you must give both sky axes.
//
// Masked pixels are given the value 0.0 before the FFT is taken and the
// mask is copied to the output.  Note that it is the callers responsibility
// to give the output a mask if the input is masked.  Otherwise the mask
// will not be copied to the output
//
// This class holds the FourierTransform internally in a 
// <linkto class=casacore::TempImage>TempImage</linkto> object.  This is
// in memory or on disk depending upon its size and the amount
// of memory in your computer.    The algorithm used
// is that in <linkto class=casacore::TempLattice>TempLattice</linkto>.
//
// In generating the Fourier Coordinates, it is currently
// assumed that there is no coordinate rotation.  This 
// needs to be dealt with.
// </synopsis>
//
// <example>
// <srcblock>
//
// // Make a constant image
//
//      casacore::IPosition shape(2, 10, 20);
//      casacore::PagedImage<casacore::Float> im1(shape, casacore::CoordinateUtil::defaultCoords2D(), "im1");
//      im1.set(1.0);
//
// // Create output images with masks if needed
//
//      casacore::PagedImage<casacore::Float> r1(shape, casacore::CoordinateUtil::defaultCoords2D(), "real1");
//      casacore::PagedImage<casacore::Float> i1(shape, casacore::CoordinateUtil::defaultCoords2D(), "imag1");
//      if (im1.isMasked()) {
//         casacore::LCPagedMask mask1 = casacore::LCPagedMask(casacore::RegionHandler::makeMask (r1, "mask0"));
//         mask1.set(true);
//         r1.defineRegion ("mask0", casacore::ImageRegion(mask1), casacore::RegionHandler::Masks);
//         r1.setDefaultMask("mask0");
//         casacore::LCPagedMask mask2 = casacore::LCPagedMask(casacore::RegionHandler::makeMask (i1, "mask0"));
//         mask2.set(true);
//         i1.defineRegion ("mask0", casacore::ImageRegion(mask1), casacore::RegionHandler::Masks);
//         i1.setDefaultMask("mask0");
//      }
// //
// // FFT
//
//      ImageFFT fft;
//      fft.fftsky(im1);
//
// // The coordinates and mask will be updated
//
//      fft.getReal(r1);
//      fft.getImag(i1);
// </srcblock>
// </example>


// <motivation>
// Taking the Fourier Transform of an image is a basic part of image analysis
// </motivation>

// <todo asof="1999/09/23">
//   <li> reverse transformations
// </todo>


class ImageFFT {
public:
    ImageFFT ();

	ImageFFT(const ImageFFT& other);

	// Assignment (reference semantics)
	ImageFFT& operator=(const ImageFFT& other);

	~ImageFFT();

	// Do the FFT of the sky plane to the uv plane
	// Masked pixels are set to zero before the FT
	void fftskyReal (const casacore::ImageInterface<casacore::Float>& in);

	void fftskyComplex (const casacore::ImageInterface<casacore::Complex>& in);


// Do the FFT of the specified pixel axes (true to FT).  
// The rest are iterated over.
// Masked pixels are set to zero before the FT
   void fft (const casacore::ImageInterface<casacore::Float>& in, 
             const casacore::Vector<casacore::Bool>& axes);

// Do the FFT of the specified pixel axes (true to FT).  
// The rest are iterated over
// Masked pixels are set to zero before the FT
   void fft (const casacore::ImageInterface<casacore::Complex>& in, 
             const casacore::Vector<casacore::Bool>& axes);

// Return the FFT (from the last call to fftsky or fft) in the 
// desired form.    The casacore::CoordinateSystem, MiscInfo, casacore::ImageInfo,
// history and units are copied/updated in the output image
// from the image that was FFTd.   If the input image is masked,
// and the output image has a writable mask, the mask will
// be transferred. Any output mask should be initialized to
// true before calling these functions.
// <group>
   void getComplex (casacore::ImageInterface<casacore::Complex>& out) const;
   void getReal (casacore::ImageInterface<casacore::Float>& out) const;
   void getImaginary (casacore::ImageInterface<casacore::Float>& out) const;
   void getAmplitude (casacore::ImageInterface<casacore::Float>& out) const;
   void getPhase (casacore::ImageInterface<casacore::Float>& out) const;
// </group>

private:

   SPIIC _tempImagePtr;
   SPIIF _floatImage;
   SPIIC _complexImage;
   casacore::Bool _done;

// Check axes for multi-dim FFT
   void checkAxes(const casacore::CoordinateSystem& cSys, casacore::uInt ndim, 
                  const casacore::Vector<casacore::Bool>& axes);

// Copy the  mask to the output
   void copyMask (casacore::ImageInterface<casacore::Float>& out) const;
   void copyMask (casacore::ImageInterface<casacore::Complex>& out) const;
   void copyMask (casacore::ImageInterface<casacore::Float>& out,
                  const casacore::ImageInterface<casacore::Float>& in) const;
   void copyMask (casacore::ImageInterface<casacore::Float>& out,
                  const casacore::ImageInterface<casacore::Complex>& in) const;
   void copyMask (casacore::ImageInterface<casacore::Complex>& out,
                  const casacore::ImageInterface<casacore::Float>& in) const;
   void copyMask (casacore::ImageInterface<casacore::Complex>& out,
                  const casacore::ImageInterface<casacore::Complex>& in) const;

// Copy MiscInfo, casacore::ImageInfo, casacore::Unit, logSInk to output
// <group>
   void copyMiscellaneous (casacore::ImageInterface<casacore::Float>& out) const;
   void copyMiscellaneous (casacore::ImageInterface<casacore::Complex>& out) const;
// </group>

   static void _fftskyRealToComplex (
		   casacore::ImageInterface<casacore::Complex>& out,
		   const casacore::ImageInterface<casacore::Float>& in,
		   const casacore::Vector<casacore::Int>& pixelAxes
   );

   static void _fftskyComplexToComplex (
		   casacore::ImageInterface<casacore::Complex>& out,
   		   const casacore::ImageInterface<casacore::Complex>& in,
   		   const casacore::Vector<casacore::Int>& pixelAxes
   );

   // FFT (real) given axes
   static void _fftRealToComplex(
       casacore::ImageInterface<casacore::Complex>& out,
       const casacore::ImageInterface<casacore::Float>& in,
       const casacore::Vector<casacore::Bool>& axes
   );

   // FFT (complex) given axes
   void _fftComplexToComplex(
       casacore::ImageInterface<casacore::Complex>& out,
       const casacore::ImageInterface<casacore::Complex>& in,
       const casacore::Vector<casacore::Bool>& axes
   );

// Find the sky axes in this CoordinateSystem
   casacore::Bool _findSky(
		   casacore::Int& dC, casacore::Vector<casacore::Int>& pixelAxes,
		   casacore::Vector<casacore::Int>& worldAxes, const casacore::CoordinateSystem& cSys,
		   casacore::Bool throwIt
   );

// Overwrite the coordinate system with Fourier coordinates for sky axes only
   static void _setSkyCoordinates (
		   casacore::ImageInterface<casacore::Complex>& out,
		   const casacore::CoordinateSystem& csys, const casacore::IPosition& shape,
		   casacore::uInt dC
   );

// Overwrite the coordinate system with Fourier coordinates for all desginated axes
   void _setCoordinates (
		  casacore::ImageInterface<casacore::Complex>& out,
		  const casacore::CoordinateSystem& cSys,
		  const casacore::Vector<casacore::Bool>& axes,
		  const casacore::IPosition& shape
);
};

} //# NAMESPACE CASA - END

#endif
