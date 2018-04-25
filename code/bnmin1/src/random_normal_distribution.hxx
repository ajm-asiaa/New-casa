/* 
   This file is part of BNMin1 and is licensed under GNU General
   Public License version 2

 * based on boost random/normal_distribution.hpp header file
 * Copyright Jens Maurer 2000-2001
 * Distributed under the Boost Software License, Version 1.0. (See
 * http://www.boost.org/LICENSE_1_0.txt)

 */
#ifndef _BNMIN1_RANDOM_NORMAL_DISTRIBUTION_HXX__
#define _BNMIN1_RANDOM_NORMAL_DISTRIBUTION_HXX__

#include <cmath>
#include <cassert>
#include <iostream>

namespace bnmin1boost {

// deterministic Box-Muller method, uses trigonometric functions
template<class RealType = double>
class normal_distribution
{
public:
  typedef RealType input_type;
  typedef RealType result_type;

#if !defined(BOOST_NO_LIMITS_COMPILE_TIME_CONSTANTS) && !(defined(BOOST_MSVC) && BOOST_MSVC <= 1300)
    static_assert(!std::numeric_limits<RealType>::is_integer,"problems here");
#endif

  explicit normal_distribution(const result_type& mean_arg = result_type(0),
                               const result_type& sigma_arg = result_type(1))
    : _mean(mean_arg), _sigma(sigma_arg), _valid(false)
  {
    assert(_sigma >= result_type(0));
  }

  // compiler-generated copy constructor is NOT fine, need to purge cache
  normal_distribution(const normal_distribution& other)
    : _mean(other._mean), _sigma(other._sigma), _valid(false)
  {
  }

  // compiler-generated copy ctor and assignment operator are fine

  RealType mean() const { return _mean; }
  RealType sigma() const { return _sigma; }

  void reset() { _valid = false; }

  template<class Engine>
  result_type operator()(Engine& eng)
  {
#ifndef BOOST_NO_STDC_NAMESPACE
    // allow for Koenig lookup
    using std::sqrt; using std::log; using std::sin; using std::cos;
#endif
    if(!_valid) {
      _r1 = eng();
      _r2 = eng();
      _cached_rho = sqrt(-result_type(2) * log(result_type(1)-_r2));
      _valid = true;
    } else {
      _valid = false;
    }
    // Can we have a boost::mathconst please?
    const result_type pi = result_type(3.14159265358979323846);
    
    return _cached_rho * (_valid ?
                          cos(result_type(2)*pi*_r1) :
                          sin(result_type(2)*pi*_r1))
      * _sigma + _mean;
  }

#ifndef BOOST_RANDOM_NO_STREAM_OPERATORS
  template<class CharT, class Traits>
  friend std::basic_ostream<CharT,Traits>&
  operator<<(std::basic_ostream<CharT,Traits>& os, const normal_distribution& nd)
  {
    os << nd._mean << " " << nd._sigma << " "
       << nd._valid << " " << nd._cached_rho << " " << nd._r1;
    return os;
  }

  template<class CharT, class Traits>
  friend std::basic_istream<CharT,Traits>&
  operator>>(std::basic_istream<CharT,Traits>& is, normal_distribution& nd)
  {
    is >> std::ws >> nd._mean >> std::ws >> nd._sigma
       >> std::ws >> nd._valid >> std::ws >> nd._cached_rho
       >> std::ws >> nd._r1;
    return is;
  }
#endif
private:
  result_type _mean, _sigma;
  result_type _r1, _r2, _cached_rho;
  bool _valid;
};

} // namespace boost

#endif
