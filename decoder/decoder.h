#ifndef decoder_H
#define decoder_H 1
#include "Rtypes.h"

#pragma pack (push, 1)
struct rpeak_type {
  Float_t Area;
  Float_t Height;
  Float_t Width;
  Float_t Time; //exact time relative to pulse start (from 1st deriv)
  UChar_t Ch; //Channel number
  UChar_t Type; //peak type
};
#pragma pack (pop)

typedef std::vector<rpeak_type> pulse_vect;

#endif
