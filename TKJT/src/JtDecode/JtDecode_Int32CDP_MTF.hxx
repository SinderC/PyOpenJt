// JT format reading and visualization tools
// Copyright (C) 2026 Open CAD Viewer contributors
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

#ifndef _JtDecode_Int32CDP_MTF_HeaderFile
#define _JtDecode_Int32CDP_MTF_HeaderFile

#include <JtDecode_Int32CDP.hxx>

//! Move-to-front pseudo-codec (JT 10, ISO 14306-4 11.2.1): the values are split into the
//! values first seen ("window values") and, per value, its position in a 16-entry
//! most-recently-used window or an escape meaning "next new value" ("window offsets").
class JtDecode_Int32CDP_MTF : public JtDecode_Int32CDP::EncodedData
{
public:
  JtDecode_Int32CDP_MTF (JtDecode_Int32CDP theValues, JtDecode_Int32CDP theOffsets)
    : myValues (theValues), myOffsets (theOffsets) {}

  //! Get expected count of output values.
  Standard_EXPORT int32_t GetOutValCount() const Standard_OVERRIDE;

  //! Decode the loaded data.
  Standard_EXPORT Decoded::Mover Decode() Standard_OVERRIDE;

private:
  JtDecode_Int32CDP myValues;
  JtDecode_Int32CDP myOffsets;
};

#endif
