#include "renoise.h"
#include <math.h>

Renoise_Gradient_Point renoise_gradient_point_generate() {
    double angle = ((double) random() / 2147483647.0) * M_2_PI;
    return(Renoise_Gradient_Point) {
        .x = cos(angle),
        .y = sin(angle),
    };
}
