#include "image_loader.hpp"
#include "webp/decode.h"

#include "embedded_images/units/all_unit_images.h"
#include "embedded_images/elements/all_element_images.h"
#include "embedded_images/roles/all_role_images.h"

// LoadWebPTextureFromMemory の実装
GLuint LoadWebPTextureFromMemory(const unsigned char* buffer, size_t size, int* out_width, int* out_height) {
    if (!buffer || size == 0) return 0;

    int width = 0, height = 0;
    uint8_t* rgba = WebPDecodeRGBA(buffer, size, &width, &height);
    if (!rgba) return 0;

    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

    WebPFree(rgba);

    if (out_width) *out_width = width;
    if (out_height) *out_height = height;

    return image_texture;
}

void ImageManager::Init() {
	
	// アイコン画像の登録
    auto RegisterImage = [this](int id, const unsigned char* data, size_t size) {
        int w, h;
        GLuint tex = LoadWebPTextureFromMemory(data, size, &w, &h);
        if (tex != 0) {
            texture_map_[id] = tex;
            id_list_.push_back(id); // IDをリストに記録
        }
    };
#include "embedded_images/units/register_unit_images.h"

	// 属性画像の登録
	auto RegisterElementImage = [this](Element element, const unsigned char* data, size_t size) {
		int w, h;
		GLuint tex = LoadWebPTextureFromMemory(data, size, &w, &h);
		if(tex != 0) {
			element_texture_map_[static_cast<size_t>(element)] = tex;
		}
	};
#include "embedded_images/elements/register_element_images.h"

	// ロール画像の登録
	auto RegisterRoleImage = [this](Role role, const unsigned char* data, size_t size) {
		int w, h;
		GLuint tex = LoadWebPTextureFromMemory(data, size, &w, &h);
		if(tex != 0) {
			role_texture_map_[static_cast<size_t>(role)] = tex;
		}
	};
#include "embedded_images/roles/register_role_images.h"
}

// アイコンのgetter
GLuint ImageManager::GetTexture(int id) const {
    auto it = texture_map_.find(id);
    if (it != texture_map_.end()) {
        return it->second;
    }
    return 0;
}

// 属性のgetter
GLuint ImageManager::GetElementTexture(Element element) const {
	const size_t index = static_cast<size_t>(element);
	
	// 範囲外
	if (index >= element_texture_map_.size()) return 0;
	
	return element_texture_map_[index];
}

// ロールのgetter
GLuint ImageManager::GetRoleTexture(Role role) const {
	const size_t index = static_cast<size_t>(role);
	
	// 範囲外
	if (index >= role_texture_map_.size()) return 0;
	
	return role_texture_map_[index];
}


void ImageManager::Cleanup() {
	
	// アイコン
    for (auto& pair : texture_map_) {
        if (pair.second != 0) {
            glDeleteTextures(1, &pair.second);
        }
    }
    texture_map_.clear();
    id_list_.clear();
	
	// 属性
	for (GLuint& texture : element_texture_map_) {
		if (texture != 0) {
			glDeleteTextures(1, &texture);
			texture = 0;
		}
	}
	
	// ロール
	for (GLuint& texture : role_texture_map_) {
		if (texture != 0) {
			glDeleteTextures(1, &texture);
			texture = 0;
		}
	}
}