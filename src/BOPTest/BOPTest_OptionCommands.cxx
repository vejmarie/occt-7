// Created by: Peter KURNEV
// Copyright (c) 2014 OPEN CASCADE SAS
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


#include <BOPTest.hxx>
#include <BOPTest_Objects.hxx>
#include <DBRep.hxx>
#include <Draw.hxx>

#include <stdio.h>
#include <string.h>
static Standard_Integer boptions (Draw_Interpretor&, Standard_Integer, const char**); 
static Standard_Integer brunparallel (Draw_Interpretor&, Standard_Integer, const char**); 
static Standard_Integer bnondestructive(Draw_Interpretor&, Standard_Integer, const char**);
static Standard_Integer bfuzzyvalue(Draw_Interpretor&, Standard_Integer, const char**);

//=======================================================================
//function : OptionCommands
//purpose  : 
//=======================================================================
void BOPTest::OptionCommands(Draw_Interpretor& theCommands)
{
  static Standard_Boolean done = Standard_False;
  if (done) return;
  done = Standard_True;
  // Chapter's name
  const char* g = "BOPTest commands";
  // Commands  
  theCommands.Add("boptions", "use boptions, shows current value of BOP options" , __FILE__, boptions, g);
  theCommands.Add("brunparallel", "use brunparallel [0/1]" , __FILE__, brunparallel, g);
  theCommands.Add("bnondestructive", "use bnondestructive [0/1]", __FILE__, bnondestructive, g);
  theCommands.Add("bfuzzyvalue", "use bfuzzyvalue value", __FILE__, bfuzzyvalue, g);
}
//=======================================================================
//function : boptions
//purpose  : 
//=======================================================================
Standard_Integer boptions(Draw_Interpretor& di,
                          Standard_Integer n, 
                          const char** ) 
{ 
  if (n!=1) {
    di << " use boptions\n";
    return 0;
  }
  //
  char buf[128];
  Standard_Boolean bRunParallel, bNonDestructive;
  Standard_Real aFuzzyValue;
  //
  bRunParallel=BOPTest_Objects::RunParallel();
  bNonDestructive = BOPTest_Objects::NonDestructive();
  aFuzzyValue = BOPTest_Objects::FuzzyValue();
  
  Sprintf(buf, " RunParallel: %d\n",  bRunParallel);
  di << buf;
  Sprintf(buf, " NonDestructive: %d\n", bNonDestructive);
  di << buf;
  Sprintf(buf, " FuzzyValue : %lf\n", aFuzzyValue);
  di << buf;
  //
  return 0;
}
//=======================================================================
//function : bfuzzyvalue
//purpose  : 
//=======================================================================
Standard_Integer bfuzzyvalue(Draw_Interpretor& di,
                             Standard_Integer n, 
                             const char** a) 
{ 
  if (n!=2) {
    di << " use bfuzzyvalue value\n";
    return 0;
  }
  //
  Standard_Real aFuzzyValue;
  //
  aFuzzyValue=Draw::Atof(a[1]);
  if (aFuzzyValue<0.) {
    di << " Wrong value.\n"; 
    return 0;  
  }
  //
  BOPTest_Objects::SetFuzzyValue(aFuzzyValue);
  //
  return 0;
}
//=======================================================================
//function : brunparallel
//purpose  : 
//=======================================================================
Standard_Integer brunparallel(Draw_Interpretor& di,
                              Standard_Integer n, 
                              const char** a) 
{ 
  if (n!=2) {
    di << " use brunparallel [0/1]\n";
    return 0;
  }
  //
  Standard_Integer iX;
  Standard_Boolean bRunParallel;
  //
  iX=Draw::Atoi(a[1]);
  if (iX<0 || iX>1) {
    di << " Wrong value.\n"; 
    return 0;  
  }
  //
  bRunParallel=(Standard_Boolean)(iX);
  BOPTest_Objects::SetRunParallel(bRunParallel);
  //
  return 0;
}
//=======================================================================
//function : bnondestructive
//purpose  : 
//=======================================================================
Standard_Integer bnondestructive(Draw_Interpretor& di,
  Standard_Integer n,
  const char** a)
{
  if (n != 2) {
    di << " use bnondestructive [0/1]\n";
    return 0;
  }
  //
  Standard_Integer iX;
  Standard_Boolean bNonDestructive;
  //
  iX = Draw::Atoi(a[1]);
  if (iX<0 || iX>1) {
    di << " Wrong value.\n";
    return 0;
  }
  //
  bNonDestructive = (Standard_Boolean)(iX);
  BOPTest_Objects::SetNonDestructive(bNonDestructive);
  //
  return 0;
}
