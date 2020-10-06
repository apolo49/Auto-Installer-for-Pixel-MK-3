#include "../headers/CreateProfile.h"

int CreateProfile::Begin(bool options[4], std::string PxMKDir, std::string MCDir, int Memory, std::string JavaPath, std::atomic<float>* Percent, std::atomic<float>* Progress) {
	Percent->store(0.1);
	if (!options[3])
		if (InstallForge(Progress)) {
			Progress->store(0.0);
			return 4;
		}
	Progress->store(0.0);
	Percent->store(0.2);
	if (!std::filesystem::is_directory(PxMKDir)) {
		if (CreatePxMKDir(PxMKDir))
			return 3;
	}
	Percent->store(0.25);
	if (AddProfileToProfiles(MCDir, options[2], Memory, PxMKDir, JavaPath))
		return 2;
	Percent->store(0.3);
	if (GrabPack(Percent) || InstallPack(PxMKDir))
		return 1;
	return 0;
}

int CreateProfile::InstallForge(std::atomic<float>* Progress) {
	/*if (Downloader::GetFile("forge-1.12.2-14.23.5.2854-installer.jar", "files.minecraftforge.net/maven/net/minecraftforge/forge/1.12.2-14.23.5.2854/forge-1.12.2-14.23.5.2854-installer.jar", Progress))
		return 1;
		*/
	try {
		system("javaw -jar forge-1.12.2-14.23.5.2854-installer.jar");
		remove("forge-1.12.2-14.23.5.2854-installer.jar");
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

int CreateProfile::AddProfileToProfiles(std::string MCDir, bool JVMArgs, int Memory, std::string PxMKDir, std::string JavaPath) {
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
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	char buffer[80];
	strftime(buffer, 80, "%Y-%m-%dT%T.000z", timeinfo);

	json profile;
	profile["Pixel MK 3"]["created"] = buffer;
	profile["Pixel MK 3"]["gameDir"] = PxMKDir;
	profile["Pixel MK 3"]["icon"] = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAMAAAD04JH5AAAABGdBTUEAALGOfPtRkwAAACBjSFJNAACHDwAAjA8AAP1SAACBQAAAfXkAAOmLAAA85QAAGcxzPIV3AAADAFBMVEUAAAARERESEhIUFBQVFRUWFhYXFxcYGBgZGRkaGhobGxscHBwdHR0eHh4fHx8gICAhISEiIiIjIyMkJCQlJSUmJiYnJycoKCgpKSkqKiorKyssLCwtLS0uLi4vLy8wMDAxMTEyMjIzMzM0NDQ1NTU2NjY3Nzc4ODg5OTk6Ojo7Ozs8PDw9PT0+Pj4/Pz9AQEBBQUFCQkJDQ0NERERFRUVGRkZHR0dISEhJSUlKSkpLS0tMTExNTU1OTk5PT09QUFBRUVFSUlJTU1NUVFRVVVVWVlZXV1dYWFhZWVlaWlpbW1tcXFxdXV1eXl5fX19gYGBhYWFiYmJjY2NkZGRlZWVmZmZnZ2doaGhpaWlqampra2tsbGxtbW1ubm5vb29wcHBxcXFycnJzc3N0dHR1dXV2dnZ3d3d4eHh5eXl6enp7e3t8fHx9fX1+fn5/f3+AgICBgYGCgoKDg4OEhISFhYWGhoaHh4eIiIiJiYmKioqLi4uMjIyNjY2Ojo6Pj4+QkJCRkZGSkpKTk5OUlJSVlZWWlpaXl5eYmJiZmZmampqbm5ucnJydnZ2enp6fn5+goKChoaGioqKjo6OkpKSlpaWmpqanp6eoqKipqamqqqqrq6usrKytra2urq6vr6+wsLCxsbGysrKzs7O0tLS1tbW2tra3t7e4uLi5ubm6urq7u7u8vLy9vb2+vr6/v7/Z2dnq6ur19fX7+/v+/v7///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA2MGaoAAABAHRSTlP///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////8AU/cHJQAAAAlwSFlzAAAOwwAADsMBx2+oZAAAAAd0SU1FB94IAxQ6AY+k1pMAAAAZdEVYdFNvZnR3YXJlAHBhaW50Lm5ldCA0LjAuMjHxIGmVAAAQeUlEQVR4XtWb91ca2xbH33t5KVeT2BURS9SbRM3NtcUeIyYqGhWJEQtIFQRBUKRIE0VBiiBgAcEGlliSvPr/zZvtjC/GaHLvT04+a4XMnDkz+7tO3Wef41+QG+ZnFNDWVldXVvbiBX6L8v59czOHY7Xit3+Kn09ANNrT4/FsborFJBKehLx5s7Cg0XC5ra2hEJ70h/k5BHC57e34JbKywmJtb29teb1lZa2tkKJQ6HQbKBoNhVJYeJbpT0B8Af391dWTkwJBURF2b7WOjkaju7s7Oz09FAqktLUFAiEUnS4n5/Hjs0xIe3tDg0KBXX8f4gugUndRgsHBwdev4V6jUSgwARMTWVkIEgoJBFABoZBen5OTlwd5mEwu12gcGKitjUTg/nsQX0BpaTAYi0Wj8/OVlRoNgggENlv0jPX1iYnOTqnU6dzcxATk5xcUIIhSKRB4vW6309ne/vjxzAz+oWsgvgAGw2LZ24uhaLXPn1OpExO7u7EYVAuI2N3d2oqghMMbG4FAY2NJiUolFi8teb0ej9VaWBgX96OmSEwBUmlHB36JzM+LRHt70WgstrcXDm9vg1EQBObhensbBEQiGxvBoN+vVisUS0t+vw8FGmlCAv6ZayGiAD5fLJ6cLCqansbu6XQwt4cCFQFC9vb2UTARIGFzE6pgbW19PRhcWfH7Ayh+/+Bgaupvv2Hf4HDCYezqMsQTMDw8MgIG3G6hsL4ehmKDAUxDGiZga8vlUqvF4u7u1laBQKnUau32YBCqIBQK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";
	if (!JVMArgs) {
		profile["Pixel MK 3"]["javaArgs"] = "-Xmx" + std::to_string(Memory) + "G -XX:+UnlockExperimentalVMOptions -XX:+UseG1GC -XX:G1NewSizePercent=20 -XX:G1ReservePercent=20 -XX:MaxGCPauseMillis=50 -XX:G1HeapRegionSize=32M";
	}
	else {
		profile["Pixel MK 3"]["javaArgs"] = "-Xmx" + std::to_string(Memory) + "G -d64 -server -XX:+AggressiveOpts -XX:ParallelGCThreads=3 -XX:+UseConcMarkSweepGC -XX:+UnlockExperimentalVMOptions -XX:+UseParNewGC -XX:+ExplicitGCInvokesConcurrent -XX:MaxGCPauseMillis=10 -XX:GCPauseIntervalMillis=50 -XX:+UseFastAccessorMethods -XX:+OptimizeStringConcat -XX:NewSize=84m -XX:+UseAdaptiveGCBoundary -XX:NewRatio=3 -Dfml.readTimeout=90 -Ddeployment.trace=true -Ddeployment.log=true -Ddeployment.trace.level=all";
	}
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

int CreateProfile::GrabPack(std::atomic<float>* Progress) {
	float link = 0;
	if (Downloader::GetFile("https://www.dropbox.com/s/z8yh8ddp6zw8si2/Pixel%20MK%203.zip?dl=1", "Pixel MK 3.zip", &link))
		return 1;
	return 0;
}

int CreateProfile::InstallPack(std::string PxMKDir) {
	return 0;
}