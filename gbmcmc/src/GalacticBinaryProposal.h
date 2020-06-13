/*
 *  Copyright (C) 2019 Tyson B. Littenberg (MSFC-ST12), Neil J. Cornish, Kristen Lackeos
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with with program; see the file COPYING. If not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 */

/// @file GalacticBinaryProposal.h


#ifndef GalacticBinaryProposal_h
#define GalacticBinaryProposal_h

#ifdef __GNUC__
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

#include <stdio.h>

/*!
 \brief Prototype structure for proposal distributions.
 
 Generic data structure for holding all information needed by proposal distributions.
 Structure contains function for drawing new parameters, evaluating the proposal density,
 tracking acceptance ratios, and various book-keeping scalars, vectors, and matrices to
 hold needed metadata.

*/

struct Proposal
{
    /**
     \brief Function that generates updated source parameters.
     
     @param[in]  params parameter vector
     @param[out] params updated parameter values
     @param[out] logQ proposal density
     */
    double (*function)(struct Data*,struct Model*,struct Source*,struct Proposal*,double*,gsl_rng*);

    /**
     \brief Compute proposal density given parameters.
     
     @param[in]  params parameter vector
     @param[out] logQ proposal density
     */
    double (*density)(struct Data*, struct Model*, struct Source*,struct Proposal*,double*);
    
    int *trial;      //!<total number of trials for proposal
    int *accept;     //!<total number of accepted trials for proposals*/
    char name[128];  //!<string identifying proposal type
    double norm;     //!<proposal normalization
    double maxp;     //!<max value of proposal density for rejection sampling
    double weight;   //!<proposal weight [0,1] for fixed dimension moves
    double rjweight; //!<proposal weight [0,1] for trans dimensional moves
    int size;        //!<size of proposal arrays
    double *vector;  //!<utility 1D array for proposal metadata
    double **matrix; //!<utility 2D array for proposal metadata
    double ***tensor;//!<utility 3D array for proposal metadata
};

/** Compute whitened power spectrum of data and normalize to preferentially draw frequencies with excess power */
void setup_frequency_proposal(struct Data *data);

/** Compute and print acceptance ratios for each proposal */
void print_acceptance_rates(struct Proposal **proposal, int NP, int ic, FILE *fptr);

/** Shift start time of data segment */
double t0_shift                 (UNUSED struct Data *data, struct Model *model, UNUSED struct Source *source, UNUSED struct Proposal *proposal, UNUSED double *params, gsl_rng *seed);

/** Fair draw from prior for each parameter */
double draw_from_prior          (UNUSED struct Data *data, struct Model *model, UNUSED struct Source *source, struct Proposal *proposal, double *params, gsl_rng *seed);

/** Fair draw from prior for location and orientation parameters */
double draw_from_extrinsic_prior(UNUSED struct Data *data, struct Model *model, UNUSED struct Source *source, struct Proposal *proposal, double *params, gsl_rng *seed);

/**
 \brief Jump from current location along eigenvectors of Fisher information matrix
 
 Use precomputed Fisher matrix
 \f$ \Gamma_{ij} \equiv \langle \frac{\partial^2 h}{\partial \theta_i^2} \vert \frac{\partial^2 h}{\partial\theta_j^2} \rangle \f$
 and center it at current parameters.
 Choose one eigenvector at random and propose a Gaussian jump along that direction
 scaled by the associated eigenvalue.
 
 Jumps are conditioned against singular values, in which case the jump along that parameter
 direction is 0.01 the current value.
 
 The proposal is assumed symmetric because the Fisher matrix is not being updated at each location.
 
 The Fisher matrix is updated periodically during the MCMC.

 */
double draw_from_fisher         (UNUSED struct Data *data, struct Model *model, struct Source *source, struct Proposal *proposal, double *params, gsl_rng *seed);

/**
 \brief Draw each parameter from 1D marginalized CDF
 
 Use CDFs constructed from chain file input at command line with --update flag to draw new parameters.  The proposal draws a p-value from \f$U[0,1]\f$ and interpolates the samples from the input chain file to find the associated parameter value.
 
 */
double draw_from_cdf            (UNUSED struct Data *data, struct Model *model, struct Source *source, struct Proposal *proposal, double *params, gsl_rng *seed);

/**
 \brief Draw from multivariate Gaussian characterized by chain covariance
 
 Use covariance matrices \f$ \bf{C}\f$ input at command line with --update_cov flag to draw new parameters.  The covariance matrices are typically computed from already acquired chain files (e.g., from a previous run on less data).
 
 
 The posteriors are bimodal so each source is represented by two multivariate Gaussians, one at each mode, characterized by the covariance matrix of the samples associated with each mode.
 
 The proposal first randomly selects a source, then randomly selects a mode from the source.  The proposed values are then computed using
 \f$ \vec\theta_y = \vec\theta_0 + {\bf L}\vec{n}\f$ where
 \f$ \vec{n} \f$ are fair draws from \f$ N[0,1]\f$,
 \f$ \vec\theta_0 \f$ are the mode centroids from the input chain,
 and \f$ {\bf}L \f$ is the LU decomposition of \f$ \bf{C} \f$.

 */
double draw_from_cov            (UNUSED struct Data *data, struct Model *model, struct Source *source, struct Proposal *proposal, double *params, gsl_rng *seed);


double draw_from_fstatistic     (struct Data *data, UNUSED struct Model *model, UNUSED struct Source *source, struct Proposal *proposal, double *params, gsl_rng *seed);
double draw_signal_amplitude    (struct Data *data, struct Model *model, UNUSED struct Source *source, UNUSED struct Proposal *proposal, double *params, gsl_rng *seed);
double draw_from_spectrum       (struct Data *data, struct Model *model, struct Source *source, UNUSED struct Proposal *proposal, double *params, gsl_rng *seed);
double fm_shift                 (struct Data *data, struct Model *model, struct Source *source, struct Proposal *proposal, double *params, gsl_rng *seed);
double psi_phi_jump             (UNUSED struct Data *data, UNUSED struct Model *model, struct Source *source, UNUSED struct Proposal *proposal, double *params, gsl_rng *seed);
double jump_from_fstatistic     (struct Data *data, struct Model *model, struct Source *source, struct Proposal *proposal, double *params, gsl_rng *seed);
double draw_from_galaxy_prior   (struct Model *model, struct Prior *prior, double *params, gsl_rng *seed);

double draw_calibration_parameters(struct Data *data, struct Model *model, gsl_rng *seed);

//double cdf_density(struct Model *model, struct Source *source, struct Proposal *proposal);
double cdf_density(UNUSED struct Data *data, struct Model *model, struct Source * source, struct Proposal *proposal, UNUSED double *params);

//double cov_density(struct Model *model, struct Source *source, struct Proposal *proposal);
double cov_density(UNUSED struct Data *data, struct Model *model, struct Source *source, struct Proposal *proposal, double *params);


void initialize_proposal(struct Orbit *orbit, struct Data *data, struct Prior *prior, struct Chain *chain, struct Flags *flags, struct Proposal **proposal, int NMAX);

void setup_fstatistic_proposal(struct Orbit *orbit, struct Data *data, struct Flags *flags, struct Proposal *proposal);

void setup_prior_proposal(struct Flags *flags, struct Prior *prior, struct Proposal *proposal);

void test_covariance_proposal(struct Data *data, struct Flags *flags, struct Model *model, struct Prior *prior, struct Proposal *proposal, gsl_rng *seed);

void setup_cdf_proposal(struct Data *data, struct Flags *flags, struct Proposal *proposal, int NMAX);

void setup_covariance_proposal(struct Data *data, struct Flags *flags, struct Proposal *proposal);

//double evaluate_fstatistic_proposal(struct Data *data, struct Proposal *proposal, double *params);
double evaluate_fstatistic_proposal(struct Data *data, UNUSED struct Model *model, UNUSED struct Source * source, struct Proposal *proposal, double *params);

double prior_density(struct Data *data, struct Model *model, UNUSED struct Source *source, struct Proposal *proposal, double *params);

//dummy function for proposal that don't have density functions defined
double symmetric_density(UNUSED struct Data *data, UNUSED struct Model *model, UNUSED struct Source *source, UNUSED struct Proposal *proposal, UNUSED double *params);

#endif /* GalacticBinaryProposal_h */
