// Created by: Peter KURNEV
// Copyright (c) 2010-2014 OPEN CASCADE SAS
// Copyright (c) 2007-2010 CEA/DEN, EDF R&D, OPEN CASCADE
// Copyright (c) 2003-2007 OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN, CEDRAT,
//                         EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.


#include <Bnd_Box.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BOPAlgo_SectionAttribute.hxx>
#include <BOPAlgo_Tools.hxx>
#include <BOPCol_DataMapOfIntegerInteger.hxx>
#include <BOPCol_DataMapOfIntegerListOfInteger.hxx>
#include <BOPCol_ListOfShape.hxx>
#include <BOPCol_MapOfInteger.hxx>
#include <BOPDS_Curve.hxx>
#include <BOPDS_DS.hxx>
#include <BOPDS_Iterator.hxx>
#include <BOPDS_PaveBlock.hxx>
#include <BOPDS_ShapeInfo.hxx>
#include <BOPDS_VectorOfInterfVV.hxx>
#include <BOPTools_AlgoTools.hxx>
#include <BRepBndLib.hxx>
#include <BRep_TVertex.hxx>
#include <BRep_Tool.hxx>
#include <gp_Pnt.hxx>
#include <IntTools_Context.hxx>
#include <NCollection_BaseAllocator.hxx>
#include <Precision.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>

//=======================================================================
// function: PerformVV
// purpose: 
//=======================================================================
void BOPAlgo_PaveFiller::PerformVV() 
{
  Standard_Boolean bWithSubShape;
  Standard_Integer n1, n2, iFlag, aSize, k, aNbBlocks;
  Handle(NCollection_BaseAllocator) aAllocator;
  //
  myErrorStatus=0;
  //
  myIterator->Initialize(TopAbs_VERTEX, TopAbs_VERTEX);
  aSize=myIterator->ExpectedLength();
  if (!aSize) {
    return; 
  }
  //
  BOPDS_VectorOfInterfVV& aVVs=myDS->InterfVV();
  aVVs.SetIncrement(aSize);
  //
  //-----------------------------------------------------scope f
  aAllocator=
    NCollection_BaseAllocator::CommonBaseAllocator();
  BOPCol_IndexedDataMapOfIntegerListOfInteger aMILI(100, aAllocator);
  BOPCol_DataMapOfIntegerListOfInteger aMBlocks(100, aAllocator);
  //
  // 1. Map V/LV
  for (; myIterator->More(); myIterator->Next()) {
    myIterator->Value(n1, n2, bWithSubShape);
    //
    const TopoDS_Vertex& aV1=(*(TopoDS_Vertex *)(&myDS->Shape(n1))); 
    const TopoDS_Vertex& aV2=(*(TopoDS_Vertex *)(&myDS->Shape(n2))); 
    //
    iFlag=BOPTools_AlgoTools::ComputeVV(aV1, aV2);
    if (!iFlag) {
      BOPAlgo_Tools::FillMap(n1, n2, aMILI, aAllocator);
    }
  } 
  //
  // 2. Make blocks
  BOPAlgo_Tools::MakeBlocksCnx(aMILI, aMBlocks, aAllocator);
  //
  // 3. Make vertices
  aNbBlocks=aMBlocks.Extent();
  for (k=0; k<aNbBlocks; ++k) {
    const BOPCol_ListOfInteger& aLI=aMBlocks.Find(k);
    //
    MakeSDVertices(aLI);
  }
  //
  BOPCol_DataMapIteratorOfDataMapOfIntegerInteger aItDMII;
  //
  BOPCol_DataMapOfIntegerInteger& aDMII=myDS->ShapesSD();
  aItDMII.Initialize(aDMII);
  for (; aItDMII.More(); aItDMII.Next()) {
    n1=aItDMII.Key();
    myDS->InitPaveBlocksForVertex(n1);
  }
  //
  //-----------------------------------------------------scope t
  aMBlocks.Clear();
  aMILI.Clear();
}

//=======================================================================
// function: PerformVV
// purpose: 
//=======================================================================
void BOPAlgo_PaveFiller::MakeSDVertices(const BOPCol_ListOfInteger& theVertIndices)
{
  TopoDS_Vertex aVSD, aVn;
  BOPCol_ListIteratorOfListOfInteger aItLI(theVertIndices);
  BOPCol_ListOfShape aLV;
  for (; aItLI.More(); aItLI.Next()) {
    Standard_Integer nX = aItLI.Value();
    Standard_Integer nSD;
    if (myDS->HasShapeSD(nX, nSD)) {
      aVSD = TopoDS::Vertex(myDS->Shape(nSD));
    }
    const TopoDS_Shape& aV = myDS->Shape(nX);
    aLV.Append(aV);
  }
  BOPTools_AlgoTools::MakeVertex(aLV, aVn);
  if (!aVSD.IsNull()) {
    // update old SD vertex with new value
    Handle(BRep_TVertex)& aTVertex =
      reinterpret_cast<Handle(BRep_TVertex)&>(const_cast<Handle(TopoDS_TShape)&>(aVSD.TShape()));
    aTVertex->Pnt(BRep_Tool::Pnt(aVn));
    aTVertex->Tolerance(BRep_Tool::Tolerance(aVn));
  }
  //
  // Append new vertex to the DS
  BOPDS_ShapeInfo aSIn;
  aSIn.SetShapeType(TopAbs_VERTEX);
  aSIn.SetShape(aVn);
  Standard_Integer n = myDS->Append(aSIn);
  //
  BOPDS_ShapeInfo& aSIDS = myDS->ChangeShapeInfo(n);
  Bnd_Box& aBox = aSIDS.ChangeBox();
  BRepBndLib::Add(aVn, aBox);
  aBox.SetGap(aBox.GetGap() + Precision::Confusion());
  //
  // Fill ShapesSD
  BOPDS_VectorOfInterfVV& aVVs = myDS->InterfVV();
  aVVs.SetIncrement(theVertIndices.Extent());
  //
  aItLI.Initialize(theVertIndices);
  for (; aItLI.More(); aItLI.Next()) {
    Standard_Integer n1 = aItLI.Value();
    myDS->AddShapeSD(n1, n);
    //
    BOPCol_ListIteratorOfListOfInteger aItLI2 = aItLI;
    aItLI2.Next();
    for (; aItLI2.More(); aItLI2.Next()) {
      Standard_Integer n2 = aItLI2.Value();
      //
      myDS->AddInterf(n1, n2);
      BOPDS_InterfVV& aVV = aVVs.Append1();
      //
      aVV.SetIndices(n1, n2);
      aVV.SetIndexNew(n);
    }
  }
}
