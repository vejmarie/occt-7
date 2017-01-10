// Created on: 2004-06-29
// Created by: Eugeny NAPALKOV 
// Copyright (c) 2004-2014 OPEN CASCADE SAS
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

#ifndef _AppStdL_Application_HeaderFile
#define _AppStdL_Application_HeaderFile

#include <Standard.hxx>
#include <Standard_Type.hxx>

#include <TDocStd_Application.hxx>
#include <TColStd_SequenceOfExtendedString.hxx>
#include <Standard_CString.hxx>
class CDM_MessageDriver;


class AppStdL_Application;
DEFINE_STANDARD_HANDLE(AppStdL_Application, TDocStd_Application)


class AppStdL_Application : public TDocStd_Application
{

public:

  
  Standard_EXPORT AppStdL_Application();
  
  Standard_EXPORT virtual Handle(CDM_MessageDriver) MessageDriver() Standard_OVERRIDE;
  
  //! returns supported format for application documents.
  Standard_EXPORT virtual void Formats (TColStd_SequenceOfExtendedString& theFormats) Standard_OVERRIDE;
  
  //! returns   the file  name  which  contains  application
  //! resources
  Standard_EXPORT Standard_CString ResourcesName() Standard_OVERRIDE;




  DEFINE_STANDARD_RTTIEXT(AppStdL_Application,TDocStd_Application)

protected:




private:


  Handle(CDM_MessageDriver) myMessageDriver;


};







#endif // _AppStdL_Application_HeaderFile
