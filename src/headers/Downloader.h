#include "curl/curl.h"
#include <string>
#include <fstream>
#include <thread>
#include <filesystem>
#define STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES         670641754

class Downloader {
public:
	static int GetFile(const char* url, const char outFileName[FILENAME_MAX], double* Progress, double* TotalProgress, double ProgBefore, double remainder);

private:

	struct myprogress {
		double* dlProgress;
		double* TotalProgress;
		double ProgBefore;
		double Remainder;
	};

	static int progress_callback(void* p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
};