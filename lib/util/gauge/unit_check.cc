// $Id: unit_check.cc,v 1.3 2004-01-01 15:32:55 edwards Exp $

/*! \file
 *  \brief Test a gauge field is unitarized
 */

#include "chromabase.h"
#include "util/gauge/reunit.h"
#include "util/gauge/unit_check.h"

using namespace QDP;


//! Check the unitarity of color matrix in SU(N)
/*!
 * \ingroup gauge
 *
 * \param  a  The LatticeColorMatrix to be tested
 */
void unitarityCheck(const multi1d<LatticeColorMatrix>& u)
{
  int numbad;

  for (int mu=0; mu < Nd; ++mu)
  {
    LatticeColorMatrix u_tmp = u[mu];
    reunit(u_tmp, numbad, REUNITARIZE_ERROR);
  }
}


