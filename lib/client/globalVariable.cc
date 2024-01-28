#include "globalVariable.h"

std::mutex mtx_download;
std::mutex mtx_upload;
long total_downloaddata = 0;
long total_uploaddata = 0;
std::mutex mtx_delay_up;
std::mutex mtx_delay_down;
int delay_up = 0;
int delay_down = 0;
