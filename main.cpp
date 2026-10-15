#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

#include "image_loader.hpp"
#include "formation/formationView.h"
#include "simulation/simulationView.hpp"
#include "window_config.h"
#include "scene_type.hpp"

#include <filesystem>
#include <string>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

// 太字フォント（simulationView.hpp で extern 宣言）
ImFont* g_font_bold = nullptr;

// fontファイルをwindows非依存にするため。
static std::filesystem::path GetExecutableDirectory()
{
#if defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string buffer(size, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) != 0)
        return std::filesystem::current_path();
    return std::filesystem::weakly_canonical(std::filesystem::path(buffer.c_str())).parent_path();
#elif defined(_WIN32)
    std::wstring buffer(32768, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size())
        return std::filesystem::current_path();
    buffer.resize(length);
    return std::filesystem::path(buffer).parent_path();
#elif defined(__linux__)
    std::string buffer(4096, '\0');
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length <= 0)
        return std::filesystem::current_path();
    buffer.resize(static_cast<size_t>(length));
    return std::filesystem::path(buffer).parent_path();
#else
    return std::filesystem::current_path();
#endif
}

static std::filesystem::path GetJapaneseFontPath(const char* filename)
{
#if defined(__APPLE__)
    return GetExecutableDirectory().parent_path() / "Resources" / filename;
#elif defined(_WIN32)
    return std::filesystem::path("C:/Windows/Fonts") / filename;
#else
    return GetExecutableDirectory() / "resources" / filename;
#endif
}

// ウィンドウタイトル変更関数
void UpdateWindowTitle(GLFWwindow* window, SceneType scene)
{
	switch (scene)
	{
	case SceneType::Formation:
		glfwSetWindowTitle(
			window,
			"Exedra Sim (Formation)"
		);
		break;

	case SceneType::Simulation:
		glfwSetWindowTitle(
			window,
			"Exedra Sim (Simulation)"
		);
		break;
	}
}


int main()
{
	// =========================================================
	// GLFW
	// =========================================================

	if (!glfwInit())
		return -1;


	glfwWindowHint(
		GLFW_RESIZABLE,
		GLFW_FALSE
	);


	GLFWwindow* window =
		glfwCreateWindow(
			WINDOW_WIDTH,
			WINDOW_HEIGHT,
			"Exedra Sim (Formation)",
			NULL,
			NULL
		);


	if (!window)
	{
		glfwTerminate();
		return -1;
	}


	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);


	// =========================================================
	// ImGui
	// =========================================================

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGuiIO& io =
		ImGui::GetIO();

	io.IniFilename = nullptr;

	ImGui::StyleColorsDark();


	// =========================================================
	// 日本語フォント
	// =========================================================

	// 全 OS で同梱フォントを使用する。実行時の作業ディレクトリには依存しない。
	const std::filesystem::path regular_font_path = GetJapaneseFontPath("meiryo.ttc");
	const std::string regular_font_path_string = regular_font_path.string();

	ImFont* font = io.Fonts->AddFontFromFileTTF(
		regular_font_path_string.c_str(),
		18.0f,
		nullptr,
		io.Fonts->GetGlyphRangesJapanese()
	);

	if (font == nullptr)
	{
		font = io.Fonts->AddFontDefault();
	}

	// 太字
	const std::filesystem::path bold_font_path = GetJapaneseFontPath("meiryob.ttc");
	const std::string bold_font_path_string = bold_font_path.string();
	g_font_bold = io.Fonts->AddFontFromFileTTF(
		bold_font_path_string.c_str(),
		24.0f,
		nullptr,
		io.Fonts->GetGlyphRangesJapanese()
	);
	if (g_font_bold == nullptr)
		g_font_bold = font;

	// =========================================================
	// ImGui Backend
	// =========================================================

	ImGui_ImplGlfw_InitForOpenGL(
		window,
		true
	);

	ImGui_ImplOpenGL3_Init(
		"#version 130"
	);


	// =========================================================
	// ImageManager
	// =========================================================

	ImageManager::GetInstance().Init();


	// =========================================================
	// FormationView
	// =========================================================

	FormationView formationView;
	formationView.Initialize();
	
	SceneType current_scene = SceneType::Formation;


	// =========================================================
	// メインループ
	// =========================================================

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();


		// -----------------------------------------------------
		// ImGui Frame開始
		// -----------------------------------------------------

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		
		SceneType previous_scene = current_scene;
		switch (current_scene)
		{
		case SceneType::Formation:
			formationView.Update();
			current_scene = formationView.Draw();
			break;

		case SceneType::Simulation:
			current_scene = RenderSimulationScene();
			break;
		}
		
		if (current_scene != previous_scene) UpdateWindowTitle(window, current_scene);


		// =====================================================
		// ImGui Render
		// =====================================================

		ImGui::Render();


		int display_w;
		int display_h;

		glfwGetFramebufferSize(
			window,
			&display_w,
			&display_h
		);


		glViewport(
			0,
			0,
			display_w,
			display_h
		);


		glClearColor(
			0.15f,
			0.15f,
			0.15f,
			1.0f
		);


		glClear(
			GL_COLOR_BUFFER_BIT
		);


		ImGui_ImplOpenGL3_RenderDrawData(
			ImGui::GetDrawData()
		);


		glfwSwapBuffers(window);
	}


	// =========================================================
	// 終了処理
	// =========================================================

	ImageManager::GetInstance().Cleanup();


	ImGui_ImplOpenGL3_Shutdown();

	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();


	glfwDestroyWindow(window);

	glfwTerminate();


	return 0;
}