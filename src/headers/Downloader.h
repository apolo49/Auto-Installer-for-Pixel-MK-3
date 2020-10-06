#include "curl/curl.h"
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <filesystem>

class Downloader {
public:
	static int GetFile(const char* url, const char outFileName[FILENAME_MAX], float* Progress) {
		try {
			CURL* curl;
			FILE* fp;
			CURLcode res;
			curl = curl_easy_init();
			if (curl) {
				fp = fopen(outFileName, "wb");
				curl_easy_setopt(curl, CURLOPT_URL, url);
				curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
				curl_easy_setopt(curl, CURLOPT_XFERINFODATA, Progress);
				curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, &progress_callback);
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_data);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
				res = curl_easy_perform(curl);
				/* always cleanup */
				curl_easy_cleanup(curl);
				fclose(fp);
			}
			std::cout << res << std::endl;
			return 0;
		}
		catch (std::exception& e) {
			std::cout << e.what() << std::endl;
			return 1;
		}
	}

private:

	static int progress_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
		*((float*)clientp) = (dlnow / dltotal);
		if (dlnow == dltotal) {
			return 1;
		}
		return 0;
	}

	static size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
		size_t written = fwrite(ptr, size, nmemb, stream);
		return written;
	}
};