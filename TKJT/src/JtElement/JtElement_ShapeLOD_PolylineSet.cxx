// JT format reading and visualization tools
// Copyright (C) 2013-2015 OPEN CASCADE SAS
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

#include <JtElement_ShapeLOD_PolylineSet.hxx>
#include <JtData_Reader.hxx>

IMPLEMENT_STANDARD_RTTIEXT(JtElement_ShapeLOD_PolylineSet, JtElement_ShapeLOD_Vertex)

IMPLEMENT_OBJECT_CLASS(JtElement_ShapeLOD_PolylineSet, "Polyline Set Shape LOD Object",
                       "10dd10a1-2ac8-11d1-9b-6b-00-80-c7-bb-59-97")

//=======================================================================
//function : Read
//purpose  : Read this entity from a translate file
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_PolylineSet::Read (JtData_Reader& theReader)
{
  Jt_I16 aVersion;
  return readVertexShapeLODData (theReader, ShapeKind_PolylineSet)
      && theReader.ReadVersion (aVersion);
}

//=======================================================================
//function : Dump
//purpose  :
//=======================================================================
Standard_Integer JtElement_ShapeLOD_PolylineSet::Dump (Standard_OStream& theStream) const
{
  theStream << "JtElement_ShapeLOD_PolylineSet ";
  return JtElement_ShapeLOD_Vertex::Dump (theStream);
}
