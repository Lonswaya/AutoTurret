#include "MotionDetector.h"

#include <stdio.h>

int main() {
    MotionConfig config;
    config.blur_size = 0;
    config.motion_thresh = 100;

    MotionDetector md;
    md_init(&md, &config);

    int x, y, dx, dy;

    while(1) {
        md_detect(&md, &x, &y, &dx, &dy);
    }

}
