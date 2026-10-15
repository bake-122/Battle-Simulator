#ifndef IMAGE_LOADER_HPP
#define IMAGE_LOADER_HPP

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>
#include <cstddef>
#include <array>

#include "unit_types.h"

GLuint LoadWebPTextureFromMemory(const unsigned char* buffer, size_t size, int* out_width, int* out_height);

class ImageManager {
public:
    static ImageManager& GetInstance() {
        static ImageManager instance;
        return instance;
    }

    void Init();
    GLuint GetTexture(int id) const;
	GLuint GetElementTexture(Element element) const;
	GLuint GetRoleTexture(Role role) const;
    void Cleanup();

    // 順序アクセス用に追加
    int GetImageCount() const { return static_cast<int>(id_list_.size()); }
    int GetIdByIndex(int index) const {
        if (index >= 0 && index < static_cast<int>(id_list_.size())) {
            return id_list_[index];
        }
        return 0;
    }

private:
	// アイコン用
    std::unordered_map<int, GLuint> texture_map_;
    std::vector<int> id_list_; // 登録された全IDを順番に保持
	
	// 属性用
	std::array<GLuint, 7> element_texture_map_{};
	
	// ロール用
	std::array<GLuint, 7> role_texture_map_{};
};

#endif // IMAGE_LOADER_HPP