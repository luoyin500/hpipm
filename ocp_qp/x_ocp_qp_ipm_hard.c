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



int MEMSIZE_IPM_HARD_OCP_QP(struct OCP_QP *qp, struct IPM_HARD_OCP_QP_ARG *arg)
	{

	// loop index
	int ii;

	// extract ocp qp size
	int N = qp->N;
	int *nx = qp->nx;
	int *nu = qp->nu;
	int *nb = qp->nb;
	int *ng = qp->ng;

	// compute core qp size and max size
	int nvt = 0;
	int net = 0;
	int nct = 0;
	int nxM = 0;
	int nuM = 0;
	int nbM = 0;
	int ngM = 0;
	for(ii=0; ii<N; ii++)
		{
		nvt += nx[ii]+nu[ii];
		net += nx[ii+1];
		nct += nb[ii]+ng[ii];
		nxM = nx[ii]>nxM ? nx[ii] : nxM;
		nuM = nu[ii]>nuM ? nu[ii] : nuM;
		nbM = nb[ii]>nbM ? nb[ii] : nbM;
		ngM = ng[ii]>ngM ? ng[ii] : ngM;
		}
	nvt += nx[ii]+nu[ii];
	nct += nb[ii]+ng[ii];
	nxM = nx[ii]>nxM ? nx[ii] : nxM;
	nuM = nu[ii]>nuM ? nu[ii] : nuM;
	nbM = nb[ii]>nbM ? nb[ii] : nbM;
	ngM = ng[ii]>ngM ? ng[ii] : ngM;

	int size = 0;

	size += 7*(N+1)*sizeof(struct STRVEC); // dux dt res_g res_d res_m Gamma gamma
	size += 3*N*sizeof(struct STRVEC); // dpi res_b Pb
	size += 4*sizeof(struct STRVEC); // tmp_nxM tmp_nbM tmp_ngM

	size += 1*(N+1)*sizeof(struct STRMAT); // L
	size += 2*sizeof(struct STRMAT); // AL

	size += 1*SIZE_STRVEC(nbM); // tmp_nbM
	size += 1*SIZE_STRVEC(nxM); // tmp_nxM
	size += 2*SIZE_STRVEC(nxM); // tmp_ngM
	for(ii=0; ii<N; ii++) size += 1*SIZE_STRVEC(nx[ii+1]); // Pb
	for(ii=0; ii<=N; ii++) size += 1*SIZE_STRMAT(nu[ii]+nx[ii]+1, nu[ii]+nx[ii]); // L
	size += 2*SIZE_STRMAT(nuM+nxM+1, nxM+ngM); // AL

	size += 1*sizeof(struct IPM_HARD_CORE_QP_WORKSPACE);
	size += 1*MEMSIZE_IPM_HARD_CORE_QP(nvt, net, nct, arg->iter_max);

	size = (size+63)/64*64; // make multiple of typical cache line size
	size += 1*64; // align once to typical cache line size

	return size;

	}



void CREATE_IPM_HARD_OCP_QP(struct OCP_QP *qp, struct IPM_HARD_OCP_QP_ARG *arg, struct IPM_HARD_OCP_QP_WORKSPACE *workspace, void *mem)
	{

	// loop index
	int ii;

	// extract ocp qp size
	int N = qp->N;
	int *nx = qp->nx;
	int *nu = qp->nu;
	int *nb = qp->nb;
	int *ng = qp->ng;


	workspace->memsize = MEMSIZE_IPM_HARD_OCP_QP(qp, arg);


	// compute core qp size and max size
	int nvt = 0;
	int net = 0;
	int nct = 0;
	int nxM = 0;
	int nuM = 0;
	int nbM = 0;
	int ngM = 0;

	for(ii=0; ii<N; ii++)
		{
		nvt += nx[ii]+nu[ii];
		net += nx[ii+1];
		nct += nb[ii]+ng[ii];
		nxM = nx[ii]>nxM ? nx[ii] : nxM;
		nuM = nu[ii]>nuM ? nu[ii] : nuM;
		nbM = nb[ii]>nbM ? nb[ii] : nbM;
		ngM = ng[ii]>ngM ? ng[ii] : ngM;
		}
	nvt += nx[ii]+nu[ii];
	nct += nb[ii]+ng[ii];
	nxM = nx[ii]>nxM ? nx[ii] : nxM;
	nuM = nu[ii]>nuM ? nu[ii] : nuM;
	nbM = nb[ii]>nbM ? nb[ii] : nbM;
	ngM = ng[ii]>ngM ? ng[ii] : ngM;


	// core struct
	struct IPM_HARD_CORE_QP_WORKSPACE *sr_ptr = mem;

	// core workspace
	workspace->core_workspace = sr_ptr;
	sr_ptr += 1;
	struct IPM_HARD_CORE_QP_WORKSPACE *cws = workspace->core_workspace;


	// matrix struct
	struct STRMAT *sm_ptr = (struct STRMAT *) sr_ptr;

	workspace->L = sm_ptr;
	sm_ptr += N+1;
	workspace->AL = sm_ptr;
	sm_ptr += 2;


	// vector struct
	struct STRVEC *sv_ptr = (struct STRVEC *) sm_ptr;

	workspace->dux = sv_ptr;
	sv_ptr += N+1;
	workspace->dpi = sv_ptr;
	sv_ptr += N;
	workspace->dt = sv_ptr;
	sv_ptr += N+1;
	workspace->res_g = sv_ptr;
	sv_ptr += N+1;
	workspace->res_b = sv_ptr;
	sv_ptr += N;
	workspace->res_d = sv_ptr;
	sv_ptr += N+1;
	workspace->res_m = sv_ptr;
	sv_ptr += N+1;
	workspace->Gamma = sv_ptr;
	sv_ptr += N+1;
	workspace->gamma = sv_ptr;
	sv_ptr += N+1;
	workspace->Pb = sv_ptr;
	sv_ptr += N;
	workspace->tmp_nbM = sv_ptr;
	sv_ptr += 1;
	workspace->tmp_nxM = sv_ptr;
	sv_ptr += 1;
	workspace->tmp_ngM = sv_ptr;
	sv_ptr += 2;


	// align to typicl cache line size
	size_t s_ptr = (size_t) sv_ptr;
	s_ptr = (s_ptr+63)/64*64;


	// void stuf
	char *c_ptr = (char *) s_ptr;

	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRMAT(nu[ii]+nx[ii]+1, nu[ii]+nx[ii], workspace->L+ii, c_ptr);
		c_ptr += (workspace->L+ii)->memory_size;
		}

	CREATE_STRMAT(nuM+nxM+1, nxM+ngM, workspace->AL+0, c_ptr);
	c_ptr += (workspace->AL+0)->memory_size;

	CREATE_STRMAT(nuM+nxM+1, nxM+ngM, workspace->AL+1, c_ptr);
	c_ptr += (workspace->AL+1)->memory_size;

	for(ii=0; ii<N; ii++)
		{
		CREATE_STRVEC(nx[ii+1], workspace->Pb+ii, c_ptr);
		c_ptr += (workspace->Pb+ii)->memory_size;
		}

	CREATE_STRVEC(nbM, workspace->tmp_nbM, c_ptr);
	c_ptr += workspace->tmp_nbM->memory_size;

	CREATE_STRVEC(nxM, workspace->tmp_nxM, c_ptr);
	c_ptr += workspace->tmp_nxM->memory_size;

	CREATE_STRVEC(ngM, workspace->tmp_ngM+0, c_ptr);
	c_ptr += (workspace->tmp_ngM+0)->memory_size;

	CREATE_STRVEC(ngM, workspace->tmp_ngM+1, c_ptr);
	c_ptr += (workspace->tmp_ngM+1)->memory_size;



	cws->nv = nvt;
	cws->ne = net;
	cws->nc = nct;
	cws->iter_max = arg->iter_max;
	CREATE_IPM_HARD_CORE_QP(cws, c_ptr);
	c_ptr += workspace->core_workspace->memsize;

	cws->alpha_min = arg->alpha_min;
	cws->mu_max = arg->mu_max;
	cws->mu0 = arg->mu0;
	cws->nt_inv = 1.0/(2*nct); // TODO avoid computation if nt=0 XXX


	// alias members of workspace and core_workspace
	//
	c_ptr = (char *) cws->dv;
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nu[ii]+nx[ii], workspace->dux+ii, c_ptr);
		c_ptr += (nu[ii]+nx[ii])*sizeof(REAL);
		}
	//
	c_ptr = (char *) cws->dpi;
	for(ii=0; ii<N; ii++)
		{
		CREATE_STRVEC(nx[ii+1], workspace->dpi+ii, c_ptr);
		c_ptr += (nx[ii+1])*sizeof(REAL);
		}
	//
	c_ptr = (char *) cws->dt;
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nb[ii], workspace->dt+ii, c_ptr);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		}
	//
	c_ptr = (char *) cws->res_g;
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nu[ii]+nx[ii], workspace->res_g+ii, c_ptr);
		c_ptr += (nu[ii]+nx[ii])*sizeof(REAL);
		}
	//
	c_ptr = (char *) cws->res_b;
	for(ii=0; ii<N; ii++)
		{
		CREATE_STRVEC(nx[ii+1], workspace->res_b+ii, c_ptr);
		c_ptr += (nx[ii+1])*sizeof(REAL);
		}
	//
	c_ptr = (char *) cws->res_d;
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nb[ii], workspace->res_d+ii, c_ptr);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		}
	//
	c_ptr = (char *) cws->res_m;
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nb[ii], workspace->res_m+ii, c_ptr);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		}
	//
	c_ptr = (char *) cws->Gamma;
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nb[ii], workspace->Gamma+ii, c_ptr);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		}
	//
	c_ptr = (char *) cws->gamma;
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nb[ii], workspace->gamma+ii, c_ptr);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		c_ptr += nb[ii]*sizeof(REAL);
		c_ptr += ng[ii]*sizeof(REAL);
		}
	//
	workspace->stat = cws->stat;

	return;

	}



void SOLVE_IPM_HARD_OCP_QP(struct OCP_QP *qp, struct OCP_QP_SOL *qp_sol, struct IPM_HARD_OCP_QP_WORKSPACE *ws)
	{

	struct IPM_HARD_CORE_QP_WORKSPACE *cws = ws->core_workspace;

	// alias qp vectors into qp_sol
	cws->v = qp_sol->ux->pa;
	cws->pi = qp_sol->pi->pa;
	cws->lam = qp_sol->lam->pa;
	cws->t = qp_sol->t->pa;

	if(cws->nc==0)
		{
		FACT_SOLVE_KKT_UNCONSTR_OCP_QP(qp, qp_sol, ws);
		COMPUTE_RES_HARD_OCP_QP(qp, qp_sol, ws);
		cws->mu = ws->res_mu;
		ws->iter = 0;
		return;
		}

	// init solver
	INIT_VAR_HARD_OCP_QP(qp, qp_sol, ws);

	// compute residuals
	COMPUTE_RES_HARD_OCP_QP(qp, qp_sol, ws);
	cws->mu = ws->res_mu;

	int kk;
	for(kk=0; kk<cws->iter_max & cws->mu>cws->mu_max; kk++)
		{

		// fact and solve kkt
		FACT_SOLVE_KKT_STEP_HARD_OCP_QP(qp, ws);

		// alpha
		COMPUTE_ALPHA_HARD_QP(cws);
		cws->stat[5*kk+0] = cws->alpha;

		//
		UPDATE_VAR_HARD_QP(cws);

		// compute residuals
		COMPUTE_RES_HARD_OCP_QP(qp, qp_sol, ws);
		cws->mu = ws->res_mu;
		cws->stat[5*kk+1] = ws->res_mu;

		}
	
	ws->iter = kk;
	
	return;

	}



void SOLVE_IPM2_HARD_OCP_QP(struct OCP_QP *qp, struct OCP_QP_SOL *qp_sol, struct IPM_HARD_OCP_QP_WORKSPACE *ws)
	{

	struct IPM_HARD_CORE_QP_WORKSPACE *cws = ws->core_workspace;

	// alias qp vectors into qp_sol
	cws->v = qp_sol->ux->pa;
	cws->pi = qp_sol->pi->pa;
	cws->lam = qp_sol->lam->pa;
	cws->t = qp_sol->t->pa;

	REAL tmp;

	if(cws->nc==0)
		{
		FACT_SOLVE_KKT_UNCONSTR_OCP_QP(qp, qp_sol, ws);
		COMPUTE_RES_HARD_OCP_QP(qp, qp_sol, ws);
		cws->mu = ws->res_mu;
		ws->iter = 0;
		return;
		}

	// init solver
	INIT_VAR_HARD_OCP_QP(qp, qp_sol, ws);

	// compute residuals
	COMPUTE_RES_HARD_OCP_QP(qp, qp_sol, ws);
	cws->mu = ws->res_mu;

#if 0
	printf("\nres_g\n");
	for(int ii=0; ii<=qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->nx[ii]+qp->nu[ii], ws->res_g+ii, 0);
		}
	printf("\nres_b\n");
	for(int ii=0; ii<qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->nx[ii+1], ws->res_b+ii, 0);
		}
	printf("\nres_d_lb\n");
	for(int ii=0; ii<qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->nb[ii], ws->res_d_lb+ii, 0);
		}
	printf("\nres_d_ub\n");
	for(int ii=0; ii<qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->nb[ii], ws->res_d_ub+ii, 0);
		}
	printf("\nres_d_lg\n");
	for(int ii=0; ii<qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->ng[ii], ws->res_d_lg+ii, 0);
		}
	printf("\nres_d_ug\n");
	for(int ii=0; ii<qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->ng[ii], ws->res_d_ug+ii, 0);
		}
	printf("\nres_m_lb\n");
	for(int ii=0; ii<qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->nb[ii], ws->res_m_lb+ii, 0);
		}
	printf("\nres_m_ub\n");
	for(int ii=0; ii<qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->nb[ii], ws->res_m_ub+ii, 0);
		}
	printf("\nres_m_lg\n");
	for(int ii=0; ii<qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->ng[ii], ws->res_m_lg+ii, 0);
		}
	printf("\nres_m_ug\n");
	for(int ii=0; ii<qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->ng[ii], ws->res_m_ug+ii, 0);
		}
	exit(1);
#endif

#if 0
	int ii;
	for(ii=0; ii<1; ii++)
		{
		cws->sigma = 1.0;
		cws->stat[5*kk+2] = cws->sigma;
		COMPUTE_CENTERING_CORRECTION_HARD_QP(cws);
		FACT_SOLVE_KKT_STEP_HARD_OCP_QP(qp, ws);
		COMPUTE_ALPHA_HARD_QP(cws);
		cws->stat[5*kk+3] = cws->alpha;
		UPDATE_VAR_HARD_QP(cws);
		COMPUTE_RES_HARD_OCP_QP(qp, qp_sol, ws);
		cws->mu = ws->res_mu;
		cws->stat[5*kk+4] = ws->res_mu;
		kk++;
		}
//	ws->iter = kk;
//		return;
#endif

	int kk = 0;
	for(; kk<cws->iter_max & cws->mu>cws->mu_max; kk++)
		{

		// fact and solve kkt
		FACT_SOLVE_KKT_STEP_HARD_OCP_QP(qp, ws);

#if 0
	printf("\ndux\n");
	for(int ii=0; ii<=qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->nx[ii]+qp->nu[ii], ws->dux+ii, 0);
		}
	printf("\ndpi\n");
	for(int ii=0; ii<qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(qp->nx[ii+1], ws->dpi+ii, 0);
		}
	printf("\ndt\n");
	for(int ii=0; ii<=qp->N; ii++)
		{
		PRINT_E_TRAN_STRVEC(2*qp->nb[ii]+2*qp->ng[ii], ws->dt_lb+ii, 0);
		}
	exit(1);
#endif
		// alpha
		COMPUTE_ALPHA_HARD_QP(cws);
		cws->stat[5*kk+0] = cws->alpha;

		// mu_aff
		COMPUTE_MU_AFF_HARD_QP(cws);
		cws->stat[5*kk+1] = cws->mu_aff;

		tmp = cws->mu_aff/cws->mu;
		cws->sigma = tmp*tmp*tmp;
		cws->stat[5*kk+2] = cws->sigma;

		COMPUTE_CENTERING_CORRECTION_HARD_QP(cws);

		// fact and solve kkt
		SOLVE_KKT_STEP_HARD_OCP_QP(qp, ws);

#if 0
int ii;
for(ii=0; ii<=qp->N; ii++)
	d_print_tran_strvec(qp->nu[ii]+qp->nx[ii], ws->dux+ii, 0);
for(ii=0; ii<qp->N; ii++)
	d_print_tran_strvec(qp->nx[ii+1], ws->dpi+ii, 0);
exit(1);
#endif
		// alpha
		COMPUTE_ALPHA_HARD_QP(cws);
		cws->stat[5*kk+3] = cws->alpha;

		//
		UPDATE_VAR_HARD_QP(cws);

		// compute residuals
		COMPUTE_RES_HARD_OCP_QP(qp, qp_sol, ws);
		cws->mu = ws->res_mu;
		cws->stat[5*kk+4] = ws->res_mu;

		}
	
	ws->iter = kk;
	
	return;

	}



