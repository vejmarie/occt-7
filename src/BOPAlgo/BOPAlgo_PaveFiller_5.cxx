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


#include <Precision.hxx>

#include <Bnd_Box.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BOPAlgo_SectionAttribute.hxx>
#include <BOPAlgo_Tools.hxx>
#include <BOPCol_MapOfInteger.hxx>
#include <BOPCol_NCVector.hxx>
#include <BOPCol_Parallel.hxx>
#include <BOPCol_DataMapOfShapeReal.hxx>
#include <BOPDS_CommonBlock.hxx>
#include <BOPDS_CoupleOfPaveBlocks.hxx>
#include <BOPDS_Curve.hxx>
#include <BOPDS_DataMapOfPaveBlockListOfInteger.hxx>
#include <BOPDS_DS.hxx>
#include <BOPDS_Interf.hxx>
#include <BOPDS_Iterator.hxx>
#include <BOPDS_MapOfPaveBlock.hxx>
#include <BOPDS_Pave.hxx>
#include <BOPDS_PaveBlock.hxx>
#include <BOPTools_AlgoTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepBndLib.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <gp_Pnt.hxx>
#include <IntTools_CommonPrt.hxx>
#include <IntTools_Context.hxx>
#include <IntTools_EdgeFace.hxx>
#include <IntTools_Range.hxx>
#include <IntTools_SequenceOfCommonPrts.hxx>
#include <IntTools_Tools.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Vertex.hxx>

//=======================================================================
//class    : BOPAlgo_EdgeFace
//purpose  : 
//=======================================================================
class BOPAlgo_EdgeFace : 
  public IntTools_EdgeFace,
  public BOPAlgo_Algo {
 
 public:
  DEFINE_STANDARD_ALLOC
  
  BOPAlgo_EdgeFace() : 
    IntTools_EdgeFace(), 
    BOPAlgo_Algo(),
    myIE(-1), myIF(-1) {
  };
  //
  virtual ~BOPAlgo_EdgeFace(){
  };
  //
  void SetIndices(const Standard_Integer nE,
                  const Standard_Integer nF) {
    myIE=nE;
    myIF=nF;
  }
  //
  void Indices(Standard_Integer& nE,
               Standard_Integer& nF) {
    nE=myIE;
    nF=myIF;
  }
  //
  void SetNewSR(const IntTools_Range& aR){
    myNewSR=aR;
  }
  //
  IntTools_Range& NewSR(){
    return myNewSR;
  }
  //
  void SetPaveBlock(const Handle(BOPDS_PaveBlock)& aPB) {
    myPB=aPB;
  }
  //
  Handle(BOPDS_PaveBlock)& PaveBlock() {
    return myPB;
  }
  //
  virtual void Perform() {
    BOPAlgo_Algo::UserBreak();
    IntTools_EdgeFace::Perform();
  }
  //
 protected:
  Standard_Integer myIE;
  Standard_Integer myIF;
  IntTools_Range myNewSR;
  Handle(BOPDS_PaveBlock) myPB;
};
//
//=======================================================================
typedef BOPCol_NCVector<BOPAlgo_EdgeFace> BOPAlgo_VectorOfEdgeFace; 
//
typedef BOPCol_ContextFunctor 
  <BOPAlgo_EdgeFace,
  BOPAlgo_VectorOfEdgeFace,
  Handle(IntTools_Context), 
  IntTools_Context> BOPAlgo_EdgeFaceFunctor;
//
typedef BOPCol_ContextCnt 
  <BOPAlgo_EdgeFaceFunctor,
  BOPAlgo_VectorOfEdgeFace,
  Handle(IntTools_Context)> BOPAlgo_EdgeFaceCnt;
//
//=======================================================================
//function : PerformEF
//purpose  : 
//=======================================================================
void BOPAlgo_PaveFiller::PerformEF()
{
  Standard_Integer iSize;
  //
  myErrorStatus=0;
  //
  FillShrunkData(TopAbs_EDGE, TopAbs_FACE);
  //
  myIterator->Initialize(TopAbs_EDGE, TopAbs_FACE);
  iSize=myIterator->ExpectedLength();
  if (!iSize) {
    return; 
  }
  //
  Standard_Boolean bJustAdd, bV[2];
  Standard_Boolean bV1, bV2, bExpressCompute;
  Standard_Integer nV1, nV2;
  Standard_Integer nE, nF, aDiscretize, i, aNbCPrts, iX, nV[2];
  Standard_Integer aNbEdgeFace, k;
  Standard_Real aTolE, aTolF, aTS1, aTS2, aT1, aT2, aDeflection;
  Handle(NCollection_BaseAllocator) aAllocator;
  TopAbs_ShapeEnum aType;
  BOPDS_ListIteratorOfListOfPaveBlock aIt;
  BOPAlgo_VectorOfEdgeFace aVEdgeFace; 
  //-----------------------------------------------------scope f
  //
  aAllocator=NCollection_BaseAllocator::CommonBaseAllocator();
  //
  BOPCol_MapOfInteger aMIEFC(100, aAllocator);
  BOPDS_IndexedDataMapOfShapeCoupleOfPaveBlocks aMVCPB(100, aAllocator);
  BOPDS_IndexedDataMapOfPaveBlockListOfInteger aMPBLI(100, aAllocator);
  //
  aDiscretize=35;
  aDeflection=0.01;
  //
  BOPDS_VectorOfInterfEF& aEFs=myDS->InterfEF();
  aEFs.SetIncrement(iSize);
  //
  for (; myIterator->More(); myIterator->Next()) {
    myIterator->Value(nE, nF, bJustAdd);
    if(bJustAdd) {
      continue;
    }
    //
    const BOPDS_ShapeInfo& aSIE=myDS->ShapeInfo(nE);
    if (aSIE.HasFlag()){//degenerated 
      continue;
    }
    //
    const TopoDS_Edge& aE=(*(TopoDS_Edge *)(&aSIE.Shape()));
    const TopoDS_Face& aF=(*(TopoDS_Face *)(&myDS->Shape(nF)));
    const Bnd_Box& aBBF=myDS->ShapeInfo(nF).Box(); 
    //
    BOPDS_FaceInfo& aFI=myDS->ChangeFaceInfo(nF);
    const BOPDS_IndexedMapOfPaveBlock& aMPBF=aFI.PaveBlocksOn();
    //
    const BOPCol_MapOfInteger& aMVIn=aFI.VerticesIn();
    const BOPCol_MapOfInteger& aMVOn=aFI.VerticesOn();
    //
    aTolE=BRep_Tool::Tolerance(aE);
    aTolF=BRep_Tool::Tolerance(aF);
    //
    BOPDS_ListOfPaveBlock& aLPB=myDS->ChangePaveBlocks(nE);
    aIt.Initialize(aLPB);
    for (; aIt.More(); aIt.Next()) {
      Handle(BOPDS_PaveBlock)& aPB=aIt.ChangeValue();
      //
      const Handle(BOPDS_PaveBlock) aPBR=myDS->RealPaveBlock(aPB);
      if (aMPBF.Contains(aPBR)) {
        continue;
      }
      //
      if (!aPB->HasShrunkData()) {
        continue;
      }
      //
      Bnd_Box aBBE;
      aPB->ShrunkData(aTS1, aTS2, aBBE);
      //
      if (aBBF.IsOut (aBBE)) {
        continue;
      }
      //
      aPBR->Indices(nV1, nV2);
      bV1=aMVIn.Contains(nV1) || aMVOn.Contains(nV1);
      bV2=aMVIn.Contains(nV2) || aMVOn.Contains(nV2);
      bExpressCompute=bV1 && bV2;
      //
      BOPAlgo_EdgeFace& aEdgeFace=aVEdgeFace.Append1();
      //
      aEdgeFace.SetIndices(nE, nF);
      aEdgeFace.SetPaveBlock(aPB);
      //
      aEdgeFace.SetEdge (aE);
      aEdgeFace.SetFace (aF);
      aEdgeFace.SetTolE (aTolE);
      aEdgeFace.SetTolF (aTolF);
      aEdgeFace.SetDiscretize (aDiscretize);
      aEdgeFace.SetDeflection (aDeflection);
      aEdgeFace.UseQuickCoincidenceCheck(bExpressCompute);
      //
      IntTools_Range aSR(aTS1, aTS2);
      IntTools_Range anewSR=aSR;
      BOPTools_AlgoTools::CorrectRange(aE, aF, aSR, anewSR);
      aEdgeFace.SetNewSR(anewSR);
      //
      aPB->Range(aT1, aT2);
      IntTools_Range aPBRange(aT1, aT2);
      aSR = aPBRange;
      BOPTools_AlgoTools::CorrectRange(aE, aF, aSR, aPBRange);
      aEdgeFace.SetRange (aPBRange);
      aEdgeFace.SetProgressIndicator(myProgressIndicator);
      //
    }//for (; aIt.More(); aIt.Next()) {
  }//for (; myIterator->More(); myIterator->Next()) {
  //
  aNbEdgeFace=aVEdgeFace.Extent();
  //=================================================================
  BOPAlgo_EdgeFaceCnt::Perform(myRunParallel, aVEdgeFace, myContext);
  //=================================================================
  //
  for (k=0; k < aNbEdgeFace; ++k) {
    BOPAlgo_EdgeFace& aEdgeFace=aVEdgeFace(k);
    if (!aEdgeFace.IsDone()) {
      continue;
    }
    //~~~
    aEdgeFace.Indices(nE, nF);
    //
    const TopoDS_Edge& aE=aEdgeFace.Edge();
    const TopoDS_Face& aF=aEdgeFace.Face();
    //
    aTolE=aEdgeFace.TolE();
    aTolF=aEdgeFace.TolF();
    const IntTools_Range& anewSR=aEdgeFace.NewSR();
    Handle(BOPDS_PaveBlock)& aPB=aEdgeFace.PaveBlock();
    //
    aPB->Range(aT1, aT2);
    aPB->Indices(nV[0], nV[1]);
    //
    BOPDS_FaceInfo& aFI=myDS->ChangeFaceInfo(nF);
    const BOPCol_MapOfInteger& aMIFOn=aFI.VerticesOn();
    const BOPCol_MapOfInteger& aMIFIn=aFI.VerticesIn();
    //~~~
    const IntTools_SequenceOfCommonPrts& aCPrts=aEdgeFace.CommonParts();
    aNbCPrts = aCPrts.Length();
    //
    Standard_Boolean bLinePlane = Standard_False;
    if (aNbCPrts) {
      BRepAdaptor_Curve aBAC(aE);
      BRepAdaptor_Surface aBAS(aF, Standard_False);
      //
      bLinePlane = (aBAC.GetType() == GeomAbs_Line &&
                    aBAS.GetType() == GeomAbs_Plane);
    }

    for (i=1; i<=aNbCPrts; ++i) {
      const IntTools_CommonPrt& aCPart=aCPrts(i);
      aType=aCPart.Type();
      switch (aType) {
        case TopAbs_VERTEX:  {
          Standard_Boolean bIsOnPave[2];
          Standard_Integer j;
          Standard_Real aT, aTolToDecide; 
          TopoDS_Vertex aVnew;
          //
          IntTools_Tools::VertexParameter(aCPart, aT);
          BOPTools_AlgoTools::MakeNewVertex(aE, aT, aF, aVnew);
          //
          const IntTools_Range& aR=aCPart.Range1();
          aTolToDecide=5.e-8;
          //
          IntTools_Range aR1(aT1,anewSR.First()),aR2(anewSR.Last(), aT2);
          //
          bIsOnPave[0]=IntTools_Tools::IsInRange(aR1, aR, aTolToDecide); 
          bIsOnPave[1]=IntTools_Tools::IsInRange(aR2, aR, aTolToDecide); 
          //
          if ((bIsOnPave[0] && bIsOnPave[1]) || 
              (bLinePlane && (bIsOnPave[0] || bIsOnPave[1]))) {
            bV[0]=CheckFacePaves(nV[0], aMIFOn, aMIFIn);
            bV[1]=CheckFacePaves(nV[1], aMIFOn, aMIFIn);
            if (bV[0] && bV[1]) {
              IntTools_CommonPrt aCP = aCPart;
              aCP.SetType(TopAbs_EDGE);
              BOPDS_InterfEF& aEF=aEFs.Append1();
              iX=aEFs.Extent()-1;
              aEF.SetIndices(nE, nF);
              aEF.SetCommonPart(aCP);
              myDS->AddInterf(nE, nF);
              //
              aMIEFC.Add(nF);
              //           
              BOPAlgo_Tools::FillMap(aPB, nF, aMPBLI, aAllocator);
              break;
            }
          }
          for (j=0; j<2; ++j) {
            if (bIsOnPave[j]) {
              bV[j]=CheckFacePaves(nV[j], aMIFOn, aMIFIn);
              if (bV[j]) {
                const TopoDS_Vertex& aV=
                  (*(TopoDS_Vertex *)(&myDS->Shape(nV[j])));
                //
                Standard_Real f, l, aTolVnew, aDistPP, aTolPC, aTolV;
                //
                const Handle(Geom_Curve)& aCur = BRep_Tool::Curve(aE, f, l);
                //
                gp_Pnt aP1 = BRep_Tool::Pnt(aV);
                gp_Pnt aP2 = aCur->Value(aT);
                //
                
                aDistPP=aP1.Distance(aP2);
                
                aTolPC=Precision::PConfusion();
                aTolV=BRep_Tool::Tolerance(aV);
                if (aDistPP > (aTolV+aTolPC)) {
                  aTolVnew=Max(aTolE, aDistPP);
                  UpdateVertex(nV[j], aTolVnew);
              }
              }
              else {
                bIsOnPave[j] = ForceInterfVF(nV[j], nF);
              }
            }
          }
          //
          if (!bIsOnPave[0] && !bIsOnPave[1]) {
            if (CheckFacePaves(aVnew, aMIFOn)) {
              continue;
            }
            //
            Standard_Real aTolVnew = BRep_Tool::Tolerance(aVnew);
            aTolVnew = Max(aTolVnew, Max(aTolE, aTolF));
            BRep_Builder().UpdateVertex(aVnew, aTolVnew);
            if (bLinePlane) {
              // increase tolerance for Line/Plane intersection, but do not update 
              // the vertex till its intersection with some other shape
              IntTools_Range aCR = aCPart.Range1();
              aTolVnew = Max(aTolVnew, (aCR.Last() - aCR.First()) / 2.);
            }
            //
            const gp_Pnt& aPnew = BRep_Tool::Pnt(aVnew);
            //
            if (!myContext->IsPointInFace(aPnew, aF, aTolVnew)) {
              continue;
            }
            //
            aMIEFC.Add(nF);
            // 1
            BOPDS_InterfEF& aEF=aEFs.Append1();
            iX=aEFs.Extent()-1;
            aEF.SetIndices(nE, nF);
            aEF.SetCommonPart(aCPart);
            // 2
            myDS->AddInterf(nE, nF);
            // 3
            BOPDS_CoupleOfPaveBlocks aCPB;
            //
            aCPB.SetPaveBlocks(aPB, aPB);
            aCPB.SetIndexInterf(iX);
            aCPB.SetTolerance(aTolVnew);
            aMVCPB.Add(aVnew, aCPB);
          }
        }
          break;
        case TopAbs_EDGE:  {
          aMIEFC.Add(nF);
          //
          // 1
          BOPDS_InterfEF& aEF=aEFs.Append1();
          iX=aEFs.Extent()-1;
          aEF.SetIndices(nE, nF);
          //
          bV[0]=CheckFacePaves(nV[0], aMIFOn, aMIFIn);
          bV[1]=CheckFacePaves(nV[1], aMIFOn, aMIFIn);
          if (!bV[0] || !bV[1]) {
            myDS->AddInterf(nE, nF);
            break;
          }
          aEF.SetCommonPart(aCPart);
          // 2
          myDS->AddInterf(nE, nF);
          // 3
          BOPAlgo_Tools::FillMap(aPB, nF, aMPBLI, aAllocator);
          
        }
          break; 
        default:
          break; 
      }//switch (aType) {
    }//for (i=1; i<=aNbCPrts; ++i) {
  }// for (k=0; k < aNbEdgeEdge; ++k) {
  // 
  //=========================================
  // post treatment
  //=========================================
  BOPAlgo_Tools::PerformCommonBlocks(aMPBLI, aAllocator, myDS);
  PerformVerticesEF(aMVCPB, aAllocator);
  //
  // Update FaceInfoIn for all faces having EF common parts
  BOPCol_MapIteratorOfMapOfInteger aItMI;
  aItMI.Initialize(aMIEFC);
  for (; aItMI.More(); aItMI.Next()) {
    nF=aItMI.Value();
    myDS->UpdateFaceInfoIn(nF);
  }
  // Refine FaceInfoOn to remove all formal pave blocks 
  // made during EF processing 
  //myDS->RefineFaceInfoOn();
  //-----------------------------------------------------scope t
  aMIEFC.Clear();
  aMVCPB.Clear();
  aMPBLI.Clear();
  ////aAllocator.Nullify();
}
//=======================================================================
//function : PerformVerticesEF
//purpose  : 
//=======================================================================
Standard_Integer BOPAlgo_PaveFiller::PerformVerticesEF
  (BOPDS_IndexedDataMapOfShapeCoupleOfPaveBlocks& theMVCPB,
   const Handle(NCollection_BaseAllocator)& theAllocator)
{
  Standard_Integer aNbV, iRet;
  //
  iRet=0;
  aNbV=theMVCPB.Extent();
  if (!aNbV) {
    return iRet;
  }
  //
  Standard_Integer nVx, nVSD, iV, iErr, nE, iFlag, iX, i, aNbPBLI;
  Standard_Real aT, dummy;
  BOPCol_ListIteratorOfListOfShape aItLS;
  BOPCol_ListIteratorOfListOfInteger aItLI;
  BOPDS_PDS aPDS;
  BOPDS_ShapeInfo aSI;
  BOPDS_Pave aPave;
  //
  BOPCol_ListOfShape aLS(theAllocator);
  BOPCol_DataMapOfShapeInteger aMVI(100, theAllocator);
  BOPDS_IndexedDataMapOfPaveBlockListOfInteger aMPBLI(100, theAllocator);
  BOPAlgo_PaveFiller aPF(theAllocator); 
  BOPCol_DataMapOfShapeReal aMVIniTol;
  //
  aSI.SetShapeType(TopAbs_VERTEX);
  BOPDS_VectorOfInterfEF& aEFs=myDS->InterfEF();
  //
  // 1 prepare arguments
  for (i=1; i<=aNbV; ++i) {
    const TopoDS_Vertex& aV = TopoDS::Vertex(theMVCPB.FindKey(i));
    aLS.Append(aV);
    // if an enlarged tolerance is associated with the vertex then update it 
    // remembering its initial tolerance
    Standard_Real aTolEnlarged = theMVCPB.FindFromIndex(i).Tolerance();
    Standard_Real aIniTol = BRep_Tool::Tolerance(aV);
    if (aTolEnlarged > aIniTol) {
      aMVIniTol.Bind(aV, aIniTol);
      BRep_Builder().UpdateVertex(aV, aTolEnlarged);
    }
  }
  //
  // 2 Fuse vertices
  aPF.SetIsPrimary(Standard_False);
  aPF.SetNonDestructive(myNonDestructive);
  aPF.SetArguments(aLS);
  aPF.Perform();
  iErr=aPF.ErrorStatus();
  if (iErr) {
    iRet=1;
    return iRet;
  }
  aPDS=aPF.PDS();
  //
  // Recompute common vertex for each SD group containing enlarged vertex;
  // for that first fill in the map of SD vertex -> its counterparts
  BOPCol_IndexedDataMapOfShapeListOfShape aImages;
  aItLS.Initialize(aLS);
  for (; aItLS.More(); aItLS.Next()) {
    const TopoDS_Shape& aVx = aItLS.Value();
    nVx = aPDS->Index(aVx);
    //
    const TopoDS_Shape& aV = (aPDS->HasShapeSD(nVx, nVSD) ? aPDS->Shape(nVSD) : aVx);
    BOPCol_ListOfShape* pLst = aImages.ChangeSeek(aV);
    if (!pLst) {
      pLst = &aImages.ChangeFromIndex(aImages.Add(aV, BOPCol_ListOfShape()));
    }
    pLst->Append(aVx);
  }
  // 3 Add new vertices to theDS; 
  for (i = 1; i <= aImages.Extent(); i++) {
    TopoDS_Vertex aV = TopoDS::Vertex(aImages.FindKey(i));
    const BOPCol_ListOfShape& aLVSD = aImages.FindFromIndex(i);
    Standard_Boolean isReset = Standard_False;
    BOPCol_ListIteratorOfListOfShape it(aLVSD);
    for (; it.More(); it.Next()) {
      const TopoDS_Vertex& aVx = TopoDS::Vertex(it.Value());
      const Standard_Real* pTolIni = aMVIniTol.Seek(aVx);
      if (pTolIni) {
        // reset enlarged vertex tolerance to the initial value
        reinterpret_cast<BRep_TVertex*>(aVx.TShape().operator->())->Tolerance(*pTolIni);
        isReset = Standard_True;
      }
    }
    TopoDS_Vertex aVnew = aV;
    if (isReset && aLVSD.Extent() > 1) {
      // make new vertex again
      BOPTools_AlgoTools::MakeVertex(aLVSD, aVnew);
    }
    // index of new vertex in theDS -> iV
    aSI.SetShape(aVnew);
    iV = myDS->Append(aSI);
    //
    BOPDS_ShapeInfo& aSIDS = myDS->ChangeShapeInfo(iV);
    Bnd_Box& aBox = aSIDS.ChangeBox();
    BRepBndLib::Add(aVnew, aBox);
    aBox.SetGap(aBox.GetGap() + Precision::Confusion());
    //
    aMVI.Bind(aV, iV);
  }
  //
  // 4 Map PaveBlock/ListOfVertices to add to this PaveBlock ->aMPBLI
  aItLS.Initialize(aLS);
  for (; aItLS.More(); aItLS.Next()) {
    const TopoDS_Shape& aVx=aItLS.Value();
    nVx=aPDS->Index(aVx);
    //
    const TopoDS_Shape& aV = (aPDS->HasShapeSD(nVx, nVSD) ? aPDS->Shape(nVSD) : aVx);
    iV = aMVI.Find(aV);
    //
    BOPDS_CoupleOfPaveBlocks &aCPB=theMVCPB.ChangeFromKey(aVx);
    aCPB.SetIndex(iV);
    // update EF interference
    iX=aCPB.IndexInterf();
    BOPDS_InterfEF& aEF=aEFs(iX);
    aEF.SetIndexNew(iV);
    // map aMPBLI
    const Handle(BOPDS_PaveBlock)& aPB=aCPB.PaveBlock1();
    if (aMPBLI.Contains(aPB)) {
      BOPCol_ListOfInteger& aLI=aMPBLI.ChangeFromKey(aPB);
      aLI.Append(iV);
    }
    else {
      BOPCol_ListOfInteger aLI(theAllocator);
      aLI.Append(iV);
      aMPBLI.Add(aPB, aLI);
    }
  }
  //
  // 5 
  // 5.1  Compute Extra Paves and 
  // 5.2. Add Extra Paves to the PaveBlocks
  aNbPBLI=aMPBLI.Extent();
  for (i=1; i<=aNbPBLI; ++i) {
    Handle(BOPDS_PaveBlock) aPB=aMPBLI.FindKey(i);
    const BOPCol_ListOfInteger& aLI=aMPBLI.FindFromIndex(i);
    nE=aPB->OriginalEdge();
    const TopoDS_Edge& aE=(*(TopoDS_Edge *)(&myDS->Shape(nE)));
    // 
    aItLI.Initialize(aLI);
    for (; aItLI.More(); aItLI.Next()) {
      nVx=aItLI.Value();
      const TopoDS_Vertex& aVx=(*(TopoDS_Vertex *)(&myDS->Shape(nVx)));
      //
      iFlag=myContext->ComputeVE (aVx, aE, aT, dummy);
      if (!iFlag) {
        aPave.SetIndex(nVx);
        aPave.SetParameter(aT);
        aPB->AppendExtPave(aPave);
      }
    }
  }
  // 6  Split PaveBlocks
  for (i=1; i<=aNbPBLI; ++i) {
    Handle(BOPDS_PaveBlock) aPB=aMPBLI.FindKey(i);
    nE=aPB->OriginalEdge();
    // 3
    if (!myDS->IsCommonBlock(aPB)) {
      myDS->UpdatePaveBlock(aPB);
    }
    else {
      const Handle(BOPDS_CommonBlock)& aCB=myDS->CommonBlock(aPB);
      myDS->UpdateCommonBlock(aCB);
    }    
  }//for (; aItMPBLI.More(); aItMPBLI.Next()) {
  // 
  return iRet;
}
//=======================================================================
// function: CheckFacePaves
// purpose: 
//=======================================================================
Standard_Boolean BOPAlgo_PaveFiller::CheckFacePaves 
  (const Standard_Integer nVx,
   const BOPCol_MapOfInteger& aMIFOn,
   const BOPCol_MapOfInteger& aMIFIn)
{
  Standard_Boolean bRet;
  Standard_Integer nV;
  BOPCol_MapIteratorOfMapOfInteger aIt;
  //
  bRet=Standard_False;
  //
  aIt.Initialize(aMIFOn);
  for (; aIt.More(); aIt.Next()) {
    nV=aIt.Value();
    if (nV==nVx) {
      bRet=!bRet;
      return bRet;
    }
  }
  aIt.Initialize(aMIFIn);
  for (; aIt.More(); aIt.Next()) {
    nV=aIt.Value();
    if (nV==nVx) {
      bRet=!bRet;
      return bRet;
    }
  }
  //
  return bRet;
}
//=======================================================================
// function: CheckFacePaves
// purpose: 
//=======================================================================
Standard_Boolean BOPAlgo_PaveFiller::CheckFacePaves 
  (const TopoDS_Vertex& aVnew,
   const BOPCol_MapOfInteger& aMIF)
{
  Standard_Boolean bRet;
  Standard_Integer nV, iFlag;
  BOPCol_MapIteratorOfMapOfInteger aIt;
  //
  bRet=Standard_True;
  //
  aIt.Initialize(aMIF);
  for (; aIt.More(); aIt.Next()) {
    nV=aIt.Value();
    const TopoDS_Vertex& aV=(*(TopoDS_Vertex *)(&myDS->Shape(nV)));
    iFlag=BOPTools_AlgoTools::ComputeVV(aVnew, aV);
    if (!iFlag) {
      return bRet;
    }
  }
  //
  return !bRet;
}
//=======================================================================
//function : ForceInterfVF
//purpose  : 
//=======================================================================
Standard_Boolean BOPAlgo_PaveFiller::ForceInterfVF
  (const Standard_Integer nV, 
   const Standard_Integer nF)
{
  Standard_Boolean bRet;
  Standard_Integer iFlag, nVx;
  Standard_Real U, V, aTolVNew;
  //
  bRet = Standard_False;
  const TopoDS_Vertex& aV = *(TopoDS_Vertex*)&myDS->Shape(nV);
  const TopoDS_Face&   aF = *(TopoDS_Face*)  &myDS->Shape(nF);
  //
  iFlag = myContext->ComputeVF(aV, aF, U, V, aTolVNew);
  if (iFlag == 0 || iFlag == -2) {
    bRet=!bRet;
  //
    BOPDS_VectorOfInterfVF& aVFs=myDS->InterfVF();
    aVFs.SetIncrement(10);
    // 1
    BOPDS_InterfVF& aVF=aVFs.Append1();
    //
    aVF.SetIndices(nV, nF);
    aVF.SetUV(U, V);
    // 2
    myDS->AddInterf(nV, nF);
    //
    // 3 update vertex V/F if necessary
    nVx=UpdateVertex(nV, aTolVNew);
    // 4
    if (myDS->IsNewShape(nVx)) {
      aVF.SetIndexNew(nVx);
    }
    //
    BOPDS_FaceInfo& aFI=myDS->ChangeFaceInfo(nF);
    BOPCol_MapOfInteger& aMVIn=aFI.ChangeVerticesIn();
    aMVIn.Add(nVx);
  }
  return bRet;
}
