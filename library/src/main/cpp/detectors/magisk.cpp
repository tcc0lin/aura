#include "detectors.h"
#include "string"
#include "../utils/logging.h"

__attribute__((always_inline))
void magisk_detect_by_exec_path() {
    LOGI("--------------------magisk_detect_by_exec_path start--------------------");
    int status = 0;
    char *path = getenv("PATH");
    char *p = strtok(path, ":");
    char supath[PATH_MAX];
    do {
        sprintf(supath, "%s/su", p);
        if (access(supath, F_OK) == 0) {
            LOGI("Found su at %s", supath);
            status = 3;
        }
    } while ((p = strtok(NULL, ":")) != NULL);
    set_detection_value(7, status);
    LOGI("--------------------magisk_detect_by_exec_path end--------------------");
}
