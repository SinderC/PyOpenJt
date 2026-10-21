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

#include <JtDecode_Int32CDP_MTF.hxx>

int32_t JtDecode_Int32CDP_MTF::GetOutValCount() const
{
  return myOffsets.GetOutValCount();
}

JtDecode_Int32CDP_MTF::Decoded::Mover JtDecode_Int32CDP_MTF::Decode()
{
  Decoded aValues  (decodePackage (myValues));
  Decoded aOffsets (decodePackage (myOffsets));

  const int kWindow = 16;
  int32_t   aWindow[kWindow];
  int       aWindowSize = 0;
  int32_t   aNextValue  = 0;

  Decoded aResults (aOffsets.Count());
  for (int32_t i = 0; i < aOffsets.Count(); i++)
  {
    const int32_t anOffset = aOffsets[i];
    int32_t aValue;
    int     aFrom;
    if (anOffset < 0 || anOffset >= aWindowSize)
    {
      // escape: take the next new value; it enters the window at the front
      aValue = aNextValue < aValues.Count() ? aValues[aNextValue++] : 0;
      aFrom  = aWindowSize < kWindow ? aWindowSize++ : kWindow - 1;
    }
    else
    {
      aValue = aWindow[anOffset];
      aFrom  = anOffset;
    }
    for (int k = aFrom; k > 0; k--) // move to front
      aWindow[k] = aWindow[k - 1];
    aWindow[0]  = aValue;
    aResults[i] = aValue;
  }
  return aResults.Move();
}
