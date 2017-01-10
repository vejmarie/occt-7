// Created on: 1993-06-17
// Created by: Jean Yves LEBEY
// Copyright (c) 1993-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#ifndef TopOpeBRepDS_DoubleMapOfIntegerShape_HeaderFile
#define TopOpeBRepDS_DoubleMapOfIntegerShape_HeaderFile

#include <Standard_Integer.hxx>
#include <TopoDS_Shape.hxx>
#include <TColStd_MapIntegerHasher.hxx>
#include <TopTools_ShapeMapHasher.hxx>
#include <NCollection_DoubleMap.hxx>

typedef NCollection_DoubleMap<Standard_Integer,TopoDS_Shape,TColStd_MapIntegerHasher,TopTools_ShapeMapHasher> TopOpeBRepDS_DoubleMapOfIntegerShape;
typedef NCollection_DoubleMap<Standard_Integer,TopoDS_Shape,TColStd_MapIntegerHasher,TopTools_ShapeMapHasher>::Iterator TopOpeBRepDS_DoubleMapIteratorOfDoubleMapOfIntegerShape;


#endif
