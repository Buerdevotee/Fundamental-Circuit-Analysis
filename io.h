#ifndef IO_H
#define IO_H

#include "circuit.h"

/* Save/load circuit in a simple text format */
int io_save(const Circuit *c, const char *fname);
int io_load(Circuit *c, const char *fname);

/* Export samples to CSV (time, voltage) for a node index */
int io_export_csv(const char *fname, const double *times, const double *voltages, int n, int node_index);

#endif /* IO_H */
