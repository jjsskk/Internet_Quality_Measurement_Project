#ifndef GLOVAL_VARIABLE 
#define GLOVAL_VARIABLE
#include <mutex>

extern std::mutex mtx_download;
extern std::mutex mtx_upload;
extern long total_downloaddata;
extern long total_uploaddata;
extern std::mutex mtx_delay_up;
extern std::mutex mtx_delay_down;
extern int delay_up;
extern int delay_down;

#endif