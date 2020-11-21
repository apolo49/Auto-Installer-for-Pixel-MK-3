#include <filesystem>
#include <string>
#include <functional>
#include <algorithm>
#include <io.h>
#include <iostream>
#include <fstream>
#include <boost/container/vector.hpp>

#include "../vender/zip.hpp"
//#include <elzip/elzip.hpp>
#define DefaultProgressCallback [](double dA, double dB, const char* name) { dA; dB; }
#define DefaultErrorCallback [](const std::string& strErrorMsg) { std::cout << strErrorMsg << std::endl; }
#define MAX_FILE_BUFFER		(32 * 20 * 820)

namespace fs = std::filesystem;

namespace fileHandling
{
	int IsDirectory(const std::string& strPath);
	int IsFile(const std::string& strPath);
	int CreateDirectories(const std::string& strPath);

	int Extract(const std::string& strOutputDirectory, const std::string& strZipFile, double* Progress, double* TotProgress, std::string* ProgDsc, boost::container::vector<bool> ResAndOpts);

	namespace
	{
		class Extractor {
		private:
			typedef std::function<void(double, double, const char*)> ProgressCallback;
			typedef std::function<void(const std::string&)> ErrorCallback;
		public:
			static int ExtractAllFilesFromZip(const std::string& strOutputDirectory, const std::string& strZipFile, boost::container::vector<bool> ResAndOpts, ProgressCallback ProgressStrategy = DefaultProgressCallback, ErrorCallback ErrorStrategy = DefaultErrorCallback);
		};
	}
}