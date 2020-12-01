#include "../headers/CreateProfile.h"

int CreateProfile::Begin(boost::container::vector<bool> options, boost::container::vector<std::string> paths, int Memory, double* Percent, double* Progress, std::string* ProgressDesc) {
	if (!options[3])
		if (InstallForge(Progress, ProgressDesc, Percent)) {
			*Progress = 0.0f;
			return 4;
		}
	*Progress = 0.0f;
	if (!std::filesystem::is_directory(paths[0])) {
		if (CreatePxMKDir(paths[0]))
			return 3;
	}
	*Percent = 0.25f;
	if (AddProfileToProfiles(paths[1], options[2], options[4], Memory, paths[0], paths[2]))
		return 2;
	*Percent = 0.3f;
	if (!GrabPack(Progress, paths[0], ProgressDesc, Percent) && !InstallPack(paths[0], Progress, ProgressDesc, Percent, boost::container::vector<bool> {options[0], options[1]}))
		return 1;
	return 0;
}

int CreateProfile::InstallForge(double* Progress, std::string* ProgressDesc, double* Percent) {
	*ProgressDesc = "Downloading Minecraft Forge";
	if (Downloader::GetFile("files.minecraftforge.net/maven/net/minecraftforge/forge/1.12.2-14.23.5.2854/forge-1.12.2-14.23.5.2854-installer.jar", "forge-1.12.2-14.23.5.2854-installer.jar", Progress, Percent, 0.0f, 0.2))
		return 1;
	try {
#ifdef _WIN32
		LPSTARTUPINFOW si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));

		LPWSTR cmdline = new wchar_t[56];
		wcscpy(cmdline, L"javaw.exe -jar forge-1.12.2-14.23.5.2854-installer.jar");
		//run the process
		if (CreateProcessW(NULL, cmdline, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, si, &pi) == 0) {
			delete[] cmdline;
			remove("forge-1.12.2-14.23.5.2854-installer.jar");
			std::cout << "Failed to run forge installer:\n" << GetLastError() << std::endl;
			return 1;
		}
		delete[] cmdline;
		//wait until exit and cleanup thread and handles
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
#else
		system("java -jar forge-1.12.2-14.23.5.2854-installer.jar");
#endif
		//Remove the installer
		fs::remove("forge-1.12.2-14.23.5.2854-installer.jar");
		fs::remove("installer.log");
		return 0;
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
}

int CreateProfile::CreatePxMKDir(std::string PxMKDir) {
	try {
		return !std::filesystem::create_directories(PxMKDir);
	}
	catch (std::exception) {
		return 0;
	}
}

int CreateProfile::AddProfileToProfiles(std::string MCDir, bool JVMArgs, bool IsJavaInstalled, int Memory, std::string PxMKDir, std::string JavaPath) {
	std::string JSONDir;
	std::string::size_type n = 0;
	std::string replace = "\\";
	while ((n = JavaPath.find(replace, n)) != std::string::npos)
	{
		JavaPath.replace(n, replace.size(), "/");
		n += sizeof("/");
	}

	if (MCDir.at(MCDir.length() - 1) == '/' || MCDir.at(MCDir.length() - 1) == '\\') {
		JSONDir = MCDir + "launcher_profiles.json";
	}
	else {
		JSONDir = MCDir + "/launcher_profiles.json";
	}

	if (PxMKDir.at(PxMKDir.length() - 1) == '/' || PxMKDir.at(PxMKDir.length() - 1) == '\\') {
		PxMKDir.at(PxMKDir.length() - 1) = ' ';
	}

	while ((n = JSONDir.find(replace, n)) != std::string::npos)
	{
		JSONDir.replace(n, replace.size(), "/");
		n += sizeof("/");
	}

	while ((n = PxMKDir.find(replace, n)) != std::string::npos)
	{
		PxMKDir.replace(n, replace.size(), "/");
		n += sizeof("/");
	}

	if (!std::filesystem::exists(JSONDir))
		return 1;

	// CREATING JSON CODE FOR PROFILE

	time_t rawtime;
	time(&rawtime);
	char buffer[80];
	strftime(buffer, 80, "%Y-%m-%dT%T.000z", localtime(&rawtime));

	json profile;
	profile["Pixel MK 3"]["created"] = buffer;
	profile["Pixel MK 3"]["gameDir"] = PxMKDir;
	profile["Pixel MK 3"]["icon"] = PIXEL_MK_3_ICON;
	if (!JVMArgs) {
		profile["Pixel MK 3"]["javaArgs"] = "-Xmx" + std::to_string(Memory * 1024) + "M -XX:+UnlockExperimentalVMOptions -XX:+UseG1GC -XX:G1NewSizePercent=20 -XX:G1ReservePercent=20 -XX:MaxGCPauseMillis=50 -XX:G1HeapRegionSize=32M";
	}
	else {
		profile["Pixel MK 3"]["javaArgs"] = "-Xmx" + std::to_string(Memory * 1024) + "M -d64 -server -XX:+AggressiveOpts -XX:ParallelGCThreads=3 -XX:+UseConcMarkSweepGC -XX:+UnlockExperimentalVMOptions -XX:+UseParNewGC -XX:+ExplicitGCInvokesConcurrent -XX:MaxGCPauseMillis=10 -XX:GCPauseIntervalMillis=50 -XX:+UseFastAccessorMethods -XX:+OptimizeStringConcat -XX:NewSize=84m -XX:+UseAdaptiveGCBoundary -XX:NewRatio=3 -Dfml.readTimeout=90 -Ddeployment.trace=true -Ddeployment.log=true -Ddeployment.trace.level=all";
	}
	if (IsJavaInstalled)
		profile["Pixel MK 3"]["javaDir"] = JavaPath;
	profile["Pixel MK 3"]["lastVersionId"] = "1.12.2-forge-14.23.5.2854";
	profile["Pixel MK 3"]["name"] = "Pixel MK 3";
	profile["Pixel MK 3"]["type"] = "custom";

	//FINISHED CREATION OF JSON CODE FOR FILE

	json profiles;
	std::ifstream file(JSONDir, std::ifstream::binary);

	file >> profiles;

	try {
		profiles["profiles"]["Pixel MK 3"] = profile["Pixel MK 3"];
		std::ofstream file(JSONDir, std::ofstream::binary);
		file << profiles;
	}
	catch (std::exception) {
		return 1;
	}

	return 0;
}

int CreateProfile::GrabPack(double* Progress, std::string PxMKDir, std::string* ProgressDesc, double* percent) {
	*ProgressDesc = "Downloading Pixel MK 3";
	if (PxMKDir.at(PxMKDir.length() - 1) == '/' || PxMKDir.at(PxMKDir.length() - 1) == '\\') {
		PxMKDir.append("Pixel-MK-3.zip");
	}
	else {
		PxMKDir.append("/Pixel-MK-3.zip");
	}
	if (Downloader::GetFile("https://pixel-mk-3.s3.eu-west-2.amazonaws.com/Pixel+MK+3.zip", PxMKDir.c_str(), Progress, percent, 0.3, 0.65))
		return 1;
	return 0;
}

int CreateProfile::InstallPack(std::string PxMKDir, double* Progress, std::string* ProgressDesc, double* Percent, boost::container::vector<bool> ResAndOpts) {
	size_t uUnzippedFilesCount = 0;
	if (PxMKDir.at(PxMKDir.length() - 1) == '/' || PxMKDir.at(PxMKDir.length() - 1) == '\\') {
		if (fileHandling::Extract(PxMKDir, PxMKDir + "Pixel-MK-3.zip", Progress, Percent, ProgressDesc, ResAndOpts)) {
			fs::remove(PxMKDir + "Pixel-MK-3.zip");
			return 1;
		}
		fs::remove(PxMKDir + "Pixel-MK-3.zip");
		return 0;
	}
	else {
		if (fileHandling::Extract(PxMKDir, PxMKDir + "/Pixel-MK-3.zip", Progress, Percent, ProgressDesc, ResAndOpts)) {
			fs::remove(PxMKDir + "/Pixel-MK-3.zip");
			return 1;
		}
		fs::remove(PxMKDir + "/Pixel-MK-3.zip");
		return 0;
	}
	return 1;
}