#include "drofs_timestamp_helper.h"

#include <stdint.h>
#include <time.h>


void format_timestamp(uint32_t timestamp, char *buffer, size_t buffer_size) {
    struct tm *tm_info;

    // Convert the timestamp to a struct tm
    time_t t = (time_t)timestamp; 
    tm_info = localtime(&t);

    // Format the time as a string
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);
}
