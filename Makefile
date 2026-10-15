# ==========================================
# コンパイラ・各種設定
# ==========================================
CXX    = g++
TARGET = ExedraSim.exe

# インクルードディレクトリ
INCLUDES = -I. -I./imgui -I./glfw/include

# ライブラリ
LIBDIRS = -L./lib -L./glfw/lib
LIBS    = -lwebp -lglfw3 -lopengl32 -lgdi32


# ==========================================
# ビルドモード
# ==========================================
BUILD ?= debug

BUILD_DIR = build/$(BUILD)
TARGET_PATH = $(BUILD_DIR)/$(TARGET)


# ==========================================
# Debug / Release 設定
# ==========================================
ifeq ($(BUILD),debug)

    # --------------------------------------
    # Debug
    # 各 cpp を個別にコンパイル
    # --------------------------------------
    CXXFLAGS = -O0 -g -Wall $(INCLUDES) -MMD -MP
    LDFLAGS  = $(LIBDIRS) $(LIBS)

    SRCS = \
        imgui/imgui.cpp \
        imgui/imgui_draw.cpp \
        imgui/imgui_tables.cpp \
        imgui/imgui_widgets.cpp \
        imgui/imgui_impl_glfw.cpp \
        imgui/imgui_impl_opengl3.cpp \
        image_loader.cpp \
        formation/formationView.cpp \
        simulation/simulationView.cpp \
        simulation/simulationLogic.cpp \
        simulation/EffectList.cpp \
        simulation/ProcessList.cpp \
        main.cpp

else

    # --------------------------------------
    # Release
    # Unity Build
    # --------------------------------------
    CXXFLAGS = -O2 -Wall $(INCLUDES) -MMD -MP
    LDFLAGS  = $(LIBDIRS) $(LIBS) \
               -static \
               -static-libgcc \
               -static-libstdc++ \
               -mwindows

    SRCS = \
        imgui/imgui_impl_glfw.cpp \
        imgui/imgui_impl_opengl3.cpp \
        unity.cpp

endif


# ==========================================
# オブジェクトファイル
# ==========================================
OBJS = $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))

# Windows アイコンリソース (.rc → .o)
RESOURCE_OBJ = $(BUILD_DIR)/app_icon.o
OBJS += $(RESOURCE_OBJ)

# 依存関係ファイル
DEPS = $(OBJS:.o=.d)


# ==========================================
# ターゲット
# ==========================================
.PHONY: all debug release run run-debug run-release clean clean-debug clean-release

all: $(TARGET_PATH)

# ==========================================
# 実行
# ==========================================
run:
	$(MAKE) BUILD=debug
	build/debug/ExedraSim.exe

run-debug:
	$(MAKE) BUILD=debug
	build/debug/ExedraSim.exe

run-release:
	$(MAKE) BUILD=release
	build/release/ExedraSim.exe


# ==========================================
# ビルドディレクトリ作成
# ==========================================
$(BUILD_DIR):
	mkdir -p "$(BUILD_DIR)"


# ==========================================
# リンク
# ==========================================
$(TARGET_PATH): $(OBJS) | $(BUILD_DIR)
	$(CXX) -o $@ $^ $(LDFLAGS)


# ==========================================
# Windows リソース (.rc → .o)
# ==========================================
$(RESOURCE_OBJ): resources/app_icon.rc images/icon.ico | $(BUILD_DIR)
	windres resources/app_icon.rc -O coff -o $@


# ==========================================
# cpp → o
# ==========================================
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p "$(dir $@)"
	$(CXX) $(CXXFLAGS) -c $< -o $@


# ==========================================
# 自動生成された依存関係を読み込む
# ==========================================
-include $(DEPS)


# ==========================================
# ビルドモード切り替え
# ==========================================
debug:
	$(MAKE) BUILD=debug

release:
	$(MAKE) BUILD=release


# ==========================================
# クリーン
# ==========================================
clean-debug:
	rm -rf build/debug

clean-release:
	rm -rf build/release

clean:
	rm -rf build