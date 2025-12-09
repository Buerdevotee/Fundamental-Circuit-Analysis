#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "io.h"
#include "circuit.h"
#include "util.h"

/* File format (simple):
 * Lines: TYP ID VALUE NODE_POS NODE_NEG
 * TYP = 'R' | 'V' | 'C'
 */

int io_save(const Circuit *c, const char *fname) {
    if (!c || !fname) return -1;
    FILE *f = fopen(fname, "w");
    if (!f) return -1;
    fprintf(f, "# node_count %d\n", c->node_count);
    for (int i = 0; i < c->comp_count; ++i) {
        const Component *p = &c->components[i];
        const char *t = (p->type == COMP_RESISTOR) ? "R" : (p->type == COMP_VOLTAGE_SOURCE) ? "V" : "C";
        fprintf(f, "%s %d %.12g %d %d\n", t, p->id, p->value, p->node_pos, p->node_neg);
    }
    fclose(f);
    return 0;
}

int io_load(Circuit *c, const char *fname) {
    if (!c || !fname) return -1;
    FILE *f = fopen(fname, "r");
    if (!f) return -1;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        char typ;
        int id; double val; int n1, n2;
        if (sscanf(line, " %c %d %lf %d %d", &typ, &id, &val, &n1, &n2) >= 5) {
            ComponentType ct = COMP_RESISTOR;
            if (typ == 'R') ct = COMP_RESISTOR;
            else if (typ == 'V') ct = COMP_VOLTAGE_SOURCE;
            else if (typ == 'C') ct = COMP_CAPACITOR;
            circuit_add_component(c, ct, val, n1, n2);
        }
    }
    fclose(f);
    return 0;
}

int io_export_csv(const char *fname, const double *times, const double *voltages, int n, int node_index) {
    FILE *f = fopen(fname, "w");
    if (!f) return -1;
    fprintf(f, "time,voltage_node_%d\n", node_index);
    for (int i = 0; i < n; ++i) fprintf(f, "%.9g,%.9g\n", times[i], voltages[i]);
    fclose(f);
    return 0;
}
