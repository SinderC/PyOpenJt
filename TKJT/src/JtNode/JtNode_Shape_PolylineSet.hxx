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

#ifndef _JtNode_Shape_PolylineSet_HeaderFile
#define _JtNode_Shape_PolylineSet_HeaderFile

#include <JtNode_Shape_Vertex.hxx>

//! A Polyline Set Shape Node Element represents a collection of independent
//! and unconnected polylines. Each polyline is one primitive of the set.
class JtNode_Shape_PolylineSet : public JtNode_Shape_Vertex
{
public:
  //! Read this entity from a JT file.
  Standard_EXPORT Standard_Boolean Read (JtData_Reader& theReader) Standard_OVERRIDE;

  //! Dump this entity.
  Standard_EXPORT Standard_Integer Dump (Standard_OStream& theStream) const Standard_OVERRIDE;

  //! Bind a late loaded property to the object.
  Standard_EXPORT void BindLateLoads (const ListOfLateLoads& theLateLoads) Standard_OVERRIDE;

  DEFINE_STANDARD_RTTIEXT(JtNode_Shape_PolylineSet,JtNode_Shape_Vertex)
  DEFINE_OBJECT_CLASS (JtNode_Shape_PolylineSet)
};

DEFINE_STANDARD_HANDLE(JtNode_Shape_PolylineSet, JtNode_Shape_Vertex)

#endif // _JtNode_Shape_PolylineSet_HeaderFile
