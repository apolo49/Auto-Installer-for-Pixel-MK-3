#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <tchar.h>
#ifdef _WIN32
#include <shlwapi.h>
#endif
#include <iostream>
#include "../vender/json.hpp"
#include <time.h>
#include <thread>
#include <future>
#include <atomic>
#include "Downloader.h"
using json = nlohmann::json;

class CreateProfile
{
public:
	static int Begin(bool options[4], std::string PxMKDir, std::string MCDir, int Memory, std::string JavaPath, std::atomic<float>* Percent, std::atomic<float>* Progress);

private:
#ifdef _WIN32
	static int InstallForge(std::atomic<float>* Progress);
	static int GrabPack(std::atomic<float>* Percent);
#endif
	static int CreatePxMKDir(std::string PxMKDir);
	static int AddProfileToProfiles(std::string MCDir, bool JVMArgs, int Memory, std::string PxMKDir, std::string JavaPath);
	static int InstallPack(std::string PxMKDir);
};