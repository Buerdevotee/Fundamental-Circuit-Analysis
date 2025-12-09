#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "solver.h"
#include "util.h"

/* simple matrix allocate/free for dense n x n */
static double *mat_alloc(int n) {
    return calloc(n * n, sizeof(double));
}
static void mat_free(double *m) { free(m); }

/* Gaussian elimination with partial pivoting */
static int solve_linear_system(double *A, double *b, int n) {
    for (int k = 0; k < n; ++k) {
        /* pivot */
        int piv = k;
        double max = fabs(A[k * n + k]);
        for (int i = k + 1; i < n; ++i) {
            double val = fabs(A[i * n + k]);
            if (val > max) { max = val; piv = i; }
        }
        if (max < 1e-14) return -1; /* singular or ill-conditioned */
        if (piv != k) {
            for (int j = k; j < n; ++j) {
                double tmp = A[k * n + j]; A[k * n + j] = A[piv * n + j]; A[piv * n + j] = tmp;
            }
            double tmpb = b[k]; b[k] = b[piv]; b[piv] = tmpb;
        }
        double akk = A[k * n + k];
        for (int i = k + 1; i < n; ++i) {
            double factor = A[i * n + k] / akk;
            for (int j = k; j < n; ++j) A[i * n + j] -= factor * A[k * n + j];
            b[i] -= factor * b[k];
        }
    }
    /* back substitution */
    for (int i = n - 1; i >= 0; --i) {
        double s = b[i];
        for (int j = i + 1; j < n; ++j) s -= A[i * n + j] * b[j];
        b[i] = s / A[i * n + i];
    }
    return 0;
}

/* Note: This solver is intentionally simple for teaching.
 * It only supports resistive networks and voltage sources that reference ground.
 */
int nodal_solve(const Circuit *c, double *voltages) {
    if (!c || !voltages) return -1;
    int N = c->node_count;
    if (N <= 1) { voltages[0] = 0.0; return 0; }

    /* mark nodes with fixed voltages (due to voltage sources to ground) */
    double *known_v = xmalloc(sizeof(double) * N);
    int *is_known = xmalloc(sizeof(int) * N);
    for (int i = 0; i < N; ++i) { known_v[i] = NAN; is_known[i] = 0; }

    for (int i = 0; i < c->comp_count; ++i) {
        const Component *p = &c->components[i];
        if (p->type == COMP_VOLTAGE_SOURCE) {
            /* only support voltage source between node and ground */
            if (p->node_pos == 0 && p->node_neg != 0) {
                is_known[p->node_neg] = 1; known_v[p->node_neg] = -p->value;
            } else if (p->node_neg == 0 && p->node_pos != 0) {
                is_known[p->node_pos] = 1; known_v[p->node_pos] = p->value;
            } else if (p->node_pos != 0 && p->node_neg != 0) {
                fprintf(stderr, "Error: voltage source between non-ground nodes not supported here.\n");
                free(known_v); free(is_known); return -2;
            }
        }
    }

    /* build reduced system for unknown nodes (exclude ground and known) */
    int *map = xmalloc(sizeof(int) * N); /* node -> index in reduced system or -1 */
    int unknown_count = 0;
    for (int i = 0; i < N; ++i) {
        if (i == 0 || is_known[i]) map[i] = -1;
        else map[i] = unknown_count++;
    }

    double *G = NULL; double *I = NULL;
    if (unknown_count > 0) {
        G = mat_alloc(unknown_count);
        I = calloc(unknown_count, sizeof(double));
    }

    /* accumulate conductances */
    for (int i = 0; i < c->comp_count; ++i) {
        const Component *p = &c->components[i];
        if (p->type == COMP_RESISTOR) {
            int a = p->node_pos, b = p->node_neg;
            double g = 1.0 / p->value;
            if (a != 0 && !is_known[a]) G[map[a] * unknown_count + map[a]] += g;
            if (b != 0 && !is_known[b]) G[map[b] * unknown_count + map[b]] += g;
            if (a != 0 && b != 0 && !is_known[a] && !is_known[b]) {
                G[map[a] * unknown_count + map[b]] -= g;
                G[map[b] * unknown_count + map[a]] -= g;
            }
            /* contributions from known voltages -> move to I */
            if (a != 0 && !is_known[a] && b != 0 && is_known[b]) I[map[a]] += g * known_v[b];
            if (b != 0 && !is_known[b] && a != 0 && is_known[a]) I[map[b]] += g * known_v[a];
            /* if one side is ground (0) and other unknown: nothing extra (ground is 0) */
        }
    }

    double *x = NULL;
    if (unknown_count > 0) {
        x = xmalloc(sizeof(double) * unknown_count);
        for (int i = 0; i < unknown_count; ++i) x[i] = I[i];
        if (solve_linear_system(G, x, unknown_count) != 0) {
            fprintf(stderr, "Linear solver failed\n");
            free(known_v); free(is_known); free(map); if (G) mat_free(G); if (I) free(I); free(x);
            return -3;
        }
    }

    /* fill voltages array */
    for (int i = 0; i < N; ++i) voltages[i] = 0.0;
    for (int i = 0; i < N; ++i) {
        if (i == 0) voltages[i] = 0.0;
        else if (is_known[i]) voltages[i] = known_v[i];
        else voltages[i] = x ? x[map[i]] : 0.0;
    }

    free(known_v); free(is_known); free(map);
    if (G) mat_free(G); if (I) free(I); if (x) free(x);
    return 0;
}

/* Very small analytic RC demo:
 * Finds single R, single C, and single V source all connected to same node (node-ground).
 * Returns v(t) = V*(1 - exp(-t/(RC))) samples.
 */
int rc_analytic_step(const Circuit *c, double dt, double tmax,
                      double **out_times, double **out_voltages, int *out_n) {
    int rid = -1, cid = -1, vid = -1;
    for (int i = 0; i < c->comp_count; ++i) {
        if (c->components[i].type == COMP_RESISTOR) rid = i;
        else if (c->components[i].type == COMP_CAPACITOR) cid = i;
        else if (c->components[i].type == COMP_VOLTAGE_SOURCE) vid = i;
    }
    if (rid < 0 || cid < 0 || vid < 0) return -1;

    Component R = c->components[rid];
    Component C = c->components[cid];
    Component V = c->components[vid];

    /* require they share a node (node-ground) */
    int nodeR = (R.node_pos == 0) ? R.node_neg : R.node_pos;
    int nodeC = (C.node_pos == 0) ? C.node_neg : C.node_pos;
    int nodeV = (V.node_pos == 0) ? V.node_neg : V.node_pos;
    if (!(nodeR == nodeC && nodeR == nodeV)) return -2;

    double Rval = R.value;
    double Cval = C.value;
    double Vval = (V.node_pos == nodeV) ? V.value : -V.value;

    int nsteps = (int)ceil(tmax / dt) + 1;
    double *times = xmalloc(sizeof(double) * nsteps);
    double *voltages = xmalloc(sizeof(double) * nsteps);
    double tau = Rval * Cval;

    for (int k = 0; k < nsteps; ++k) {
        double t = k * dt;
        times[k] = t;
        voltages[k] = Vval * (1.0 - exp(-t / tau));
    }
    *out_times = times; *out_voltages = voltages; *out_n = nsteps;
    return 0;
}
