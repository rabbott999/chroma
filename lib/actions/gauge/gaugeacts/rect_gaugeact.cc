// $Id: rect_gaugeact.cc,v 3.3 2006-07-21 18:39:11 bjoo Exp $
/*! \file
 *  \brief Rectangle gauge action
 */

#include "chromabase.h"
#include "actions/gauge/gaugeacts/rect_gaugeact.h"
#include "actions/gauge/gaugeacts/gaugeact_factory.h"
#include "actions/gauge/gaugeacts/gauge_createstate_aggregate.h"

namespace Chroma
{



  namespace RectGaugeActEnv 
  {
    //! Callback
    GaugeAction< multi1d<LatticeColorMatrix>, multi1d<LatticeColorMatrix> >* createGaugeAct(XMLReader& xml, 
											    const std::string& path) 
    {
      return new RectGaugeAct(CreateGaugeStateEnv::reader(xml, path), 
			      RectGaugeActParams(xml, path));
    }

    const std::string name = "RECT_GAUGEACT";
    const bool registered = TheGaugeActFactory::Instance().registerObject(name, 
									  createGaugeAct);
  };


  // Param constructor/reader
  RectGaugeActParams::RectGaugeActParams(XMLReader& xml_in, const std::string& path) {
    XMLReader paramtop(xml_in, path);

    try {
      
      // Read AnisoParam if it exists 
      if( paramtop.count("AnisoParam") != 0 ) {
	read(paramtop, "AnisoParam", aniso);
      }
      else { 
	// Set aniso to not be anisotropic if it doesn't
	aniso.anisoP = false;
	aniso.t_dir = Nd -1;
	aniso.xi_0 = 1;
	aniso.nu = 1;
      }


      if ( aniso.anisoP == false ) { 
	
	// If we are not anisotropic look for coeff 
	// and set both coeff_s = coeff_t = coeff, do temporal = true

	read(paramtop, "coeff", coeff_s);
	coeff_t1 = coeff_s;
	coeff_t2 = coeff_s;
	
      }
      else { 

	// If we are ansotropic, look for coeff_s and coeff_t
	read(paramtop, "coeff_s", coeff_s);
	read(paramtop, "coeff_t1", coeff_t1);
	read(paramtop, "coeff_t2", coeff_t2);

      }

      // In either anisotropic or isotropic case we can optionally
      // specify whether we want to omit temporal rectangles
      if( paramtop.count("no_temporal_2link") == 0 ) {
	// Default: always do temporal
	no_temporal_2link = false;
      }
      else { 
	// If it is specified in the file, then read it.
	read(paramtop, "no_temporal_2link", no_temporal_2link);
      }

    }
    catch( const std::string& e ) { 
      QDPIO::cerr << "Error reading XML: " <<  e << endl;
      QDP_abort(1);
    }
  }

  // Read params
  void read(XMLReader& xml, const string& path, RectGaugeActParams& p) 
  {
    RectGaugeActParams tmp(xml, path);
    p=tmp;
  }


  //! Compute staple
  /*!
   * \param u_staple   result      ( Write )
   * \param state      gauge field ( Read )
   * \param mu         direction for staple ( Read )
   * \param cb         subset on which to compute ( Read )
   */
  void
  RectGaugeAct::staple(LatticeColorMatrix& u_staple,
		       const Handle< GaugeState<P,Q> >& state,
		       int mu, int cb) const
  {
    QDPIO::cerr << "RectGaugeAct::staple() - not converted from szin" << endl;
    QDP_abort(1);

    END_CODE();
  }

  inline
  void deriv_part(int mu, int nu, Real c_munu,
		  multi1d<LatticeColorMatrix>& ds_u, 
		  const multi1d<LatticeColorMatrix>& u) {

    //       ---> --->  = U(x,mu)*U(x+mu, mu)
    LatticeColorMatrix from_left_2link = u[mu]*shift(u[mu], FORWARD, mu);


    // upper_l =    ----> --->
    //             ^
    //             |
    //
    // U(x, nu)*U(x+nu,mu)*U(x+mu+nu, mu)
    LatticeColorMatrix upper_l = u[nu]*shift(from_left_2link, FORWARD, nu);


    //       <--- <----
    //      |
    //      |
    //      V ---> ---> (x+2mu)
    //
    // so here we have the staple for x + 2mu
    // we need to shift it forward by 2 mu

    // Here we shift it by mu after assembly

    // shift, backward mu of:
    // U^dag(x+mu+nu, mu)*U^dag(x+nu, mu)*U^dag(x, nu)*U(x,mu)*U(x+mu,mu)
    //=U^dag(x+nu, mu)*U^dag(x-mu+nu, mu)*U^dag(x-mu, nu)*U(x-mu,mu)*U(x,mu)
    LatticeColorMatrix tmp = shift( adj(upper_l)*from_left_2link, BACKWARD, mu);
    

    // and we shift it again
    //       <--- <----
    //      |
    //      |
    //      V ---> ---> (x)
    //=U^dag(x+-mu+nu, mu)*U^dag(x-2mu+nu, mu)*U^dag(x-2mu, nu)*U(x-2mu,mu)*U(x-mu,mu)
    ds_u[nu] = shift(tmp, BACKWARD, mu);

    
    // Tmp = u(x+mu, nu)
    tmp = shift(u[nu], FORWARD, mu);
    
    // tmp2 = u(x + 2mu, nu)
    LatticeColorMatrix tmp2 = shift(tmp, FORWARD, mu);

    // Upper_le = from_left_2link(x+nu) * u^dag(x + 2mu, nu)
    //          = u(x+nu, mu)*u(x+mu+nu, mu)*u^dag(x+2mu, nu)

    //           = ----> ----> |
    //                         | 
    //                         V

    upper_l = shift(from_left_2link, FORWARD, nu)*adj(tmp2);

    // ds_u =  ----> ----> |
    //                     | 
    //        <----  <---  V

   // = u(x+nu, mu)*u(x+mu+nu, mu)*u^dag(x+2mu, nu)*u^dag(x+mu, mu)*u^dag(x,mu)

    ds_u[nu] += upper_l * adj(from_left_2link);

    

    //              | <---- <--- ^ 
    //              |            |
    //              |            |
    //          x   V            | x + 2mu

    // u(x+2mu,nu)*u^dag(x+mu+nu,mu)*u^dag(x+nu, mu)*u^dag(x, nu)
    LatticeColorMatrix up_staple = adj(upper_l)*adj(u[nu]);

    //              ^            |
    //              |            |
    //              |            |
    //          x   <----- <---- V x + 2mu

    //  u^dag(x+2mu,nu) U^dag(x+mu,mu) U^dag(x,mu) U(x,nu)
    tmp = adj(tmp2)*adj(from_left_2link);
    LatticeColorMatrix down_staple = tmp*u[nu];

    // The following terms generate force staple for U(x+mu)
    // the backward shift moves the contribution to the correct place
    //
    //              | <---- <--- ^ 
    //              |            |
    //              |            |
    //          x   V----->      | x + 2mu

    
    // u(x+mu,nu)*u^dag(x+nu,mu)*u^dag(x-mu+nu, mu)*u^dag(x-mu, nu) u(x-mu,mu)
    ds_u[mu] = shift( up_staple*u[mu], BACKWARD, mu);

    //           x   ^---         | x+2mu
    //               |            |
    //               |            |
    //               <----- <---- V 
    
    // shift backward nu shift backward mu of 
    //  u^dag(x+mu-nu,nu) U^dag(x-nu,mu) U^dag(x-mu-nu,mu) U(x-mu-nu,nu) U(x-mu,mu)

    tmp2 = shift(down_staple, BACKWARD, nu);
    ds_u[mu] += shift(tmp2*u[mu], BACKWARD, mu);
    
    //              | <---- <--- ^ 
    //              |            |
    //              |            |
    //          x   V      ----->| x + 2mu
    tmp = shift(u[mu], FORWARD, mu);
    ds_u[mu] += tmp*up_staple;


    //         x    ^      ----->| x + 2mu + nu
    //              |            |
    //              |            |
    //              <----- <---- V


    ds_u[mu] += tmp*tmp2;

    ds_u[mu] *= c_munu/Real(-2*Nc);
    ds_u[nu] *= c_munu/Real(-2*Nc);
  }


  void
  RectGaugeAct::deriv(multi1d<LatticeColorMatrix>& ds_u,
		      const Handle< GaugeState<P,Q> >& state) const
  {
    ds_u.resize(Nd);
    Real c;

    multi1d<LatticeColorMatrix> ds_tmp(Nd);

    const multi1d<LatticeColorMatrix>& u = state->getLinks();
    
    ds_u = zero;
    ds_tmp = zero; 

    for(int mu=0; mu < Nd; mu++) { 
      for(int nu=0; nu < Nd; nu++) { 

	if (mu == nu) continue;

	if ( mu == params.aniso.t_dir ) { 
	  c = params.coeff_t1;
	}
	else if( nu == params.aniso.t_dir ) { 
	  c = params.coeff_t2;
	}
	else { 
	  c = params.coeff_s;
	}

	bool skip = (mu == params.aniso.t_dir && params.no_temporal_2link );
	if (!skip) {
	  deriv_part(mu, nu, c, ds_u, u);

	  ds_tmp[mu] += ds_u[mu];
	  ds_tmp[nu] += ds_u[nu];
	}
      }
    }

    for(int mu = 0; mu < Nd; mu++) { 
      ds_u[mu] = u[mu]*ds_tmp[mu];
    }

    getGaugeBC().zero(ds_u);
    
  }


  // Get the gauge action
  //
  // S = -(coeff/(Nc) Sum Re Tr Rect
  //
  // w_rect is defined in MesPlq as
  //
  // w_rect =( 2/(V*Nd*(Nd-1)*Nc)) * Sum Re Tr Rect
  //
  // so 
  // S = -coeff * (V*Nd*(Nd-1)/2) w_rect 
  //   = -coeff * (V*Nd*(Nd-1)/2)*(2/(V*Nd*(Nd-1)*Nc))* Sum Re Tr Rect
  //   = -coeff * (1/(Nc)) * Sum Re Tr Rect
  inline
  void S_part(int mu, int nu, Real c, LatticeReal& lgimp, 
	    const multi1d<LatticeColorMatrix>& u);
  
  Double
  RectGaugeAct::S(const Handle< GaugeState<P,Q> >& state) const
  {
    START_CODE();
    
    const multi1d<LatticeColorMatrix>& u = state->getLinks();
    LatticeReal lgimp = zero;
    Real c; 
 
    for(int mu=0; mu < Nd; ++mu) { 
      for(int nu = 0; nu < Nd; ++nu) { 
	if( mu == nu) continue;

	if( mu == params.aniso.t_dir ) { 
	  c = params.coeff_t1;    // Param for 2x1 in t
	}
	else if ( nu == params.aniso.t_dir ) { 
	  c = params.coeff_t2; 
	}
	else {
	  c = params.coeff_s;
	}
	


	// if mu (the long direction) is t_dir and we need to skip this
	// then skip = true
	bool skip = ( mu == params.aniso.t_dir && params.no_temporal_2link );
	if( !skip ) {
	  S_part(mu, nu, c, lgimp, u);
	}
      }
    }

    Double S_rect = sum(lgimp);
    S_rect *= -Double(1) / Double(Nc);   // note sign here
    return S_rect;
    
  }
  

void S_part(int mu, int nu, Real c, LatticeReal& lgimp, 
		     const multi1d<LatticeColorMatrix>& u)
{
  LatticeColorMatrix tmp1;
  LatticeColorMatrix tmp2;
  LatticeColorMatrix lr_corner;
  LatticeColorMatrix lower_l;
  LatticeColorMatrix upper_l;
  LatticeColorMatrix rectangle_2munu;
  LatticeColorMatrix rectangle_mu2nu;
  
  
  //                     ^
  // lr_corner =         |   = u(x, mu) * u(x + mu, nu)
  //                     |
  //                ----->
  //
  
  lr_corner = u[mu]*shift(u[nu], FORWARD, mu);
  
  //                        ^
  // lower_l =              |
  //            -----> ----->

  //
  //  lower_l = u(x, mu) u(x + mu, mu) * u(x + 2mu, nu)  OK!!
  lower_l = u[mu]*shift(lr_corner, FORWARD, mu);
  
  
  
  // Upper L =  <----- <-----
  //            |
  //            V
  
  // tmp2 = adj(tmp_1)*adj(u[mu]) = u^dag(x + mu, nu) u^dag(x, mu)
  tmp2 = adj(shift(u[mu], FORWARD, mu))*adj(u[mu]);
  
  // tmp1 = tmp2(x+nu) = u^dag(x + mu + nu, mu) * u^dag(x + nu, mu)
  tmp1 = shift(tmp2, FORWARD, nu);
  
  // upper_l =  u^dag(x + mu + nu, mu) * u^dag(x + nu, mu) * u^dag(nu)
  upper_l = tmp1*adj(u[nu]);
  
  //   Loop = Lower_l * upper_l
  //        =  u(x, mu) u(x + mu, mu) * u(x + 2mu, nu)
  //         * u^dag(x + mu + nu, mu) * u^dag(x + nu, mu) * u^dag(nu)
  rectangle_2munu = lower_l*upper_l;
  
  
  lgimp += c * real(trace(rectangle_2munu));
  // lgimp += c2 * real(trace(rectangle_mu2nu));
}



// Isotropic case
RectGaugeAct::RectGaugeAct(Handle< CreateGaugeState<P,Q> > cgs_,
			     const Real& coeff_s_) : cgs(cgs_)
{
  // Setup the params structure
  params.coeff_s = params.coeff_t1 = params.coeff_t2 = coeff_s_;
  params.no_temporal_2link = false;
  params.aniso.anisoP = false;
  params.aniso.t_dir = Nd -1;
  params.aniso.xi_0 = 1;
  params.aniso.nu = 1;    
}

// Anisotropic case
RectGaugeAct::RectGaugeAct(Handle< CreateGaugeState<P,Q> > cgs_,
			   const Real& coeff_s_,
			   const Real& coeff_t1_,
			   const Real& coeff_t2_,
			   const bool no_temporal_2link_,
			   const AnisoParam_t& aniso_) : cgs(cgs_) 
{
  // Setup the params structure
  params.coeff_s = coeff_s_ / aniso_.xi_0 ;
  params.coeff_t1 = coeff_t1_ * aniso_.xi_0 ;
  params.coeff_t2 = coeff_t2_ * aniso_.xi_0;
  params.no_temporal_2link = no_temporal_2link_ ;
  params.aniso = aniso_ ;          // Rely on struct copy constructor here

}

//! Read rectangle coefficient from a param struct
RectGaugeAct::RectGaugeAct(Handle< CreateGaugeState<P,Q> > cgs_, 
			   const RectGaugeActParams& p) : cgs(cgs_), params(p) {
  // It is at this point we take into account the anisotropy
  if( params.aniso.anisoP ) { 
    params.coeff_s = p.coeff_s / p.aniso.xi_0;
    params.coeff_t1 = p.coeff_t1 * p.aniso.xi_0;
    params.coeff_t2 = p.coeff_t2 * p.aniso.xi_0;
  }
}

}

