#include "../headers/Downloader.h"

int Downloader::GetFile(const char* url, const char outFileName[FILENAME_MAX], double* Progress, double* TotProg, double ProgBefore, double remainder) {
	try {
		CURL* curl;
		CURLcode res = CURLE_OK;
		curl = curl_easy_init();
		struct myprogress prog;
		if (curl) {
			prog.dlProgress = Progress;
			prog.TotalProgress = TotProg;
			prog.ProgBefore = ProgBefore;
			prog.Remainder = remainder;
			FILE* fp = fopen(outFileName, "wb");
			curl_easy_setopt(curl, CURLOPT_URL, url);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
			curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &prog /*Progress*/);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)fp);
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
			res = curl_easy_perform(curl);

			if (res != CURLE_OK)
				fprintf(stderr, "%s\n", curl_easy_strerror(res));

			/* always cleanup */
			curl_easy_cleanup(curl);
			fclose(fp);
		}
		return 0;
	}
	catch (std::exception& e) {
		return 1;
	}
}

int Downloader::progress_callback(void* p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	struct myprogress* myp = (struct myprogress*)p;
	long total = (long)dltotal;
	long downloaded = (long)dlnow;
	double percent;
	if (total > 0) {
		percent = ((double)downloaded / (double)total);
	}
	else {
		percent = 0.0f;
	}
	*(myp->TotalProgress) = (myp->ProgBefore) + ((percent * myp->Remainder));
	*(myp->dlProgress) = (percent);

	if (dlnow > STOP_DOWNLOAD_AFTER_THIS_MANY_BYTES) {
		return 1;
	}
	return 0;
}