#ifndef CIRCUIT_H
#define CIRCUIT_H

/* Component types supported by this simple analyser */
typedef enum { COMP_RESISTOR, COMP_VOLTAGE_SOURCE, COMP_CAPACITOR } ComponentType;

/* Single component record */
typedef struct {
    int id;                 /* unique id */
    ComponentType type;
    double value;           /* ohm, volt, farad */
    int node_pos;           /* positive node (0 = ground) */
    int node_neg;           /* negative node */
} Component;

/* Circuit: dynamic array of components + node count */
typedef struct {
    Component *components;
    int comp_count;
    int comp_capacity;
    int node_count;         /* number of nodes (max node index + 1). Node 0 = ground */
} Circuit;

/* lifecycle */
Circuit *circuit_create(void);
void circuit_free(Circuit *c);

/* management */
int circuit_add_component(Circuit *c, ComponentType type, double value, int npos, int nneg);
void circuit_list(const Circuit *c);

#endif /* CIRCUIT_H */
