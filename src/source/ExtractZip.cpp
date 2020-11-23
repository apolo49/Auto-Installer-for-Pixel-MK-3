#include "../headers/ExtractZip.h"

/**
*	@brief extracts all entries from a .zip file
*
*	@param strDirectory, std::string& to the output directory
*	@param strZipFile, path to the zip file
*	@param uCount, amount of extracted entries
*	@param ProgressStrategy, callback for the amount of progress undertaken by the program
*	@param ErrorStrategy, callback for handling errors thrown by the program
*
*	@return int, 0 for failure, 1 for success
*
*/
int fileHandling::Extractor::ExtractAllFilesFromZip(const std::string& strDirectory, const std::string& strZipFile, boost::container::vector<bool> ResAndOpts, ProgressCallback ProgressStrategy, ErrorCallback ErrorStrategy)
{
	std::string strOutputDirectory(strDirectory);
	if (!fs::is_directory(strDirectory) || !fs::exists(strZipFile))
		return 0;

	if (strOutputDirectory.at(strOutputDirectory.size() - 1) != '/'
		&& strOutputDirectory.at(strOutputDirectory.size() - 1) != '\\')
	{
		if (strOutputDirectory.find_first_of('/') != std::string::npos)
			strOutputDirectory.append("/");
		else
			strOutputDirectory.append("\\");
	}

	char buf[255];
	int err;
	zip* archive;
	if ((archive = zip_open(strZipFile.c_str(), 0, &err)) == NULL) {
		zip_error_to_str(buf, sizeof(buf), err, errno);
		std::cout << "Can't open zip archive '" << archive << "': " << buf << std::endl;
		std::cout << strZipFile << std::endl;
		return 0;
	}

	// Determine the size (uncompressed) of all the zip entries to send it to the progress callback
	size_t uTotSize = 0;
	size_t uWrittenBytes = 0;
	struct zip_stat sb;
	std::vector<std::string> optionFiles = { "options.txt", "optionsof.txt", "optionsshaders.txt" };
	for (int i = 0; i < zip_get_num_entries(archive, 0); ++i)
	{
		if (zip_stat_index(archive, i, 0, &sb) == 0) {
			if (!((std::string(sb.name).rfind("bin/", 0) == 0) || ((ResAndOpts[0] == false) && (std::string(sb.name).rfind("resourcepacks", 0) == 0)) || ((ResAndOpts[1] == false) && (std::find(optionFiles.begin(), optionFiles.end(), sb.name) != optionFiles.end())))) {
				uTotSize += sb.size;
			}
		}
	}
	//Extract all items from archive
	uint64_t len;
	for (int i = 0; i < zip_get_num_entries(archive, 0); i++) {
		if (zip_stat_index(archive, i, 0, &sb) == 0) {
			if (!((std::string(sb.name).rfind("bin/", 0) == 0) || ((ResAndOpts[0] == false) && (std::string(sb.name).rfind("resourcepacks", 0) == 0)) || ((ResAndOpts[1] == false) && (std::find(optionFiles.begin(), optionFiles.end(), sb.name) != optionFiles.end())))) {
				len = strlen(sb.name);
				if (sb.name[len - 1] == '/') {
					if (!CreateDirectories(strOutputDirectory + sb.name))
						return 0;
				}
				else {
					zip_file_t* zf = zip_fopen_index(archive, i, 0);
					if (!zf) {
						std::cout << "Failed to open index: " << zip_get_error(archive)->zip_err << std::endl;
						exit(100);
					}
					std::fstream fd;
					fd.exceptions(std::ifstream::badbit);
					try {
						fd.open((strOutputDirectory + std::string(sb.name)).c_str(), std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
					}
					catch (std::ios_base::failure& e) {
						if (fd.bad()) {
							std::cout << "File stream failed to create file '" << sb.name << "'." << std::endl;
							std::cout << "Entire path is: " << (strOutputDirectory + std::string(sb.name)).c_str() << std::endl;
							std::cout << e.what() << std::endl;
							std::cout << e.code() << std::endl;
							std::cout << "Badbit: " << fd.badbit << std::endl;
							std::cout << "failbit: " << fd.failbit << std::endl;
							return 0;
						}
					}

					int sum = 0;
					while (sum != sb.size) {
						len = zip_fread(zf, buf, 64);
						if (len < 0) {
							fprintf(stderr, "boese, boese\n");
							exit(102);
						}
						fd.write(buf, len);
						uWrittenBytes += len;
						sum += len;
						ProgressStrategy(uTotSize, uWrittenBytes, sb.name);
					}
					fd.close();
					zip_fclose(zf);
				}
			}
		}
		else {
			printf("File[%s] Line[%d]\n", __FILE__, __LINE__);
		}
	}
	if (zip_close(archive) == -1) {
		std::cout << "Can't close zip archive '" << archive << "'" << std::endl;;
		return 0;
	}
}

/**
 * @brief checks if the provided path is a directory
 *
 * @param file system path
 *
 * @return boolean
 */
int fileHandling::IsDirectory(const std::string& strPath)
{
	try
	{
		fs::path PathDir(strPath);
		if (fs::exists(PathDir) && fs::is_directory(PathDir))
			return 1;
	}

	catch (const fs::filesystem_error& ex)
	{
		std::cout << ex.what() << std::endl;
	}
	return 0;
}

/**
* @brief checks if the provided path is a file
*
* @param file system path
*
* @return integer, 1 for success, 0 for failure
*/
int fileHandling::IsFile(const std::string& strPath)
{
	try
	{
		fs::path PathFile(strPath);
		if (fs::exists(PathFile) && fs::is_regular_file(PathFile))
			return 1;
	}
	catch (const fs::filesystem_error& ex)
	{
		std::cout << ex.what() << std::endl;
	}
	return 0;
}

/**
* @brief creates a full path
*
* missing directories will be automatically created
*
* parent folders must exist, otherwise an exception will be thrown
*
* @param path to create the new directory
*
* @return success of the operation as an integer
*/
int fileHandling::CreateDirectories(const std::string& strPath) {
	try
	{
		std::string strPathParsed(strPath);
#ifndef __linux__
		if (strPathParsed[strPathParsed.length() - 1] == '\\' || strPathParsed[strPathParsed.length() - 1] == '/')
#else
		if (strPathParsed[strPathParsed.length() - 1] == '/')
#endif
			strPathParsed.erase(strPathParsed.length() - 1);

		fs::path PathDir(strPathParsed);
		if (fs::exists(PathDir) || fs::create_directories(PathDir))
			return 1;
	}
	catch (const fs::filesystem_error& ex)
	{
		std::cout << ex.what() << std::endl;
	}
	return 0;
}

int fileHandling::Extract(const std::string& strOutputDirectory, const std::string& strZipFile, double* Progress, double* TotProgress, std::string* ProgDsc, boost::container::vector<bool> ResAndOpts)
{
	return (!Extractor::ExtractAllFilesFromZip(strOutputDirectory, strZipFile, ResAndOpts, [Progress, TotProgress, ProgDsc](double tot, double done, const char* name) {*Progress = done / tot; *TotProgress = 0.95 + (*Progress * 0.05); *ProgDsc = std::string("Installing Pixel MK 3, Currently extracting: ") + name; }));
}