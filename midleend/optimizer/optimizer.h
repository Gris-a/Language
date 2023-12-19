#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "../../general/log/log.h"
#include "../../general/tree/include/tree.h"
#include "../save_tree/save_tree.h"
#include "../DSL.h"

const double M_ERR = 1e-7;

void Optimizer(const char *in, const char *out);

#endif //OPTIMIZER_H