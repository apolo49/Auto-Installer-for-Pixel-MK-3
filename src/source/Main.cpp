#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#define STBI_NO_HDR
#include "../vender/stb_image.h"
#include "../headers/Main.h"

uint64_t Main::PUSystemMemory = 0;
//Amount of memory on the system in KB
long Main::MemoryKB = 0;
//Installer thread
std::future<int> Main::Install;
//Clear colour for the background
ImVec4 Main::clear_colour = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
//Logger for the runtime
Logger* Main::log = nullptr;
//Vector to store fonts
std::vector<ImFont*> Main::Fonts = {};
//Pointer for the window
GLFWwindow* Main::window = nullptr;

int Main::Begin() {
	int x, y, n;
	Logger logging = Logger();
	log = &logging;
	glfwSetErrorCallback(glfw_error_callback);
	log->write("Set GLFW error call back.");
	log->write("Attempting to initialise GLFW.");
	if (!glfwInit()) {
		log->write("Could not initialise GLFW.", 3);
		return 1;
	}
	log->write("Initialsed GLFW.");

	// Decide GL+GLSL versions
#if __APPLE__
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
	GetMemory();

	// Create window with graphics context
	log->write("Attempting to create window.");
	window = glfwCreateWindow(1280, 720, "Pixel MK 3 Installer", NULL, NULL);
	if (window == nullptr) {
		log->write("Could not create window.", 3);
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	log->write("Attempting to initialise GLEW.");
	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
	bool err = false;
	glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
	bool err = false;
	glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if (err)
	{
		log->write("Failed to initialize OpenGL loader!", 3);
		return 1;
	}
	log->write("Creating ImGUI context.");
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	log->write("Setting colours.");
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	log->write("Create bindings.");
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	unsigned char* data = stbi_load_from_memory(resource::icon, resource::icon_size, &x, &y, &n, 4);
	GLFWimage Icon;
	Icon.pixels = data;
	Icon.height = y;
	Icon.width = x;

	glfwSetWindowIcon(window, 1, &Icon);
	stbi_image_free(data);

	WindowLoop();
	try {
		return Cleanup();
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
}

/*
* Reads total installed system memory.
* On Windows using built in methods in MSVC and supplied libs.
* On Linux it is grabbed using sysinfo then scaled and reduced.
* On MacOS it is grabbed using sysconf and attempted to be read. Accuracy of this method is being tested.
*/
void Main::GetMemory() {
	log->write("Attempting to read system memory.");
#ifdef _WIN32

	if (GetPhysicallyInstalledSystemMemory(&PUSystemMemory)) {
		MemoryKB = safer_cast<long, uint64_t>(PUSystemMemory);
		log->write("Successfully read system memory.");
	}
	else {
		log->write(std::string("Could not read system memory, error:").append(std::to_string(GetLastError())), 2);
	}

#elif defined(__linux__)
	struct sysinfo si;
	if (sysinfo(&si) == 0) MemoryKB = si.totalram;
	else MemoryKB = 32 * 1024 * 1024 * 1024;
	if (MemoryKB % 1024 != 0) {
		MemoryKB = MemoryKB % 1024 >= 512 ? ((MemoryKB + 1024) - (MemoryKB % 1024)) : (MemoryKB - (MemoryKB % 1024));
	}
	MemoryKB /= 1024;

#elif defined(__APPLE__)
	MemoryKB = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGESIZE);
#endif
}

/*
* Locates installed JREs at expected location.
* On Windows this is C:/Program Files/Java/.../jre/
* On Linux this is /lib/jvm/.../.../jre/
* On Apple I am expecting it to be at /Library/Internet Plug-Ins/JavaAppletPlugin.plugin/Contents/Home/; I'm are still doing research.
*/
std::vector<std::filesystem::directory_entry> Main::JavaDirLocator() {
	log->write("Setting Java location.");
	std::vector<std::filesystem::directory_entry> JavaDirs;
#if defined(_WIN32)
	if (fileHandling::IsDirectory("C:/Program Files/Java")) {
		for (const std::filesystem::directory_entry& p : std::filesystem::directory_iterator("C:/Program Files/Java")) {
			if (p.is_directory() && (p.path().filename().string().find("jre") != std::string::npos)) {
				JavaDirs.push_back(p);
				log->write("Added Java directory " + p.path().string());
			}
		}
	}
#elif defined(__APPLE__)
	std::filesystem::path JavaDir = "/Library/Internet Plug-Ins/JavaAppletPlugin.plugin/Contents/Home/";
	std::filesystem::directory_entry p(JavaDir);
	JavaDirs.push_back(defaultDir);
	log->write("Added Java directory " + p.path().string());
#elif defined(__linux__)
	if (fileHandling::IsDirectory("/lib/jvm/")) {
		for (const std::filesystem::directory_entry& p : std::filesystem::directory_iterator("/lib/jvm/")) {
			if (p.is_directory()) {
				for (const std::filesystem::directory_entry& q : std::filesystem::directory_iterator(p)) {
					if (q.is_directory() && (q.path().filename().string().find("jre") != std::string::npos)) {
						JavaDirs.push_back(p);
						log->write("Added Java directory " + p.path().string());
					}
				}
			}
		}
	}
#endif
	return JavaDirs;
}

/*
* Locates .Minecraft dir at expected location for installed jars and launcher profiles.
*/
std::string Main::MinecraftDirLocator() {
	log->write("Getting .minecraft location.");
#if defined(_WIN32)
	std::string defaultMCDir = "";

	if (fileHandling::IsDirectory(std::string(getenv("APPDATA")).append("/.minecraft/"))) {
		defaultMCDir = std::string(getenv("APPDATA")).append("/.minecraft/");
	}
	else
	{
		log->write("Failed to find .minecraft location.", 1);
	}
#elif defined(__linux__)
	const char* homedir;
	if ((homedir = getenv("HOME")) == NULL)
		homedir = getpwuid(getuid())->pw_dir;
	if (fileHandling::IsDirectory(std::string(getenv("HOME")).append("/.minecraft/"))) {
		defaultMCDir = std::string(getenv("HOME")).append("/.minecraft/");
	}
	else {
		log->write("Failed to find .minecraft location", 1);
	}
#elif defined(_APPLE_)
	defaultMCDir = "~/Library/Application Support/minecraft/";
#endif
	return defaultMCDir;
}

void Main::WindowLoop() {
	log->write("Creating file browsers.");
	ImGui::FileBrowser MCDir(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_NoModal);
	ImGui::FileBrowser PXMKDir(ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir | ImGuiFileBrowserFlags_NoModal);

	CreateFonts();

	bool open = true;
	static char bufMCDir[256];
	strcpy(bufMCDir, MinecraftDirLocator().c_str());

#if SIM_NO_JAVA_DIRS
	log->write("Simulating no Java is installed. Ignore following error.");
	std::vector<std::filesystem::directory_entry> JavaDirs = {};
#else
	log->write("Identifying all Java directories.");
	std::vector<std::filesystem::directory_entry> JavaDirs = JavaDirLocator();
	log->write("Identifying amount of Java directories.");
#endif
	size_t AmtOfJavaDirs = JavaDirs.size();
	log->write("Number of Java Dirs: " + std::to_string(AmtOfJavaDirs));
	if (AmtOfJavaDirs <= 0) {
		log->write("Could not detect any Java Directories, please could you ensure you have installed a 64 bit Java Runtime Environment:\n\tOn Windows this should be at C:/Program Files/Java\n\tOn Linux (Ubuntu) this should be at /lib/jvm/", 2);
	}
	int Chosen = 0;

	static char PixelMKResultDir[256] = "";

	log->write("Checking to see if Forge is installed.");
	//Create options vector consisting of [Do you want ResPack, Do you want default opts, Do you want default JVM args, Do you have forge, how many Java dirs are present]
	boost::container::vector<bool> options = { true, true, true, CheckForge(bufMCDir),  (AmtOfJavaDirs <= 0) ? false : true };
	boost::container::vector<std::string> paths;

	//TEST OPTIONS ONLY, MUST BE APPLIED IN PREPROCESSOR BEFORE COMPILATION
	int MaxMemory;
#if TINY_MEM_MODE
	MaxMemory = 4;

#elif SMALL_MEM_MODE
	MaxMemory = 6;

#elif LARGE_MEM_MODE
	MaxMemory = 32;
#else
	GetMemory();

	if (MemoryKB != 0)
		MaxMemory = ((MemoryKB / 1024) / 1024);
	else
		MaxMemory = 32;
#endif

	int MemoryGB = 6;
	uint8_t Result = 0xFF;

	double Percent = 0;
	double Progress = 0;
	std::string ProgressDesc = "";
	bool UnsafeMode = false;
	bool MainWindow = false;
	bool ModWindow = false;
	char code[18] = "";

	log->write("Entering main loop.");
	while (!glfwWindowShouldClose(window))
	{
		BeginFrame();

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width), static_cast<float>(height)));
		ImGui::SetNextWindowPos(ImVec2(0, 0));

		ImGui::Begin("Pixel MK 3 Installer", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		ImGui::PushFont(Fonts[2]);
		ImGui::Text("Pixel MK 3 Installer");
		ImGui::PopFont();

		ImGui::PushFont(Fonts[1]);
		if ((MaxMemory > 6 || UnsafeMode)) {
			if (((!UnsafeMode) || MainWindow) && MainWindow)
				MainScreen(bufMCDir, MCDir, AmtOfJavaDirs, JavaDirs, Chosen, PixelMKResultDir, PXMKDir, UnsafeMode, MemoryGB, MaxMemory, Percent, Progress,
					Result, options, ProgressDesc, paths, MainWindow);
			else if (ModWindow) {
				ListOfModsWindow(ModWindow);
			}
			else {
				TitleScreen(MainWindow, ModWindow);
			}
		}
		else {
			NotEnoughMemWindow(MaxMemory, code, UnsafeMode, MainWindow);
		}
		EndFrame();
	}
	log->write("Exiting main loop.");
}

/*
* Begin Rendering Frame for the Window. Polls all events and creates a new GL, GLFW and IMGUI frame
*/
void Main::BeginFrame() {
	glfwPollEvents();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Main::ListOfModsWindow(bool& ModWindow) {
	ImGui::Text("This screen is a list of all mods in the modpack.");
	ImGui::Text("All credit for these mods go to the authors and their amazing efforts for the minecraft community.");
	ImGui::Text("Without these amazing people and their creations this modpack would not exist.");
	ImGui::Indent();

	for (int i = 0; i < resource::mods.size(); i++) {
		try {
			ImGui::BulletText(resource::mods[i].at(0).c_str());
			ImGui::Indent();
			ImGui::BulletText(std::string("Author(s): ").append(resource::mods[i].at(1)).c_str());
			ImGui::Unindent();
		}
		catch (std::exception& e) {
			//TODO read last printed line to prevent flooding of log
			log->write(e.what(), 1);
		}
	}

	ImGui::Unindent();
	if (ImGui::Button("back"))
		ModWindow = false;
}

/*
* Title Screen, first screen greeted with when opening unless memory too small.
*/
void Main::TitleScreen(bool& MainWindow, bool& ModWindow) {
	ImGui::Text("Welcome to installing Pixel MK 3.");
	ImGui::Text("Pixel MK 3 is a kitchen sink modpack that includes many popular mods for Java Minecraft including:");
	ImGui::Indent();
	ImGui::BulletText("Aether II");
	ImGui::BulletText("Applied Energistics 2");
	ImGui::BulletText("BiblioCraft");
	ImGui::BulletText("Biomes O Plenty");
	ImGui::BulletText("BuildCraft");
	ImGui::BulletText("Dr Zhark's Mo' Creatures");
	ImGui::BulletText("EnderIO");
	ImGui::BulletText("ExtraUtils 2");
	ImGui::BulletText("Galacticraft");
	ImGui::BulletText("ICBM Classic");
	ImGui::BulletText("Immersive Engineering");
	ImGui::BulletText("Industrial Craft 2");
	ImGui::BulletText("Minecraft Comes Alive!");
	ImGui::BulletText("Mekanism");
	ImGui::BulletText("Morph");
	ImGui::BulletText("Mystcraft");
	ImGui::BulletText("OpenComputers");
	ImGui::BulletText("Project Red");
	ImGui::BulletText("Railcraft");
	ImGui::BulletText("Thaumcraft");
	ImGui::BulletText("AND MANY OTHERS!");
	ImGui::Unindent();
	ImGui::Text("For a full list of mods please select to go to the modlist down below or check the technic page.");
	ImGui::Text("As you can see there are many, many mods, over 150 infact! So this modpack requires a minimum of 9GB of RAM");
	ImGui::Text("to run a world and at least 12GB of RAM to run smoothly on my tests on my computer.");
	ImGui::Text("For my specs please see my steam account.");
	ImGui::Text("This installer will give the smoothest run of any versions of this modpack, if configured correctly.");
	ImGui::Text("Built into this installer are the JVM arguments for a smoother experience with added Garbage Collection Threads.");
	ImGui::Text("If you are seeing this page that means you will definitely have no problems running this modpack.");
	if (ImGui::Button("Configure installation", ImVec2(250, 30)))
		MainWindow = true;
	ImGui::SameLine();
	if (ImGui::Button("List of Mods", ImVec2(250, 30)))
		ModWindow = true;
	if (ImGui::Button("Quit", ImVec2(508, 30))) {
		glfwSetWindowShouldClose(window, true);
	}
}

/*
* Window For when there is not enough memory in the system. Memory will be checked before every update
*/
void Main::NotEnoughMemWindow(int& MaxMemory, char* code, bool& UnsafeMode, bool& MainWindow) {
	ImGui::Text("Your installed memory is less than 10GB.\nYou need at least 6GB to run the modpack and at least 10GB to also run your OS smoothly.");
	if ((MaxMemory) > 5) {
		ImGui::Text("If you have been given a passcode to run this modpack then please enter it below.");
		ImGui::InputText("##Code", code, 18);

		std::string a("5O 78 6C 4d 4b 3E");
		if (!a.compare(code)) {
			UnsafeMode = true;
			MainWindow = true;
		}
	}
	ImGui::PopFont();
	ImGui::PushFont(Fonts[0]);
	ImGui::Text("If you wish to buy more RAM, you can look at these retailers:");
	ImGui::TextURL("Amazon", "https://www.amazon.co.uk/b/?node=430511031&ref_=Oct_s9_apbd_odnav_hd_bw_bT0akp_0&pf_rd_r=BC0NWPQG7Q1PA4BSVDS3&pf_rd_p=d705626f-64c9-52fa-8b95-df0de1496db3&pf_rd_s=merchandised-search-10&pf_rd_t=BROWSE&pf_rd_i=428655031");
	ImGui::TextURL("Box", "https://www.box.co.uk/memory/sort/1/refine/49019~147657$49019~148264$49019~197745$49019~67690$49019~67704");
	ImGui::TextURL("Ebuyer", "https://www.ebuyer.com/store/Components/cat/Memory---PC");
	ImGui::TextURL("Newegg", "https://www.newegg.com/global/uk-en/p/pl?N=101582542%20500000512%20500001024%20500002048%20500004096%20500008192%20500016384&Order=1");
	ImGui::TextURL("Nova Tech", "https://www.novatech.co.uk/products/components/memory-pc/");
	ImGui::TextURL("Overclockers UK", "https://www.overclockers.co.uk/pc-components/memory");
	ImGui::TextURL("Scan", "https://www.scan.co.uk/shop/computer-hardware/memory-ram/all");
	ImGui::Text("I am not sponsored by any of these.");
}

/*
* The main screen set up and execution.
*/
void Main::MainScreen(char* bufMCDir, ImGui::FileBrowser& MCDir, size_t& AmtOfJavaDirs, std::vector<std::filesystem::directory_entry>& JavaDirs,
	int& Chosen, char* PixelMKResultDir, ImGui::FileBrowser& PXMKDir, bool& UnsafeMode, int& MemoryGB, int& MaxMemory, double& Percent, double& Progress,
	uint8_t& Result, boost::container::vector<bool>& options, std::string& ProgressDesc, boost::container::vector<std::string>& paths, bool& MainWindow) {
	ImGui::Text(".minecraft directory");
	ImGui::InputText("##.McDir", bufMCDir, 256, ImGuiInputTextFlags_AutoSelectAll);
	if (ImGui::Button("Choose Dir##MCDir")) {
		MCDir.Open();
	}

#if defined(_WIN32) || defined(__linux__)
	if (AmtOfJavaDirs > 0) {
		ImGui::Text("Choose Java Runtime Environment");
		for (uint8_t i = 0; i < AmtOfJavaDirs; i++) {
			ImGui::RadioButton(JavaDirs.at(i).path().filename().string().c_str(), &Chosen, i);
			if (i != AmtOfJavaDirs - 1) {
				ImGui::SameLine();
			}
		}
	}
	else {
		ImGui::Text("You do not have a Java Runtime Environment. You can download Java here:");
		ImGui::TextURL("java.com", "https://java.com/en/download/manual.jsp");
	}
#endif

	ImGui::Text("Where to place Pixel MK 3?");
	ImGui::InputText("##PixelMKDir", PixelMKResultDir, 256);
	if (ImGui::Button("Choose Dir##PXMKDir")) {
		PXMKDir.Open();
	}

	ImGui::Text("How much memory do you wish to give the modpack (in GB)?");
	if (UnsafeMode) {
		ImGui::PopFont();
		ImGui::PushFont(Fonts[2]);
		ImGui::Text("UNSAFE MODE IS ACTIVATED DO BE AWARE YOUR\nCOMPUTER WILL SLOW DOWN CONSIDERABLY\nIF YOU DO NOT LEAVE MORE THAN 2GB TO RUN THE OS");
		ImGui::PopFont();
		ImGui::PushFont(Fonts[1]);
		ImGui::SliderInt("##Memory", &MemoryGB, 6, MaxMemory);
	}
	else
		ImGui::SliderInt("##Memory2", &MemoryGB, 6, MaxMemory - 4);

	ImGui::Checkbox("Do you want the PureBDCraft Resourcepack that pairs with this modpack?", &options[0]);
	ImGui::Checkbox("Do you want the default option settings for this modpack?", &(options[1]));
	ImGui::Checkbox("Do you want the best JVM arguments for performance installed to the modpack?", &options[2]);

	if ((MaxMemory - 4) > 5)
		ImGui::Checkbox("Unsafe mode?", &(UnsafeMode));

	if (!options[3]) {
		ImGui::Text("The correct version of Forge has not been installed. The installer will install this for you.");
	}
	if (Percent == 0.0f) {
		if (strlen(PixelMKResultDir) != 0 && strlen(bufMCDir) != 0 && AmtOfJavaDirs > 0) {
			if (ImGui::Button("Install")) {
				Result = -1;
				Install = std::async(CreateProfile::Begin, options, paths, MemoryGB, &Percent, &Progress, &ProgressDesc);
			}
		}
	}
	if (Install.valid()) {
		auto status = Install.wait_for(std::chrono::milliseconds(0));
		if (status == std::future_status::ready) {
			Result = Install.get();
		}
		else {
			ImGui::ProgressBar(Percent);
			if (Progress != 0.0f) {
				ImGui::Text(ProgressDesc.c_str());
				ImGui::ProgressBar(Progress);
			}
		}
	}

	if (Result == 4) {
		ImGui::Text("The install failed due to forge being unable to install. Are you connected to Wi-Fi?");
	}
	else if (Result == 3) {
		ImGui::Text("The install failed due to the Pixel MK Directory being unable to be created. Did you enter the path correctly?");
	}
	else if (Result == 2) {
		ImGui::Text("The install failed due to an issue with writing to launcher_profiles.json. Did you enter the /.minecraft directory correctly?");
	}
	else if (Result == 1) {
		ImGui::Text("The install failed due to being unable to download the modpack or install the modpack.");
	}
	else if (Result == 0) {
		ImGui::ProgressBar(1);
		ImGui::Text("Done.");
		Percent = 0.0f;
		Progress = 0.0f;
	}

	MCDir.Display();

	if (MCDir.HasSelected())
	{
		strcpy(bufMCDir, MCDir.GetSelected().string().c_str());
		MCDir.ClearSelected();
	}

	PXMKDir.Display();
	if (PXMKDir.HasSelected())
	{
		strcpy(PixelMKResultDir, PXMKDir.GetSelected().string().c_str());
		PXMKDir.ClearSelected();
	}

	if ((MaxMemory - 4) > 8) {
		if (ImGui::Button("Back")) {
			MainWindow = false;
		}
	}

	ImGui::PopFont();
	ImGui::PushFont(Fonts[0]);
	//Update options array, [Do you want resource pack, Do you want in game options, Do you want JVM args, Is Forge Installed]
	options[3] = CheckForge(bufMCDir);
	//Check for Java Dirs
	if (JavaDirs.size() != 0) {
		paths = { PixelMKResultDir, bufMCDir,
#if defined(_WIN32)
			JavaDirs.at(Chosen).path().string().append("/bin/javaw.exe")
#elif defined(__APPLE__)
			JavaDirs.at(Chosen).path().string()
#elif defined(__linux__)
			JavaDirs.at(Chosen).path().string().append("/jre/bin/java")
#endif
		};
	}
	else {
		paths = { PixelMKResultDir, bufMCDir,"" };
	}
}

/*
* End Frame Render.
* Gets Display dimensions and Creates the framebuffer and viewport, sets the clear colour and swaps buffers.
*/
void Main::EndFrame() {
	int display_w, display_h;
	std::time_t currentTime = std::time(NULL);

	ImGui::PopFont();
	ImGui::PushFont(Fonts[1]);
	ImGui::Text(std::ctime(&currentTime));
	ImGui::PopFont();
	ImGui::PushFont(Fonts[0]);
	ImGui::Text("Made by Joe Targett, if you have any problems please contact me @ Joe.Targett@outlook.com");
	ImGui::PopFont();
	ImGui::End();
	ImGui::Render();
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_colour.x, clear_colour.y, clear_colour.z, clear_colour.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
}

/*
* Creates the fonts for use in the window
*/
void Main::CreateFonts() {
	log->write("Grabbing io settings.");
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL;
	log->write("Creating fonts.");
	//Creates the small font as a base for the other fonts and for use in the footer and links
	ImFontConfig LtlFont_cfg;
	LtlFont_cfg.FontDataOwnedByAtlas = false;
	LtlFont_cfg.FontData = resource::NexaSlab;
	LtlFont_cfg.FontDataSize = 77028;

	//Sets standard and title font equal to the little font
	ImFontConfig StdFont_cfg = LtlFont_cfg;
	ImFontConfig TtlFont_cfg = LtlFont_cfg;

	//Resizes all the fonts for appropriate use
	LtlFont_cfg.SizePixels = 12.5;
	StdFont_cfg.SizePixels = 25;
	TtlFont_cfg.SizePixels = 50;

	log->write("Adding fonts.");
	//Adds references to the fonts to vector for storage and use
	Fonts.push_back(io.Fonts->AddFont(&LtlFont_cfg));
	Fonts.push_back(io.Fonts->AddFont(&StdFont_cfg));
	Fonts.push_back(io.Fonts->AddFont(&TtlFont_cfg));
}

/*
* Checks for already installed forge (1.12.2 14.23.5.2854) by checking for the JSON and JAR and then the JSON seperately so it can be downloaded.
*/
bool Main::CheckForge(std::string MCDir) {
	if (fileHandling::IsFile(MCDir + "versions/1.12.2-forge-14.23.5.2854/1.12.2-forge-14.23.5.2854.json") &&
		fileHandling::IsFile(MCDir + "versions/1.12.2-forge-14.23.5.2854/1.12.2-forge-14.23.5.2854.jar") ||
		fileHandling::IsFile(MCDir + "versions/1.12.2-forge-14.23.5.2854/1.12.2-forge-14.23.5.2854.json")) {
		return true;
	}
	else {
		return false;
	}
}