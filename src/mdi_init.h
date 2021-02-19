/* -*- c++ -*- ----------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

#ifndef LMP_INIT_MDI_H
#define LMP_INIT_MDI_H

/* This is just a wrapper for the MDI Library's MDI_Init function, which simplifies
   the process of compiling LAMMPS if the MDI Library has not been built.
*/
int init_mdi(const char* options, void* world_comm) { return -1; };

#endif
