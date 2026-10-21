// JT format reading and visualization tools
// Copyright (C) 2014-2015 OPEN CASCADE SAS
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2 of the License, or any later
// version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// Copy of the GNU General Public License is in LICENSE.txt and  
// on <http://www.gnu.org/licenses/>.

#ifndef _JtDecode_Int32CDP_Bitlength2_Header_
#define _JtDecode_Int32CDP_Bitlength2_Header_

#include <JtDecode_Int32CDP_Bits.hxx>

//! Bitlength mark 2 decoder.
class JtDecode_Int32CDP_Bitlength2 : public JtDecode_Int32CDP_Bits
{
public:

  //! Initialize the bitlength 2 dedoder by reader.
  //! JT 10 (ISO 14306-4 Annex C, BitLengthCodec3T) stores min/max/mean as 4-bit "nibbler"
  //! values and uses fixed 4-bit block headers instead of JT 9's explicit bit counts.
  JtDecode_Int32CDP_Bitlength2 (JtData_Reader& theReader, bool theIsJt10 = false)
    : JtDecode_Int32CDP_Bits (theReader), myIsJt10 (theIsJt10) {}

protected:

  //! Decode the data.
  Standard_EXPORT void decode (int32_t* theResultPtr, int32_t* theResultEnd) Standard_OVERRIDE;

  Standard_EXPORT void decodeVariableWidth (int32_t* theResultPtr, int32_t* theResultEnd);
  Standard_EXPORT void decodeFixedWidth    (int32_t* theResultPtr, int32_t* theResultEnd);

  //! Read a sign-extended value stored as 4-bit nibbles, each followed by a "more" bit.
  int32_t readNibbler();

  bool myIsJt10;
};

#endif
