#ifndef UNIT_TYPE
#define UNIT_TYPE

#include <cstdint>

// ============================================================
// Enum定義
// ============================================================

enum class Element : uint8_t {
	None, Flame, Forest, Aqua, Light, Dark, Void
};

enum class Role : uint8_t {
	None, Attacker, Breaker, Healer, Buffer, Debuffer, Defender
};

// ============================================================
// Stats (ステータス)
// ============================================================

struct Stats {
	int hp;
	int atk;
	int def;
	
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

struct CharacterStats {
	Stats lv1;
	Stats lv120;
	Stats lv140;
	Stats lv160;
};

// ============================================================
// Actions (forward declaration)
// ============================================================

struct Actions;  // effect_types.h で定義されている

// ============================================================
// Character (メインの構造体)
// ============================================================

struct Character {
	int id;
	const char* name;
	Element element;
	Role role;
	int spd;
	int mp;
	CharacterStats stats;
	const Actions* actions;  // ポインタで参照
	int rarity;
};

#endif // UNIT_TYPE