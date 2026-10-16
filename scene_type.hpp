/**
 * @file scene_type.hpp
 * @brief アプリケーションの場面（シーン）種別の定義
 */

#ifndef SCENE_TYPE
#define SCENE_TYPE

/**
 * @brief 画面・場面切り替え用の列挙型
 */
enum class SceneType {
    Formation,   /**< 編成画面 */
    Simulation   /**< 戦闘シミュレーション画面 */
};

#endif