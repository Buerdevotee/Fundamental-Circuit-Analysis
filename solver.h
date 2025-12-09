#ifndef SOLVER_H
#define SOLVER_H

#include "circuit.h"

/* Nodal DC solver:
 * - Supports resistors and voltage sources that reference ground only.
 * - 'voltages' must point to an array sized at least circuit->node_count.
 * Return 0 on success, negative on error.
 */
int nodal_solve(const Circuit *c, double *voltages);

/* RC analytic step-response:
 * - For educational demo: detects a single R, single C and a single voltage source
 *   all connected to the same node (node-ground) and returns v(t) samples.
 * - Caller receives allocated arrays (*out_times, *out_voltages) and must free them.
 */
int rc_analytic_step(const Circuit *c, double dt, double tmax,
                      double **out_times, double **out_voltages, int *out_n);

#endif /* SOLVER_H */
