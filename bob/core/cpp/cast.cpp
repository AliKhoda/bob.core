/**
 * @date Wed Feb 9 16:19:18 2011 +0100
 * @author Laurent El Shafey <Laurent.El-Shafey@idiap.ch>
 *
 * @brief This file defines functions which add std::complex support to the
 * static_cast function.
 *
 * Copyright (C) Idiap Research Institute, Martigny, Switzerland
 */

#include "cast.h"

/**
  * @brief Specializations of the cast function for the std::complex type.
  */
// Complex to regular
#define COMPLEX_TO_REGULAR(COMP, REG) template<> \
  REG bob::core::cast<REG, COMP>( const COMP& in) \
  { \
    return static_cast<REG>(in.real()); \
  }

#define COMPLEX_TO_REGULAR_FULL(COMP) \
  COMPLEX_TO_REGULAR(COMP, bool) \
  COMPLEX_TO_REGULAR(COMP, int8_t) \
  COMPLEX_TO_REGULAR(COMP, int16_t) \
  COMPLEX_TO_REGULAR(COMP, int32_t) \
  COMPLEX_TO_REGULAR(COMP, int64_t) \
  COMPLEX_TO_REGULAR(COMP, uint8_t) \
  COMPLEX_TO_REGULAR(COMP, uint16_t) \
  COMPLEX_TO_REGULAR(COMP, uint32_t) \
  COMPLEX_TO_REGULAR(COMP, uint64_t) \
  COMPLEX_TO_REGULAR(COMP, float) \
  COMPLEX_TO_REGULAR(COMP, double) \
  COMPLEX_TO_REGULAR(COMP, long double)

  COMPLEX_TO_REGULAR_FULL(std::complex<float>)
  COMPLEX_TO_REGULAR_FULL(std::complex<double>)
  COMPLEX_TO_REGULAR_FULL(std::complex<long double>)

// Complex to complex
#define COMPLEX_TO_COMPLEX(FROM, TO) template<> \
  TO bob::core::cast<TO, FROM>( const FROM& in) \
  { \
    return static_cast<TO>(in); \
  }

#define COMPLEX_TO_COMPLEX_FULL(COMP) \
  COMPLEX_TO_COMPLEX(COMP, std::complex<float>) \
  COMPLEX_TO_COMPLEX(COMP, std::complex<double>) \
  COMPLEX_TO_COMPLEX(COMP, std::complex<long double>)

  COMPLEX_TO_COMPLEX_FULL(std::complex<float>)
  COMPLEX_TO_COMPLEX_FULL(std::complex<double>)
  COMPLEX_TO_COMPLEX_FULL(std::complex<long double>)

