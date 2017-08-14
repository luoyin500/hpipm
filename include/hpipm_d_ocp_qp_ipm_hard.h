/**************************************************************************************************
*                                                                                                 *
* This file is part of HPIPM.                                                                     *
*                                                                                                 *
* HPIPM -- High Performance Interior Point Method.                                                *
* Copyright (C) 2017 by Gianluca Frison.                                                          *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* HPIPM is free software; you can redistribute it and/or                                          *
* modify it under the terms of the GNU Lesser General Public                                      *
* License as published by the Free Software Foundation; either                                    *
* version 2.1 of the License, or (at your option) any later version.                              *
*                                                                                                 *
* HPIPM is distributed in the hope that it will be useful,                                        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            *
* See the GNU Lesser General Public License for more details.                                     *
*                                                                                                 *
* You should have received a copy of the GNU Lesser General Public                                *
* License along with HPIPM; if not, write to the Free Software                                    *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/



#include <blasfeo_target.h>
#include <blasfeo_common.h>




struct d_ipm_hard_ocp_qp_workspace
	{
	struct d_ipm_hard_core_qp_workspace *core_workspace;
	struct d_strvec *dux;
	struct d_strvec *dpi;
	struct d_strvec *dt;
	struct d_strvec *res_g; // q-residuals
	struct d_strvec *res_b; // b-residuals
	struct d_strvec *res_d; // d-residuals
	struct d_strvec *res_m; // m-residuals
	struct d_strvec *Gamma; // hessian update
	struct d_strvec *gamma; // hessian update
	struct d_strvec *tmp_nbM; // work space of size nbM
	struct d_strvec *tmp_nxM; // work space of size nxM
	struct d_strvec *tmp_ngM; // work space of size ngM
	struct d_strvec *Pb; // Pb
	struct d_strmat *L;
	struct d_strmat *AL;
	double *stat; // convergence statistics
	double res_mu; // mu-residual
	int iter; // iteration number
	int memsize;
	};



struct d_ipm_hard_ocp_qp_arg
	{
	double alpha_min; // exit cond on step length
	double mu_max; // exit cond on duality measure
	double mu0; // initial value for duality measure
	int iter_max; // exit cond in iter number
	};



//
int d_memsize_ipm_hard_ocp_qp(struct d_ocp_qp *qp, struct d_ipm_hard_ocp_qp_arg *arg);
//
void d_create_ipm_hard_ocp_qp(struct d_ocp_qp *qp, struct d_ipm_hard_ocp_qp_arg *arg, struct d_ipm_hard_ocp_qp_workspace *ws, void *mem);
//
void d_solve_ipm_hard_ocp_qp(struct d_ocp_qp *qp, struct d_ocp_qp_sol *qp_sol, struct d_ipm_hard_ocp_qp_workspace *ws);
//
void d_solve_ipm2_hard_ocp_qp(struct d_ocp_qp *qp, struct d_ocp_qp_sol *qp_sol, struct d_ipm_hard_ocp_qp_workspace *ws);

