/**
 * @file main.cpp
 * @brief アプリケーションのエントリーポイント、ウィンドウ管理、メインループ
 */

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
#include <fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#endif

/**
 * @brief アプリケーションの二重起動を防止
 *
 * Windowsでは名前付きMutex、macOS/Linuxではflockを使用する。
 * インスタンスが生存している間ロックを保持し、終了時に解放する。
 */
class SingleInstanceGuard
{
public:
	SingleInstanceGuard() noexcept
	{
#if defined(_WIN32)
		SetLastError(ERROR_SUCCESS);
		handle_ = CreateMutexW(nullptr, FALSE, L"Local\\ExedraSim_SingleInstance");
		if (handle_ == nullptr)
		{
			is_primary_ = false;
			return;
		}

		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CloseHandle(handle_);
			handle_ = nullptr;
			is_primary_ = false;
		}
#elif defined(__APPLE__) || defined(__linux__)
		try
		{
			const auto lock_path =
				std::filesystem::temp_directory_path() /
				("ExedraSim-" +
				 std::to_string(static_cast<unsigned long long>(getuid())) +
				 ".lock");

			lock_fd_ = open(lock_path.c_str(), O_CREAT | O_RDWR, 0600);
			if (lock_fd_ < 0 || flock(lock_fd_, LOCK_EX | LOCK_NB) != 0)
			{
				if (lock_fd_ >= 0)
					close(lock_fd_);
				lock_fd_ = -1;
				is_primary_ = false;
			}
		}
		catch (...)
		{
			if (lock_fd_ >= 0)
				close(lock_fd_);
			lock_fd_ = -1;
			is_primary_ = false;
		}
#else
		is_primary_ = false;
#endif
	}

	~SingleInstanceGuard()
	{
#if defined(_WIN32)
		if (handle_ != nullptr)
			CloseHandle(handle_);
#elif defined(__APPLE__) || defined(__linux__)
		if (lock_fd_ >= 0)
		{
			flock(lock_fd_, LOCK_UN);
			close(lock_fd_);
		}
#endif
	}

	SingleInstanceGuard(const SingleInstanceGuard&) = delete;
	SingleInstanceGuard& operator=(const SingleInstanceGuard&) = delete;

	bool IsPrimary() const noexcept
	{
		return is_primary_;
	}

private:
	bool is_primary_ = true;
#if defined(_WIN32)
	HANDLE handle_ = nullptr;
#elif defined(__APPLE__) || defined(__linux__)
	int lock_fd_ = -1;
#endif
};

/**
 * @brief 太字フォント（シミュレーション画面の見出し等で使用）
 */
ImFont* g_font_bold = nullptr;

/**
 * @brief 実行ファイルの配置ディレクトリパスを取得する
 * @return 実行ファイルの親ディレクトリパス
 */
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

/**
 * @brief OSに応じた日本語フォントファイルの絶対パスを取得する
 * @param filename フォントファイル名（meiryo.ttc等）
 * @return フォントファイルのパス
 */
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

/**
 * @brief 現在のシーンに応じてウィンドウタイトルを更新する
 * @param window GLFWウィンドウハンドル
 * @param scene 現在のシーン種別
 */
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


/**
 * @brief エントリーポイント
 * @return 終了コード (0: 正常終了, -1: 異常終了)
 */
int main()
{
	// 最初に二重起動を判定する。
	// 既に起動中、または排他ロックを取得できない場合は終了する。
	SingleInstanceGuard single_instance;
	if (!single_instance.IsPrimary())
		return 0;

	// =========================================================
	// GLFW
	// =========================================================

	if (!glfwInit())
		return -1;


	glfwWindowHint(
		GLFW_RESIZABLE,
		GLFW_FALSE
	);
	
	// macOS用設定。 OpenGLを3.2 Coreに固定 (default: 2.1)
	#if defined(__APPLE__)
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	#endif


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

	// 全 OS で同梱フォントを使用(windowsのみ標準のフォントを検索して表示)
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
	
	// macOS用設定: GLSL 1.3 -> 1.5 (OpenGL 3.2 CoreはGLSL 1.3を受け付けないらしい)
	#if defined(__APPLE__)
		ImGui_ImplOpenGL3_Init("#version 150");
	#else
		ImGui_ImplOpenGL3_Init("#version 130");
	#endif


	// =========================================================
	// ImageManager
	// =========================================================

	ImageManager::GetInstance().Init();


	// =========================================================
	// FormationView
	// =========================================================

	FormationView formationView;
	formationView.initialize();
	
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
			formationView.update();
			current_scene = formationView.draw();
			break;

		case SceneType::Simulation:
			current_scene = renderSimulationScene();
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