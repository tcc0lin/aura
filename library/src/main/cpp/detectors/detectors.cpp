#include "detectors.h"

int g_detectionResults[256] = {0};
const size_t g_detectionSize = sizeof(g_detectionResults) / sizeof(g_detectionResults[0]);

void set_detection_value(size_t index, int value) {
    if (index >= g_detectionSize) {
        return;
    }
    if (value < 1 || value > 4) {
        return;
    }
    g_detectionResults[index] = value;
}