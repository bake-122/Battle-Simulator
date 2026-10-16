/**
 * @file image_loader.hpp
 * @brief WebP画像のデコードとテクスチャ管理（ImageManager）の定義
 */

#ifndef IMAGE_LOADER_HPP
#define IMAGE_LOADER_HPP

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>
#include <cstddef>
#include <array>

#include "unit_types.h"

/**
 * @brief メモリ上のWebPバイナリからOpenGL 2Dテクスチャを生成する。
 * @param buffer WebPデータ配列へのポインタ
 * @param size データサイズ（バイト）
 * @param out_width テクスチャの幅（px）を出力するポインタ（nullptr可）
 * @param out_height テクスチャの高さ（px）を出力するポインタ（nullptr可）
 * @return 生成されたOpenGLテクスチャID（失敗時は0）
 */
GLuint LoadWebPTextureFromMemory(const unsigned char* buffer, size_t size, int* out_width, int* out_height);

/**
 * @brief キオク、属性、ロールの各種テクスチャを一括管理するシングルトンクラス
 */
class ImageManager {
public:
    /**
     * @brief シングルトンインスタンスを取得する
     * @return ImageManagerの参照
     */
    static ImageManager& GetInstance() {
        static ImageManager instance;
        return instance;
    }

    /** @brief 埋め込みWebP画像をすべてOpenGLテクスチャとして初期化・登録する */
    void Init();
    /**
     * @brief 指定IDのキオク画像テクスチャを取得する
     * @param id キオクID
     * @return OpenGLテクスチャID（未登録時は0）
     */
    GLuint GetTexture(int id) const;
    /**
     * @brief 指定属性のアイコンテクスチャを取得する
     * @param element 属性
     * @return OpenGLテクスチャID
     */
	GLuint GetElementTexture(Element element) const;
    /**
     * @brief 指定ロールのアイコンテクスチャを取得する
     * @param role ロール
     * @return OpenGLテクスチャID
     */
	GLuint GetRoleTexture(Role role) const;
    /** @brief 登録済みテクスチャをすべて破棄・解放する */
    void Cleanup();

    /**
     * @brief 登録されているキオク画像数を取得する
     * @return 登録画像数
     */
    int GetImageCount() const { return static_cast<int>(id_list_.size()); }
    /**
     * @brief インデックスから登録キオクIDを取得する（順序アクセス用）
     * @param index 添字
     * @return キオクID
     */
    int GetIdByIndex(int index) const {
        if (index >= 0 && index < static_cast<int>(id_list_.size())) {
            return id_list_[index];
        }
        return 0;
    }

private:
	/** @brief キオクIDとOpenGLテクスチャのマップ */
    std::unordered_map<int, GLuint> texture_map_;
    /** @brief 登録された全IDを順番に保持するリスト */
    std::vector<int> id_list_;
	
	/** @brief 属性別のアイコンテクスチャマップ */
	std::array<GLuint, 7> element_texture_map_{};
	
	/** @brief ロール別のアイコンテクスチャマップ */
	std::array<GLuint, 7> role_texture_map_{};
};

#endif // IMAGE_LOADER_HPP