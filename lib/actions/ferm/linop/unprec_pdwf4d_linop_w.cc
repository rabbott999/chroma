// $Id: unprec_pdwf4d_linop_w.cc,v 1.1 2004-11-17 16:53:23 edwards Exp $
/*! \file
 *  \brief Unpreconditioned projected DWF operator to 4D using prec 5D bits
 */

#include "chromabase.h"
#include "actions/ferm/invert/invcg2_array.h"
#include "actions/ferm/linop/unprec_pdwf4d_linop_w.h"
#include "actions/ferm/linop/dwffld_w.h"

using namespace QDP;
using namespace Chroma;

namespace Chroma
{

  // Apply unpreconditioned linear operator
  template<>
  void 
  UnprecPDWF4DLinOp<LatticeFermion>::operator()(LatticeFermion& chi, 
						const LatticeFermion& psi, 
						enum PlusMinus isign) const
  {
    START_CODE();

    const int  N5 = size();   // array size better match
    int n_count;
  
    // Initialize the 5D fields
    multi1d<LatticeFermion> psi5(N5);
    //  psi5 = (psi,0,0,0,..,0)^T
    psi5 = zero;
    psi5[0] = psi;

    // tmp5 = P . psi5
    multi1d<LatticeFermion> tmp5(N5);
    DwfFld(tmp5, psi5, PLUS);

    multi1d<LatticeFermion> chi5(N5);

    switch(isign)
      {
      case PLUS:
	{
	  // chi5 = D5(m_q) . tmp5 =  D5(m_q) . P . (chi,0,0,..,0)^T 
	  D->unprecLinOp(chi5, tmp5, PLUS);

	  // Solve  D5(1) . psi5 = chi5
	  psi5 = chi5;
  
#if defined(HACK_USE_PREC)
	  /* Step (i) */
	  /* chi_tmp_o =  chi_o - D_oe * A_ee^-1 * chi_e */
	  multi1d<LatticeFermion> chi_tmp(N5);
	  {
	    multi1d<LatticeFermion> tmp1(N5);
	    multi1d<LatticeFermion> tmp2(N5);

	    PV->evenEvenInvLinOp(tmp1, chi5, PLUS);
	    PV->oddEvenLinOp(tmp2, tmp1, PLUS);
	    for(int n=0; n < N5; ++n)
	      chi_tmp[n][rb[1]] = chi5[n] - tmp2[n];
	  }
#else
	  multi1d<LatticeFermion> chi_tmp(N5);
	  chi_tmp = chi5;
#endif

	  switch(invParam.invType)
	    {
	    case CG_INVERTER: 
	      {
		/* tmp5 = M_dag(u) * chi_tmp */
		(*PV)(tmp5, chi_tmp, MINUS);

		/* psi5 = (M^dag * M)^(-1) chi_tmp */
		InvCG2 (*PV, tmp5, psi5, invParam.RsdCG, invParam.MaxCG, n_count);
	      }
	      break;
  
	    default:
	      QDP_error_exit("Unknown inverter type", invParam.invType);
	    }

#if defined(HACK_USE_PREC)
	  /* Step (ii) */
	  /* psi_e = A_ee^-1 * [chi_e  -  D_eo * psi_o] */
	  {
	    multi1d<LatticeFermion> tmp1(N5);
	    multi1d<LatticeFermion> tmp2(N5);

	    PV->evenOddLinOp(tmp1, psi5, PLUS);
	    for(int n=0; n < N5; ++n)
	      tmp2[n][rb[0]] = chi5[n] - tmp1[n];
	    PV->evenEvenInvLinOp(psi5, tmp2, PLUS);
	  }
#endif
  	}
	break;

      case MINUS:
	{
	  QDP_error_exit("MINUS not supported at the moment");
	}
	break;
      }
    
    if ( n_count == invParam.MaxCG )
      QDP_error_exit("no convergence in the inverter", n_count);
  
    // Project out first slice after  chi <- chi5 <- P^(-1) . psi5
    DwfFld(chi5, psi5, MINUS);
    chi = chi5[0];

    END_CODE();
  }

}
