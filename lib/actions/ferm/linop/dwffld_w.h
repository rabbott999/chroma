// -*- C++ -*-
// $Id: dwffld_w.h,v 1.1 2003-11-09 22:34:06 edwards Exp $
/*! \file
 *  \brief DWF parity/rotation operator
 *
 * Apply a DWF style `parity' or rotation operator to move from 
 * boundary field basis to single field basis or back again 
 */

#ifndef __dwffld_w_h__
#define __dwffld_w_h__

#include "actions/ferm/linop/dwffld_w.h"

using namespace QDP;

//! DWF parity/rotation operator
/*! \ingroup linop
 *
 *  Chi  :=  P^{isign} . Psi    where  P is the rotation operator 
 *
 *  \param psi        Pseudofermion field                     (Read)
 *  \param chi        Pseudofermion field                     (Write)
 *  \param isign      Sign (Plus/Minus)    	              (Read)
 */

void DwfFld(LatticeDWFermion& chi, const LatticeDWFermion& psi, enum PlusMinus isign);

#endif
