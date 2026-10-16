/**
 * @file unit_types.h
 * @brief キオクの属性、ロール、ステータス、基本構造体の定義
 */

#ifndef UNIT_TYPE
#define UNIT_TYPE

#include <cstdint>

// ============================================================
// Enum定義
// ============================================================

/**
 * @brief キオクやスキルの属性
 */
enum class Element : uint8_t {
	None,   /**< 未設定 */
	Flame,  /**< 火属性 */
	Forest, /**< 木属性 */
	Aqua,   /**< 水属性 */
	Light,  /**< 光属性 */
	Dark,   /**< 闇属性 */
	Void    /**< 無属性 */
};

/**
 * @brief キオクの戦闘における役割（ロール）
 */
enum class Role : uint8_t {
	None,     /**< 未設定 */
	Attacker, /**< アタッカー */
	Breaker,  /**< ブレイカー */
	Healer,   /**< ヒーラー */
	Buffer,   /**< バッファー */
	Debuffer, /**< デバッファー */
	Defender  /**< ディフェンダー */
};

// ============================================================
// Stats (ステータス)
// ============================================================

/**
 * @brief HP、ATK、DEF を保持する基本ステータス構造体
 */
struct Stats {
	int hp;  /**< ヒットポイント */
	int atk; /**< 攻撃力 */
	int def; /**< 防御力 */
	
	Stats operator+(const Stats& other) const
	{
		return {
			hp  + other.hp,
			atk + other.atk,
			def + other.def
		};
	}

	Stats operator-(const Stats& other) const
	{
		return {
			hp  - other.hp,
			atk - other.atk,
			def - other.def
		};
	}

	Stats& operator+=(const Stats& other)
	{
		hp  += other.hp;
		atk += other.atk;
		def += other.def;

		return *this;
	}

	Stats& operator-=(const Stats& other)
	{
		hp  -= other.hp;
		atk -= other.atk;
		def  -= other.def;

		return *this;
	}
};

/**
 * @brief 各主要レベル（Lv1, 120, 140, 160）におけるキオクの基礎ステータス
 */
struct CharacterStats {
	Stats lv1;   /**< レベル1時ステータス */
	Stats lv120; /**< レベル120時ステータス */
	Stats lv140; /**< レベル140時ステータス */
	Stats lv160; /**< レベル160時ステータス */
};

// ============================================================
// Actions (forward declaration)
// ============================================================

/**
 * @brief スキル・アクション情報の前方宣言
 */
struct Actions;  // effect_types.h で定義されている


/**
  * @brief キオクのマスターデータから必要な情報を格納する。
  *
  * キオクID, 名前, 属性, ロール, SPD, MP,
  * 基礎ステ, 行動, レアリティを保持する。
  */
struct Character {
	int id;                 /**< キオクID */
	const char* name;       /**< キオク名 */
	Element element;        /**< 属性 */
	Role role;              /**< ロール */
	int spd;                /**< 速度（行動値計算の基礎） */
	int mp;                 /**< 最大MP */
	CharacterStats stats;   /**< レベル別基礎ステータス */
	const Actions* actions; /**< アクション定義へのポインタ */
	int rarity;             /**< 初期レアリティ */
};

#endif // UNIT_TYPE