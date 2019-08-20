/* ----------------------------------------------------------------------
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   Copyright (2003) Sandia Corporation.  Under the terms of Contract
   DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
   certain rights in this software.  This software is distributed under
   the GNU General Public License.

   See the README file in the top-level LAMMPS directory.
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   Contributing author: Taylor Barnes (MolSSI)
   MolSSI Driver Interface (MDI) support for LAMMPS
------------------------------------------------------------------------- */

#include <mpi.h>
#include <string.h>
#include "driver.h"
#include "atom.h"
#include "domain.h"
#include "update.h"
#include "force.h"
#include "min.h"
#include "minimize.h"
#include "modify.h"
#include "output.h"
#include "timer.h"
#include "error.h"
#include "comm.h"
#include "fix_driver.h"
#include "mdi.h"

#include "verlet.h"

using namespace LAMMPS_NS;

/* ---------------------------------------------------------------------- */

CommandMDI::CommandMDI(LAMMPS *lmp) : Pointers(lmp) {
  return;
}

CommandMDI::~CommandMDI() {
  return;
}

/* ---------------------------------------------------------------------- */

void CommandMDI::command(int narg, char **arg)
{

  // identify the driver fix
  for (int i = 0; i < modify->nfix; i++) {
    if (strcmp(modify->fix[i]->style,"mdi") == 0) {
      mdi_fix = static_cast<FixMDI*>(modify->fix[i]);
    }
  }

  /* format for MDI command:
   * mdi
   */
  if (narg > 0) error->all(FLERR,"Illegal MDI command");

  if (atom->tag_enable == 0)
    error->all(FLERR,"Cannot use MDI command without atom IDs");

  if (atom->tag_consecutive() == 0)
    error->all(FLERR,"MDI command requires consecutive atom IDs");

  // begin engine_mode
  char *command = NULL;
  while ( true ) {
    command = mdi_fix->engine_mode(0);

    if (strcmp(command,"INIT_MD") == 0 ) {
      // enter MDI simulation control loop
      int received_exit = mdi_md();
      if ( received_exit == 1 ) {
	return;
      }
    }
    else if (strcmp(command,"EXIT") == 0 ) {
      return;
    }
    else {
      error->all(FLERR,strcat("MDI received unsupported command: ",command));
    }
  }

  return;

}



int CommandMDI::mdi_md()
{
  // initialize an MD simulation
  update->whichflag = 1; // 1 for dynamics
  timer->init_timeout();
  update->nsteps = 1;
  update->ntimestep = 0;
  update->firststep = update->ntimestep;
  update->laststep = update->ntimestep + update->nsteps;
  update->beginstep = update->firststep;
  update->endstep = update->laststep;
  lmp->init();

  // the MD simulation is now at the @MD_INIT node
  char *command = NULL;
  command = mdi_fix->engine_mode(-1);

  // only two commands are valid at this point
  // SHOULD PROBABLY HAVE:
  // if ( command.scope < "GLOBAL" ) {
  //    pass
  // }
  if (strcmp(command,"MD_EXIT") == 0 ) {
    // return, but do not flag for global exit
    return 0;
  }
  else if (strcmp(command,"EXIT") == 0 ) {
    // return, and flag for global exit
    return 1;
  }
  // Don't include the error until after the command.scope check is added above
  /*
  else {
    error->all(FLERR,strcat("MDI received unsupported command: ",command));
  }
  */

  // NOTE: CURRENTLY TRUSTING THAT command = @FORCES

  // continue the MD simulation
  update->integrate->setup(1);

  //<<<<
  double **f2 = atom->f;
  fprintf(screen,"SSS MDI2: %f %f %f\n",f2[0][0], f2[0][1], f2[0][2]);
  //>>>>

  // the MD simulation is now at the @FORCES node
  command = mdi_fix->engine_mode(3);
  //command = mdi_fix->engine_mode(2);
  command = mdi_fix->command;

  // only two commands are valid at this point
  // SHOULD PROBABLY HAVE:
  // if ( command.scope < "GLOBAL" ) {
  //    pass
  // }
  if (strcmp(command,"EXIT_SIM") == 0 ) {
    // return, but do not flag for global exit
    return 0;
  }
  else if (strcmp(command,"EXIT") == 0 ) {
    // return, and flag for global exit
    return 1;
  }
  // Don't include the error until after the command.scope check is added above
  /*
  else {
    error->all(FLERR,strcat("MDI received unsupported command: ",command));
  }
  */

  // do MD iterations until told to exit
  while ( true ) {

    // run an MD timestep
    update->whichflag = 1; // 1 for dynamics
    timer->init_timeout();
    update->nsteps += 1;
    update->laststep += 1;
    update->endstep = update->laststep;
    output->next = update->ntimestep + 1;
    update->integrate->run(1);

    // get the most recent command the MDI engine received
    command = mdi_fix->command;

    // only two commands are valid at this point
    // SHOULD PROBABLY HAVE:
    // if ( command.scope < "GLOBAL" ) {
    //    pass
    // }
    if (strcmp(command,"EXIT_SIM") == 0 ) {
      // return, but do not flag for global exit
      return 0;
    }
    else if (strcmp(command,"EXIT") == 0 ) {
      // return, and flag for global exit
      return 1;
    }
    // Don't include the error until after the command.scope check is added above
    /*
    else {
      error->all(FLERR,strcat("MDI received unsupported command: ",command));
    }
    */

  }

}
