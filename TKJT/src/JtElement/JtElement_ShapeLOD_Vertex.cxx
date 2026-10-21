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

#include <JtElement_ShapeLOD_Vertex.hxx>

#include <NCollection_Handle.hxx>

#include <JtData_Reader.hxx>
#include <JtData_Message.hxx>
#include <JtData_Inflate.hxx>

#include <JtDecode_Int32CDP.hxx>
#include <JtDecode_VertexData.hxx>
#include <JtDecode_MeshCoderDriver.hxx>

#include <map>
#include <vector>

//#define NO_JT_MULTITHREADING
#include <JtData_Parallel.hxx>

IMPLEMENT_STANDARD_RTTIEXT(JtElement_ShapeLOD_Vertex, JtElement_ShapeLOD_Base)

IMPLEMENT_OBJECT_CLASS(JtElement_ShapeLOD_Vertex, "Vertex Shape LOD Object",
                       "10dd10b0-2ac8-11d1-6b-9b-00-80-c7-bb-59-97")

class JtElement_ShapeLOD_Vertex::VertexDataDecodeTask
{
public:

  VertexDataDecodeTask (JtData_SingleHandle<JtDecode_VertexData> theData, VertexData& theResult)
    : myData (theData) , myResultPtr (&theResult) {}

  void operator ()() const { *myResultPtr = const_cast <JtDecode_VertexData&> (*myData).Decode(); }

private:

  NCollection_Handle<JtDecode_VertexData> myData;
  VertexData* myResultPtr;
};

class JtElement_ShapeLOD_Vertex::MeshDecodeTask
{
  NCollection_Handle<JtDecode_MeshCoderDriver::InputData> myData;
  IndicesVec* myVertexIndicesPtr;
  IndicesVec* myNormalIndicesPtr;
  bool        myIsJt10;

public:
  MeshDecodeTask (JtData_SingleHandle<JtDecode_MeshCoderDriver::InputData> theData,
                  IndicesVec& theVertexIndices,
                  IndicesVec& theNormalIndices,
                  bool        theIsJt10)
    : myData             (theData)
    , myVertexIndicesPtr (&theVertexIndices)
    , myNormalIndicesPtr (&theNormalIndices)
    , myIsJt10           (theIsJt10) {}

  void operator ()() const
  {
    JtDecode_MeshCoderDriver aMeshCoderDriver;
    aMeshCoderDriver.SetJt10      (myIsJt10);
    aMeshCoderDriver.SetInputData (const_cast <JtDecode_MeshCoderDriver::InputData&> (*myData));
    aMeshCoderDriver.Decode       (myVertexIndicesPtr, myNormalIndicesPtr);
  }
};

//=======================================================================
//function : QuantizationParams::Read
//purpose  :
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_Vertex::QuantizationParams::Read (JtData_Reader& theReader)
{
  return theReader.ReadUniformStruct<Jt_U8> (*this);
}

//=======================================================================
//function : VertexBinding1::Read
//purpose  :
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_Vertex::VertexBinding1::Read (JtData_Reader& theReader)
{
  uint8_t aBuffer[sizeof (Jt_I32)];
  if (!theReader.ReadArray (aBuffer))
    return Standard_False;

  if (theReader.Model()->IsFileLE())
  {
    myNormalBinding    = aBuffer[0];
    myTextCoordBinding = aBuffer[1];
    myColorBinding     = aBuffer[2];
  }
  else
  {
    myNormalBinding    = aBuffer[3];
    myTextCoordBinding = aBuffer[2];
    myColorBinding     = aBuffer[1];
  }

  return Standard_True;
}

//=======================================================================
//function : VertexBinding2::Read
//purpose  :
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_Vertex::VertexBinding2::Read (JtData_Reader& theReader)
{
  if (theReader.Model()->IsFileLE())
  {
    return theReader.ReadU8  (reinterpret_cast <uint8_t&> (myLSB))
        && theReader.ReadU32 (myTextFlags)
        && theReader.Skip    (sizeof (Jt_U64) - sizeof (uint32_t) - sizeof (uint8_t) * 2)
        && theReader.ReadU8  (reinterpret_cast <uint8_t&> (myMSB));
  }
  else
  {
    return theReader.ReadU8  (reinterpret_cast <uint8_t&> (myMSB))
        && theReader.Skip    (sizeof (Jt_U64) - sizeof (uint32_t) - sizeof (uint8_t) * 2)
        && theReader.ReadU32 (myTextFlags)
        && theReader.ReadU8  (reinterpret_cast <uint8_t&> (myLSB));
  }
}

//=======================================================================
//function : VertexBinding2::IsNormalBinding
//purpose  :
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_Vertex::VertexBinding2::IsNormalBinding()
{
  return myLSB.Normal;
}

//=======================================================================
//function : VertexBinding2::IsFlagBinding
//purpose  :
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_Vertex::VertexBinding2::IsFlagBinding()
{
  return myLSB.Flag;
}

//=======================================================================
//function : VertexBinding2::IsAuxFieldBinding
//purpose  :
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_Vertex::VertexBinding2::IsAuxFieldBinding()
{
  return myMSB.Auxiliary;
}

//=======================================================================
//function : VertexBinding2::NbVertexCoordComponents
//purpose  :
//=======================================================================
Standard_Size JtElement_ShapeLOD_Vertex::VertexBinding2::NbVertexCoordComponents()
{
  switch (myLSB.Vertex)
  {
  case 1 << 0: return 2;
  case 1 << 1: return 3;
  case 1 << 2: return 4;
  default: return 0;
  }
}

//=======================================================================
//function : VertexBinding2::NbColorComponents
//purpose  :
//=======================================================================
Standard_Size JtElement_ShapeLOD_Vertex::VertexBinding2::NbColorComponents()
{
  switch (myLSB.Color)
  {
  case 1 << 0: return 3;
  case 1 << 1: return 4;
  default: return 0;
  }
}

//=======================================================================
//function : VertexBinding2::NbTextCoordComponents
//purpose  :
//=======================================================================
Standard_Size JtElement_ShapeLOD_Vertex::VertexBinding2::NbTextCoordComponents
  (Standard_Size theTexture)
{
  switch ((myTextFlags >> (theTexture * 4)) & 0x0F)
  {
  case 1 << 0: return 1;
  case 1 << 1: return 2;
  case 1 << 2: return 3;
  case 1 << 3: return 4;
  default: return 0;
  }
}

//=======================================================================
//function : Read
//purpose  : Read this entity from a translate file
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_Vertex::Read (JtData_Reader& theReader)
{
  return readVertexShapeLODData (theReader);
}

//=======================================================================
//function : Dump
//purpose  :
//=======================================================================
Standard_Integer JtElement_ShapeLOD_Vertex::Dump (Standard_OStream& theStream) const
{
  theStream << "JtElement_ShapeLOD_Vertex ";
  return JtElement_ShapeLOD_Base::Dump (theStream);
}

//=======================================================================
//function : readVertexShapeLODData
//purpose  : Read Vertex Shape LOD Data collection
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_Vertex::readVertexShapeLODData (
  JtData_Reader& theReader,
  ShapeKind      theKind)
{
  myIndices.Free();
  myVertices.Free();
  myNormals.Free();

  if (theReader.Model()->MajorVersion() < 9)
  {
    if (!JtData_Object::Read (theReader))
      return Standard_False;

    if (theReader.Model()->MajorVersion() > 7)
    {
      Jt_I16 aVersion;
      if (!theReader.ReadI16 (aVersion))
        return Standard_False;
    }

    VertexBinding1     aVertexBinding;
    QuantizationParams aQuantizationParams;
    return aVertexBinding     .Read (theReader)
        && aQuantizationParams.Read (theReader);
  }
  else
  {
    Jt_I16 aVersion;
    VertexBinding2 aVertexBinding;
    Jt_I16 aMeshVersion;
    Jt_I32 aTriID;
    Jt_I16 aVersion4;
    if (!JtElement_ShapeLOD_Base::Read (theReader)
     || !theReader.ReadVersion (aVersion)
     || !aVertexBinding.Read (theReader))
    {
      return Standard_False;
    }

    // JT 10 files wrap the TopoMesh data in an embedded logical element header (length, object
    // type ID, base type, object ID). ISO 14306-4 does not describe it; seen in NX-written files.
    if (theReader.Model()->MajorVersion() >= 10)
    {
      static const Jt_GUID aTopologicallyCompressedGUID ("f830a5ad-be4c-4fbc-9b-5f-b9-26-92-78-d2-e1");
      static const Jt_GUID aCompressedRepV1GUID         ("11c12d32-38f9-45ba-93-ba-66-f9-d5-38-dd-fb");
      const Jt_GUID&       aTopoMeshGUID = theKind == ShapeKind_PolylineSet ? aCompressedRepV1GUID
                                                                            : aTopologicallyCompressedGUID;
      Jt_I32  aLength, anObjectID;
      Jt_GUID aGUID;
      Jt_U8   aBaseType;
      if (!theReader.ReadI32  (aLength)
       || !theReader.ReadGUID (aGUID)
       || !(aGUID == aTopoMeshGUID)
       || !theReader.ReadU8   (aBaseType)
       || !theReader.ReadI32  (anObjectID))
      {
        ALARM ("Error: Unsupported JT 10 TopoMesh layout");
        return Standard_False;
      }
    }

    if (!theReader.ReadVersion (aMeshVersion)
     || !theReader.ReadI32 (aTriID)
     || !theReader.ReadVersion (aVersion4))
    {
      return Standard_False;
    }

    if (theKind == ShapeKind_TriStripSet)
      return readTopologicallyCompressedData (theReader);
    else if (theKind == ShapeKind_PolylineSet)
      return readCompressedPolylineData (theReader);
    else
      // TODO: Read TopoMesh Compressed Rep Data
      return Standard_True;
  }
}

//=======================================================================
//function : readVertexBasedShapeCompressedRepData
//purpose  : Read and decode Vertex Based Shape Compressed Rep Data collection
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_Vertex::readVertexBasedShapeCompressedRepData (JtData_Reader& theReader)
{
  // The Vertex Based Shape Compressed Rep Data collection is the
  // compressed and/or encoded representation of the vertex coordinates,
  // normal, texture coordinate, and color data for a vertex based shape. p.234

  // - Version Number is the version identifier for this Vertex Based Shape Rep Data.
  //   Version number "0x0001" is currently the only valid value.
  // - Normal Binding specifies how (at what granularity) normal vector data is supplied ("bound") for the Shape Rep
  //   in either the Lossless Compressed Raw Vertex Data
  // - Texture Coord Binding specifies how (at what granularity) texture coordinate data is supplied ("bound")
  //   for the Shape Rep in either the Lossless Compressed Raw Vertex Data or Lossy Quantized Raw Vertex Data collections.
  // - Color Binding specifies how (at what granularity) color data is supplied ("bound") for the Shape Rep in either the Lossless
  //   Compressed Raw Vertex Data or Lossy Quantized Raw Vertex Data collections.
  Jt_I16 aVersionNumber;
  Jt_U8 aNormalBinding, aTextCoordBinding, aColorBinding;
  QuantizationParams aQuantParam;
  if (!theReader.ReadI16 (aVersionNumber)
   || !theReader.ReadU8  (aNormalBinding)
   || !theReader.ReadU8  (aTextCoordBinding)
   || !theReader.ReadU8  (aColorBinding)
   || !aQuantParam.Read  (theReader))
  {
    return Standard_False;
  }

  /* Primitive List Indices is a vector of indices into the uncompressed
   * Raw Vertex Data marking the start/beginning of primitives.
   * The Primitive List Indices array uses the Int32 version of the CODEC
   * to compress and encode data. */
  Jt_VecI32 aPimitiveListIndices;
  {
    JtDecode_Int32CDP anEncodedPimitiveListIndices;
    if (!anEncodedPimitiveListIndices.Load1 (theReader))
        return Standard_False;

    aPimitiveListIndices = anEncodedPimitiveListIndices.DecodeI32 (JtDecode_Unpack_Stride1);
  }
  Jt_I32 aNbPrimitives = aPimitiveListIndices.Count() - 1;
  // In case 'aPimitiveListIndices' is empty
  if (aNbPrimitives < 0)
    return Standard_False;
  Jt_I32 aNbFaces = aPimitiveListIndices.Last() - aPimitiveListIndices.First() - aNbPrimitives * 2;

  // Read raw vertex data
  Jt_VecI32 aVertexDataIndices;
  if (aQuantParam.BitsPerVertex() == 0)
  {
    // lossless compressed raw vertex data

    // read sizes
    Jt_I32 anUncompressedSize, aCompressedSize;
    if (!theReader.ReadI32 (anUncompressedSize)
     || !theReader.ReadI32 (aCompressedSize))
    {
      return Standard_False;
    }

    // select appropriate reader for reading the segment data
    JtData_Reader* aDataReaderPtr;
    if (aCompressedSize > 0)
      // use JtData_Inflate is the data is compressed
      aDataReaderPtr = new JtData_Inflate (theReader, aCompressedSize);
    else
      // use the original reader otherwise
      aDataReaderPtr = &theReader;

    // determine number of vertices
    VertexData::SizeType aVertexCount = aPimitiveListIndices.Last();

    // allocate output storage
    JtData_CompVector <Jt_F32, VertexData::SizeType, VertexData::CompCountType>
      aVertices (aVertexCount                     , 3),
      aNormals  (aNormalBinding ? aVertexCount : 0, 3);

    // read the data
    for (VertexData::SizeType i = 0; i < aVertexCount; i++)
    {
      if (aNormalBinding == VertexBinding1::PerVertex)
        aDataReaderPtr->ReadArray (aNormals[i]);

      aDataReaderPtr->ReadArray   (aVertices[i]);
    }

    // move the data to the output vectors
    myVertices << reinterpret_cast<VertexData&> (aVertices);
    myNormals  << reinterpret_cast<VertexData&> (aNormals);

    // free the JtData_Inflate reader if it was used
    if (aDataReaderPtr != &theReader)
      delete aDataReaderPtr;

    // generate conventional OpenGL arrays
    myIndices.Allocate (aNbFaces * 3);

    Jt_I32 aPrimIdx, anOrigin;
    Jt_I32 anOffset1 = 1;
    Jt_I32 anOffset2 = 2;
    IndicesVec::SizeType aVertIdx = 0;
    for (aPrimIdx = 0; aPrimIdx < aNbPrimitives; aPrimIdx++)
    {
      for (anOrigin = aPimitiveListIndices[aPrimIdx]; anOrigin < aPimitiveListIndices[aPrimIdx + 1] - 2; ++anOrigin)
      {
        myIndices[aVertIdx++] = anOrigin;
        myIndices[aVertIdx++] = anOrigin + anOffset1;
        myIndices[aVertIdx++] = anOrigin + anOffset2;
        anOffset1 ^= 1 ^ 2;
        anOffset2 ^= 1 ^ 2;
      }
    }
  }
  else
  {
    /*
     *Read the lossy quantized raw vertex data
     */

    // create a TBB task group for parallel decoding of read data
    JtData_Parallel::TaskGroup aDecodeTasks;

    // read unique vertex coords data and start decoding it
    {
      JtDecode_VertexData::Handle anEncodedVertices =
        JtDecode_VertexData::LoadQuantizedCoords (theReader);

      if (!anEncodedVertices)
        return Standard_False;

      aDecodeTasks.Run (VertexDataDecodeTask (anEncodedVertices, myVertices));
    }

    // read unique vertex normals data if present and start decoding it
    if (aNormalBinding != VertexBinding1::None)
    {
      JtDecode_VertexData::Handle anEncodedNormals =
        JtDecode_VertexData::LoadQuantizedNormals (theReader);

      if (!anEncodedNormals)
        return Standard_False;

      aDecodeTasks.Run (VertexDataDecodeTask (anEncodedNormals, myNormals));
    }

    /*
     * Read vertex data indices.
     * The Compressed Vertex Index List uses the Int32 version of the coded to compress
     * and encode data.
     */
    {
      JtDecode_Int32CDP anEncodedVertexDataIndices;
      if (!anEncodedVertexDataIndices.Load1 (theReader))
          return Standard_False;

      aVertexDataIndices = anEncodedVertexDataIndices.DecodeI32 (JtDecode_Unpack_StripIdx);
    }

    // generate conventional OpenGL arrays
    myIndices.Allocate (aNbFaces * 3);

    Jt_I32 aPrimIdx, anOrigin;
    Jt_I32 anOffset1 = 1;
    Jt_I32 anOffset2 = 2;
    IndicesVec::SizeType aVertIdx = 0;
    for (aPrimIdx = 0; aPrimIdx < aNbPrimitives; aPrimIdx++)
    {
      for (anOrigin = aPimitiveListIndices[aPrimIdx]; anOrigin < aPimitiveListIndices[aPrimIdx + 1] - 2; ++anOrigin)
      {
        myIndices[aVertIdx++] = aVertexDataIndices[anOrigin];
        myIndices[aVertIdx++] = aVertexDataIndices[anOrigin + anOffset1];
        myIndices[aVertIdx++] = aVertexDataIndices[anOrigin + anOffset2];
        anOffset1 ^= 1 ^ 2;
        anOffset2 ^= 1 ^ 2;
      }
    }

    // wait until all vectors are decoded
    aDecodeTasks.Wait();
  }

  return Standard_True;
}

//=======================================================================
//function : readTopologicallyCompessedData
//purpose  : Read and decode Topologically Compressed Rep Data collection
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_Vertex::readTopologicallyCompressedData (JtData_Reader& theReader)
{
  // Create a TBB task group for parallel decoding of read data
  JtData_Parallel::TaskGroup aDecodeTasks;

  // Read mesh data and start decoding it
  JtDecode_MeshCoderDriver::InputData::Handle aMeshData =
    JtDecode_MeshCoderDriver::LoadInputData (theReader);

  if (!aMeshData)
    return Standard_False;

  IndicesVec aVertexIndices, aNormalIndices;
  aDecodeTasks.Run (MeshDecodeTask (aMeshData, aVertexIndices, aNormalIndices,
                                    theReader.Model()->MajorVersion() >= 10));

  ///////////////////////////////////////////////////////////////////
  // Topologically Compressed Vertex Records data collection
  ///////////////////////////////////////////////////////////////////

  // Read parameters
  VertexBinding2 aVertexBindings;
  QuantizationParams aQuantParam;
  Jt_I32 aNumberOfVert, aNumberOfAttr;

  if (!aVertexBindings.Read (theReader)
   || !aQuantParam.Read     (theReader)
   || !theReader.ReadI32 (aNumberOfVert))
  {
    return Standard_False;
  }

  if (aNumberOfVert == 0)
    return Standard_True;

  if (!theReader.ReadI32 (aNumberOfAttr))
    return Standard_False;

  Standard_Boolean aCoordsPresent = (aVertexBindings.NbVertexCoordComponents() > 0);
  Standard_Boolean aNormalsPresent = aVertexBindings.IsNormalBinding();

  // Read vertex coords data and start decoding it
  int32_t aUniqueVerticesCount = 0;
  VertexData aUniqueVertices;
  if (aCoordsPresent)
  {
    JtDecode_VertexData::Handle anEncodedVertices =
      JtDecode_VertexData::LoadCompressedCoords (theReader);

    if (!anEncodedVertices)
      return Standard_False;

    aUniqueVerticesCount = anEncodedVertices->GetOutVertexCount();
    aDecodeTasks.Run (VertexDataDecodeTask (anEncodedVertices, aUniqueVertices));
  }

  // Read vertex normals data and start decoding it
  VertexData aUniqueNormals;
  if (aNormalsPresent)
  {
    JtDecode_VertexData::Handle anEncodedNormals =
      JtDecode_VertexData::LoadCompressedNormals (theReader);

    if (!anEncodedNormals)
      return Standard_False;

    aDecodeTasks.Run (VertexDataDecodeTask (anEncodedNormals, aUniqueNormals));
  }

  // Create a temporary vector of maps used to map combination of coord index and normal index
  // to a single common index if both coords and normals are present
  typedef std::map<int32_t, int32_t> Map;
  JtData_Vector<Map, int32_t> aMaps;
  if (aCoordsPresent && aNormalsPresent)
    aMaps.Allocate (aUniqueVerticesCount);

  // Wait until all vectors are decoded
  aDecodeTasks.Wait();

  // If both coords and normals are present,
  // convert the data from separately indexed vectors of unique coords and normals
  // to vectors representing unique combinations of coords and normals with each
  // combination indexed by a single index
  if (aCoordsPresent && aNormalsPresent)
  {
    // build the output single vector of indices
    // and a auxiliary vector of indices of unique combinations of source
    // indices of coords and normals
    myIndices.Allocate (aVertexIndices.Count());
    IndicesVec anIdxIndices (aVertexIndices.Count());
    int32_t aUniquePairsCount = 0;

    for (int32_t anIdxIdx = 0; anIdxIdx < aVertexIndices.Count(); anIdxIdx++)
    {
      int32_t aVertexIdx = aVertexIndices[anIdxIdx];
      int32_t aNormalIdx = aNormalIndices[anIdxIdx];

      Map& aMap = aMaps[aVertexIdx];
      Map::iterator anIt = aMap.find (aNormalIdx);

      if (anIt == aMap.end())
      {
        aMap.insert (anIt, Map::value_type (aNormalIdx, aUniquePairsCount));
        anIdxIndices[aUniquePairsCount] = anIdxIdx;
        myIndices[anIdxIdx] = aUniquePairsCount++;
      }
      else
      {
        myIndices[anIdxIdx] = anIt->second;
      }
    }

    // free the memory used by the vector of maps since it will not be needed anymore
    aMaps.Free();

    // build the output vectors of coords and normals using the auxiliary vector
    myVertices.Allocate (aUniquePairsCount, aUniqueVertices.CompCount());
    myNormals .Allocate (aUniquePairsCount, aUniqueNormals.CompCount());

    for (int32_t aUniquePairIdx = 0; aUniquePairIdx < aUniquePairsCount; aUniquePairIdx++)
    {
      int32_t anIdxIdx = anIdxIndices[aUniquePairIdx];
      myVertices[aUniquePairIdx] = aUniqueVertices[aVertexIndices[anIdxIdx]];
      myNormals [aUniquePairIdx] = aUniqueNormals [aNormalIndices[anIdxIdx]];
    }
  }

  // If only coords are present, use the coords vector and its indices
  else if (aCoordsPresent)
  {
    myVertices << aUniqueVertices;
    myIndices  << aVertexIndices;
  }

  // If only normals are present, use the normals vector and its indices
  else if (aNormalsPresent)
  {
    myNormals << aUniqueNormals;
    myIndices << aNormalIndices;
  }

  // Success
  return Standard_True;
}

//=======================================================================
//function : readCompressedPolylineData
//purpose  : Read the TopoMesh Compressed Rep Data V1 of a polyline set (ISO 14306-3 7.2.2.4): the
//           polylines as runs of vertex list indices, and the unique vertex coordinates. The
//           attributes of the vertices (normals, colours, ...) are not read.
//=======================================================================
Standard_Boolean JtElement_ShapeLOD_Vertex::readCompressedPolylineData (JtData_Reader& theReader)
{
  Jt_I32 aNbFaceGroups, aNbPrimitives, aNbVertexIdx;
  if (!theReader.ReadI32 (aNbFaceGroups)
   || !theReader.ReadI32 (aNbPrimitives)
   || !theReader.ReadI32 (aNbVertexIdx))
    return Standard_False;

  // Face group start indices (not needed), primitive start indices in the vertex list, and
  // the vertex list: one index per vertex of each primitive into the unique vertex coordinates.
  // The three are Lag1 predicted.
  Jt_VecI32 aFaceGroups, aPrimitives, aVertexList;
  Jt_VecI32* aLists[3] = {&aFaceGroups, &aPrimitives, &aVertexList};
  for (Jt_VecI32* aList : aLists)
  {
    JtDecode_Int32CDP aCodec;
    if (!aCodec.Load2 (theReader))
      return Standard_False;
    *aList = aCodec.DecodeI32 (JtDecode_Unpack_Lag1);
  }

  // Hash, bindings, the number of vertex attribute records, the number of unique coordinates,
  // how many vertex records use each of them, and its hash.
  Jt_I32            aHash, aNbRecords, aNbUnique, aLengthsHash;
  VertexBinding2    aBindings;
  JtDecode_Int32CDP aLengthsCodec;
  if (!theReader.ReadI32 (aHash)
   || !aBindings.Read (theReader)
   || !theReader.ReadI32 (aNbRecords)
   || !theReader.ReadI32 (aNbUnique)
   || !aLengthsCodec.Load2 (theReader))
    return Standard_False;
  Jt_VecI32 aLengths;
  aLengths = aLengthsCodec.DecodeI32 (JtDecode_Unpack_Null);
  if (!theReader.ReadI32 (aLengthsHash))
    return Standard_False;

  if (aNbUnique == 0)
    return Standard_True;
  if (aBindings.NbVertexCoordComponents() == 0)
    return Standard_False;

  JtDecode_VertexData::Handle anEncodedVertices = JtDecode_VertexData::LoadCompressedCoords (theReader);
  if (!anEncodedVertices)
    return Standard_False;
  JtData_Parallel::TaskGroup aDecodeTasks;
  aDecodeTasks.Run (VertexDataDecodeTask (anEncodedVertices, myVertices));
  aDecodeTasks.Wait();

  // The vertex list indexes vertex records; each unique coordinate serves as many consecutive records
  // as its length says.
  static const size_t aMaxRecords = 1 << 26;
  std::vector<int32_t> aRecordCoords;
  for (int32_t u = 0; u < aLengths.Count(); ++u)
  {
    if (aLengths[u] < 0 || aRecordCoords.size() + aLengths[u] > aMaxRecords)
      return Standard_False;
    aRecordCoords.insert (aRecordCoords.end(), aLengths[u], u);
  }

  // A segment between each two consecutive vertices of a polyline
  const int32_t    aNbVertexRecords = (int32_t)aRecordCoords.size();
  Standard_Boolean isValid          = aPrimitives.Count() > 1;
  Standard_Size    aNbSegments      = 0;
  for (int32_t aPrim = 0; isValid && aPrim + 1 < aPrimitives.Count(); ++aPrim)
  {
    const int32_t aFirst = aPrimitives[aPrim], aLast = aPrimitives[aPrim + 1];
    isValid = aFirst >= 0 && aFirst <= aLast && aLast <= aVertexList.Count();
    if (isValid && aLast > aFirst)
      aNbSegments += aLast - aFirst - 1;
  }
  if (!isValid)
    return Standard_False;

  myIndices.Allocate (aNbSegments * 2);
  IndicesVec::SizeType anOut = 0;
  for (int32_t aPrim = 0; aPrim + 1 < aPrimitives.Count(); ++aPrim)
  {
    for (int32_t i = aPrimitives[aPrim]; i + 1 < aPrimitives[aPrim + 1]; ++i)
    {
      const int32_t aFrom = aVertexList[i], aTo = aVertexList[i + 1];
      if (aFrom < 0 || aFrom >= aNbVertexRecords || aTo < 0 || aTo >= aNbVertexRecords)
        return Standard_False;
      myIndices[anOut++] = aRecordCoords[aFrom];
      myIndices[anOut++] = aRecordCoords[aTo];
    }
  }
  return Standard_True;
}
