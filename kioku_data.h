/**
 * @file kioku_data.h
 * @brief 全キオク（キャラクター）の静的マスターデータ定義
 */

#ifndef KIOKU_DATA
#define KIOKU_DATA
#include <array>
#include <algorithm>
#include "unit_types.h"

/**
 * @brief 全キオクの静的マスターデータ配列
 */
inline constexpr std::array<Character, 128> Kioku_data = {
	{
		{
			.id = 1001,
			.name = "ルクス☆マギカ",
			.element = Element::Light,
			.role = Role::Breaker,
			.spd = 107,
			.mp = 75,
			.stats = {
				.lv1   = {306, 87, 102},
				.lv120 = {5499, 1566, 1835},
				.lv140 = {5907, 1740, 2039},
				.lv160 = {6314, 1914, 2243}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 1002,
			.name = "プルウィア☆ノイヤー",
			.element = Element::Light,
			.role = Role::Buffer,
			.spd = 122,
			.mp = 100,
			.stats = {
				.lv1   = {328, 97, 109},
				.lv120 = {5902, 1751, 1967},
				.lv140 = {6339, 1946, 2186},
				.lv160 = {6777, 2141, 2405}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 1003,
			.name = "プルウィア☆クララ",
			.element = Element::Aqua,
			.role = Role::Healer,
			.spd = 107,
			.mp = 100,
			.stats = {
				.lv1   = {385, 81, 107},
				.lv120 = {6923, 1462, 1917},
				.lv140 = {7436, 1624, 2130},
				.lv160 = {7948, 1786, 2343}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 1004,
			.name = "プルウィア☆マギカ",
			.element = Element::Light,
			.role = Role::Breaker,
			.spd = 114,
			.mp = 75,
			.stats = {
				.lv1   = {326, 91, 107},
				.lv120 = {5864, 1629, 1922},
				.lv140 = {6299, 1810, 2136},
				.lv160 = {6733, 1991, 2350}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 1005,
			.name = "ウィンクティオ☆マギカ",
			.element = Element::Light,
			.role = Role::Debuffer,
			.spd = 122,
			.mp = 0,
			.stats = {
				.lv1   = {309, 109, 104},
				.lv120 = {5562, 1962, 1872},
				.lv140 = {5974, 2180, 2080},
				.lv160 = {6379, 2398, 2288}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 2001,
			.name = "時間停止攻撃",
			.element = Element::Void,
			.role = Role::Buffer,
			.spd = 116,
			.mp = 75,
			.stats = {
				.lv1   = {314, 100, 101},
				.lv120 = {5643, 1807, 1822},
				.lv140 = {6061, 2008, 2024},
				.lv160 = {6479, 2209, 2226}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 2002,
			.name = "ミサイルによる集中砲火",
			.element = Element::Dark,
			.role = Role::Attacker,
			.spd = 98,
			.mp = 90,
			.stats = {
				.lv1   = {263, 133, 85},
				.lv120 = {4725, 2399, 1535},
				.lv140 = {5075, 2666, 1706},
				.lv160 = {5425, 2933, 1877}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 2003,
			.name = "黒い魔力による制圧",
			.element = Element::Void,
			.role = Role::Attacker,
			.spd = 100,
			.mp = 150,
			.stats = {
				.lv1   = {299, 126, 91},
				.lv120 = {5373, 2259, 1638},
				.lv140 = {5771, 2510, 1820},
				.lv160 = {6169, 2761, 2002}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 2004,
			.name = "キュゥべえボムによる爆破攻撃",
			.element = Element::Dark,
			.role = Role::Debuffer,
			.spd = 123,
			.mp = 75,
			.stats = {
				.lv1   = {303, 124, 91},
				.lv120 = {5454, 2232, 1638},
				.lv140 = {5858, 2480, 1820},
				.lv160 = {6262, 2728, 2002}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 2008,
			.name = "金属体への変換と投擲",
			.element = Element::Flame,
			.role = Role::Attacker,
			.spd = 105,
			.mp = 0,
			.stats = {
				.lv1 = {282, 137, 85},
				.lv120 = {5071, 2468, 1532},
				.lv140 = {5446, 2742, 1702},
				.lv160 = {5822, 3016, 1872}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 3001,
			.name = "ティロ・フィナーレ",
			.element = Element::Forest,
			.role = Role::Buffer,
			.spd = 120,
			.mp = 75,
			.stats = {
				.lv1   = {310, 100, 103},
				.lv120 = {5584, 1804, 1845},
				.lv140 = {5997, 2004, 2050},
				.lv160 = {6411, 2204, 2255}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 3002,
			.name = "ベベ・オー・ランタン",
			.element = Element::Dark,
			.role = Role::Debuffer,
			.spd = 111,
			.mp = 75,
			.stats = {
				.lv1   = {306, 119, 95},
				.lv120 = {5508, 2142, 1710},
				.lv140 = {5916, 2380, 1900},
				.lv160 = {6324, 2618, 2090}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 3003,
			.name = "フィオレ・フィナーレ",
			.element = Element::Forest,
			.role = Role::Attacker,
			.spd = 96,
			.mp = 100,
			.stats = {
				.lv1   = {296, 130, 87},
				.lv120 = {5330, 2340, 1570},
				.lv140 = {5725, 2600, 1744},
				.lv160 = {6119, 2860, 1918}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 3004,
			.name = "パンナ・ヴォルティコーザ",
			.element = Element::Forest,
			.role = Role::Healer,
			.spd = 109,
			.mp = 90,
			.stats = {
				.lv1   = {379, 81, 109},
				.lv120 = {6815, 1462, 1953},
				.lv140 = {7320, 1624, 2170},
				.lv160 = {7824, 1786, 2387}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 3005,
			.name = "ご招待のドッペル",
			.element = Element::Light,
			.role = Role::Breaker,
			.spd = 106,
			.mp = 90,
			.stats = {
				.lv1   = {320, 92, 108},
				.lv120 = {5751, 1654, 1937},
				.lv140 = {6177, 1838, 2152},
				.lv160 = {6603, 2022, 2367}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 3006,
			.name = "スピエタータ・スカーリカ",
			.element = Element::Forest,
			.role = Role::Defender,
			.spd = 96,
			.mp = 75,
			.stats = {
				.lv1   = {317, 70, 140},
				.lv120 = {5702, 1265, 2520},
				.lv140 = {6125, 1406, 2800},
				.lv160 = {6547, 1547, 3080}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 4001,
			.name = "バルダメンテ・フォルティッシモ",
			.element = Element::Aqua,
			.role = Role::Defender,
			.spd = 84,
			.mp = 75,
			.stats = {
				.lv1   = {319, 68, 132},
				.lv120 = {5735, 1215, 2380},
				.lv140 = {6160, 1350, 2644},
				.lv160 = {6584, 1485, 2908}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 4002,
			.name = "メロディア・アパッショナータ",
			.element = Element::Aqua,
			.role = Role::Debuffer,
			.spd = 106,
			.mp = 90,
			.stats = {
				.lv1   = {312, 118, 94},
				.lv120 = {5616, 2124, 1692},
				.lv140 = {6032, 2360, 1880},
				.lv160 = {6448, 2596, 2068}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 4003,
			.name = "ボンナターレ・グラツィオーソ",
			.element = Element::Aqua,
			.role = Role::Buffer,
			.spd = 121,
			.mp = 75,
			.stats = {
				.lv1   = {332, 96, 110},
				.lv120 = {5967, 1719, 1980},
				.lv140 = {6409, 1910, 2200},
				.lv160 = {6851, 2101, 2420}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 4004,
			.name = "グランディオーソ・シンフォニア",
			.element = Element::Aqua,
			.role = Role::Healer,
			.spd = 110,
			.mp = 90,
			.stats = {
				.lv1   = {363, 96, 99},
				.lv120 = {6534, 1732, 1780},
				.lv140 = {7018, 1924, 1978},
				.lv160 = {7502, 2116, 2176}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 5001,
			.name = "盟神抉槍",
			.element = Element::Flame,
			.role = Role::Attacker,
			.spd = 86,
			.mp = 90,
			.stats = {
				.lv1   = {265, 131, 87},
				.lv120 = {4601, 2432, 1544},
				.lv140 = {5121, 2620, 1736},
				.lv160 = {5475, 2882, 1910}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 5002,
			.name = "真夏のファイヤーアタック！",
			.element = Element::Flame,
			.role = Role::Buffer,
			.spd = 121,
			.mp = 75,
			.stats = {
				.lv1   = {334, 97, 108},
				.lv120 = {6010, 1737, 1948},
				.lv140 = {6455, 1930, 2164},
				.lv160 = {6901, 2123, 2380}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 5003,
			.name = "聖夜捧呈",
			.element = Element::Flame,
			.role = Role::Debuffer,
			.spd = 106,
			.mp = 75,
			.stats = {
				.lv1   = {303, 121, 94},
				.lv120 = {5454, 2178, 1692},
				.lv140 = {5858, 2420, 1880},
				.lv160 = {6262, 2662, 2068}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 5004,
			.name = "紅珠焼菓",
			.element = Element::Flame,
			.role = Role::Breaker,
			.spd = 112,
			.mp = 75,
			.stats = {
				.lv1   = {343, 91, 111},
				.lv120 = {6167, 1643, 1989},
				.lv140 = {6624, 1826, 2210},
				.lv160 = {7080, 2009, 2431}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 6001,
			.name = "魔法のケーキドーム",
			.element = Element::Light,
			.role = Role::Healer,
			.spd = 109,
			.mp = 90,
			.stats = {
				.lv1   = {407, 74, 97},
				.lv120 = {7322, 1323, 1746},
				.lv140 = {7865, 1470, 1940},
				.lv160 = {8407, 1617, 2134}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 7001,
			.name = "ホロウ・ウーマン",
			.element = Element::Light,
			.role = Role::Buffer,
			.spd = 119,
			.mp = 75,
			.stats = {
				.lv1   = {329, 93, 104},
				.lv120 = {5924, 1665, 1868},
				.lv140 = {6363, 1850, 2076},
				.lv160 = {6801, 2035, 2284}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 7002,
			.name = "解はデジャ・ブ",
			.element = Element::Void,
			.role = Role::Breaker,
			.spd = 117,
			.mp = 75,
			.stats = {
				.lv1   = {324, 100, 103},
				.lv120 = {5832, 1800, 1854},
				.lv140 = {6264, 2000, 2060},
				.lv160 = {6696, 2200, 2266}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 8001,
			.name = "ストラーダ・フトゥーロ",
			.element = Element::Light,
			.role = Role::Breaker,
			.spd = 112,
			.mp = 75,
			.stats = {
				.lv1   = {320, 93, 106},
				.lv120 = {5767, 1672, 1912},
				.lv140 = {6194, 1858, 2124},
				.lv160 = {6622, 2044, 2336}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 8002,
			.name = "沈黙のドッペル",
			.element = Element::Dark,
			.role = Role::Healer,
			.spd = 102,
			.mp = 75,
			.stats = {
				.lv1   = {340, 101, 92},
				.lv120 = {6113, 1822, 1651},
				.lv140 = {6566, 2024, 1834},
				.lv160 = {7018, 2226, 2017}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 8003,
			.name = "エヴォルツィオーネ・プレゼンテ",
			.element = Element::Aqua,
			.role = Role::Attacker,
			.spd = 102,
			.mp = 150,
			.stats = {
				.lv1   = {284, 134, 87},
				.lv120 = {5114, 2414, 1571},
				.lv140 = {5493, 2682, 1746},
				.lv160 = {5871, 2950, 1921}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 9001,
			.name = "アブソリュート・レイン",
			.element = Element::Aqua,
			.role = Role::Attacker,
			.spd = 92,
			.mp = 90,
			.stats = {
				.lv1   = {257, 135, 86},
				.lv120 = {4622, 2423, 1544},
				.lv140 = {4965, 2692, 1716},
				.lv160 = {5307, 2961, 1888}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 9003,
			.name = "アブソリュート・ヴィーナス",
			.element = Element::Aqua,
			.role = Role::Breaker,
			.spd = 114,
			.mp = 75,
			.stats = {
				.lv1   = {336, 92, 112},
				.lv120 = {6048, 1658, 2016},
				.lv140 = {6496, 1842, 2240},
				.lv160 = {6944, 2026, 2464}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 10001,
			.name = "炎扇斬舞",
			.element = Element::Flame,
			.role = Role::Buffer,
			.spd = 120,
			.mp = 75,
			.stats = {
				.lv1   = {311, 97, 106},
				.lv120 = {5589, 1742, 1901},
				.lv140 = {6003, 1936, 2112},
				.lv160 = {6417, 2130, 2323}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 10002,
			.name = "ウワサの雷扇痺舞",
			.element = Element::Flame,
			.role = Role::Defender,
			.spd = 87,
			.mp = 75,
			.stats = {
				.lv1   = {297, 72, 135},
				.lv120 = {5346, 1298, 2430},
				.lv140 = {5742, 1442, 2700},
				.lv160 = {6138, 1586, 2970}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 11001,
			.name = "フォルターゲフェングニス",
			.element = Element::Forest,
			.role = Role::Defender,
			.spd = 83,
			.mp = 75,
			.stats = {
				.lv1   = {307, 68, 135},
				.lv120 = {5530, 1228, 2435},
				.lv140 = {5939, 1364, 2706},
				.lv160 = {6349, 1500, 2977}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 12001,
			.name = "ウルトラグレートビッグハンマー",
			.element = Element::Dark,
			.role = Role::Debuffer,
			.spd = 122,
			.mp = 95,
			.stats = {
				.lv1   = {330, 110, 86},
				.lv120 = {5940, 1980, 1548},
				.lv140 = {6380, 2200, 1720},
				.lv160 = {6820, 2420, 1892}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 13001,
			.name = "アサルトパラノイア",
			.element = Element::Dark,
			.role = Role::Breaker,
			.spd = 112,
			.mp = 90,
			.stats = {
				.lv1   = {321, 97, 107},
				.lv120 = {5778, 1744, 1930},
				.lv140 = {6206, 1938, 2144},
				.lv160 = {6634, 2132, 2358}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 14001,
			.name = "エッジオブユニヴァース",
			.element = Element::Flame,
			.role = Role::Attacker,
			.spd = 86,
			.mp = 120,
			.stats = {
				.lv1   = {256, 135, 86},
				.lv120 = {4612, 2428, 1541},
				.lv140 = {4953, 2698, 1712},
				.lv160 = {5295, 2968, 1883}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 15001,
			.name = "インフィニットポセイドン",
			.element = Element::Aqua,
			.role = Role::Breaker,
			.spd = 108,
			.mp = 75,
			.stats = {
				.lv1   = {321, 92, 107},
				.lv120 = {5783, 1660, 1922},
				.lv140 = {6212, 1844, 2136},
				.lv160 = {6640, 2028, 2350}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 16001,
			.name = "ジャッジメントアース",
			.element = Element::Forest,
			.role = Role::Healer,
			.spd = 107,
			.mp = 90,
			.stats = {
				.lv1   = {391, 69, 107},
				.lv120 = {7409, 1174, 1863},
				.lv140 = {7552, 1384, 2130},
				.lv160 = {8072, 1522, 2343}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 17001,
			.name = "パープルウィルオーウィスプ",
			.element = Element::Flame,
			.role = Role::Breaker,
			.spd = 93,
			.mp = 85,
			.stats = {
				.lv1   = {288, 83, 97},
				.lv120 = {5123, 1446, 1688},
				.lv140 = {5559, 1658, 1935},
				.lv160 = {5943, 1824, 2129}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 17002,
			.name = "かりんのキオク",
			.element = Element::Flame,
			.role = Role::Breaker,
			.spd = 114,
			.mp = 0,
			.stats = {
				.lv1   = {288, 84, 96},
				.lv120 = {5132, 1457, 1672},
				.lv140 = {5570, 1670, 1917},
				.lv160 = {5954, 1837, 2109}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 18001,
			.name = "竜真螺旋咆撃",
			.element = Element::Aqua,
			.role = Role::Attacker,
			.spd = 99,
			.mp = 90,
			.stats = {
				.lv1   = {232, 120, 79},
				.lv120 = {4127, 2085, 1372},
				.lv140 = {4479, 2390, 1573},
				.lv160 = {4788, 2629, 1731}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 18002,
			.name = "明日香のキオク",
			.element = Element::Aqua,
			.role = Role::Attacker,
			.spd = 94,
			.mp = 0,
			.stats = {
				.lv1   = {233, 121, 77},
				.lv120 = {4156, 2102, 1347},
				.lv140 = {4510, 2410, 1544},
				.lv160 = {4821, 2651, 1699}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 19001,
			.name = "ネオ・ジェネシス☆彡",
			.element = Element::Flame,
			.role = Role::Breaker,
			.spd = 118,
			.mp = 75,
			.stats = {
				.lv1   = {326, 96, 107},
				.lv120 = {5864, 1669, 1886},
				.lv140 = {6299, 1854, 2196},
				.lv160 = {6733, 2039, 2416}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 20001,
			.name = "創造の子どもたち",
			.element = Element::Forest,
			.role = Role::Debuffer,
			.spd = 121,
			.mp = 90,
			.stats = {
				.lv1   = {348, 110, 85},
				.lv120 = {5821, 1735, 1832},
				.lv140 = {6177, 1816, 2276},
				.lv160 = {6603, 1998, 2504}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 21001,
			.name = "Nine Phases",
			.element = Element::Void,
			.role = Role::Breaker,
			.spd = 111,
			.mp = 75,
			.stats = {
				.lv1   = {321, 98, 106},
				.lv120 = {5773, 1757, 1915},
				.lv140 = {6200, 1952, 2128},
				.lv160 = {6628, 2147, 2341}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 21002,
			.name = "Marigold Dadaism",
			.element = Element::Flame,
			.role = Role::Attacker,
			.spd = 98,
			.mp = 90,
			.stats = {
				.lv1   = {304, 123, 92},
				.lv120 = {5465, 2205, 1661},
				.lv140 = {5870, 2450, 1846},
				.lv160 = {6274, 2695, 2031}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 22001,
			.name = "ルーチェ・スペランツァー",
			.element = Element::Dark,
			.role = Role::Buffer,
			.spd = 119,
			.mp = 100,
			.stats = {
				.lv1   = {308, 100, 104},
				.lv120 = {5535, 1791, 1872},
				.lv140 = {5945, 1990, 2080},
				.lv160 = {6355, 2189, 2288}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 23001,
			.name = "断罪の光芒",
			.element = Element::Void,
			.role = Role::Attacker,
			.spd = 96,
			.mp = 90,
			.stats = {
				.lv1   = {261, 134, 85},
				.lv120 = {4693, 2410, 1535},
				.lv140 = {5040, 2678, 1706},
				.lv160 = {5388, 2946, 1877}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 24001,
			.name = "絶対自壊演舞",
			.element = Element::Void,
			.role = Role::Healer,
			.spd = 107,
			.mp = 90,
			.stats = {
				.lv1   = {381, 71, 108},
				.lv120 = {6858, 1278, 1946},
				.lv140 = {7366, 1420, 2162},
				.lv160 = {7874, 1562, 2378}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 25001,
			.name = "桜隠れ",
			.element = Element::Dark,
			.role = Role::Breaker,
			.spd = 108,
			.mp = 75,
			.stats = {
				.lv1   = {323, 96, 108},
				.lv120 = {5805, 1730, 1935},
				.lv140 = {6235, 1922, 2150},
				.lv160 = {6665, 2114, 2365}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 26001,
			.name = "桜語り",
			.element = Element::Light,
			.role = Role::Debuffer,
			.spd = 122,
			.mp = 75,
			.stats = {
				.lv1   = {345, 101, 90},
				.lv120 = {6210, 1818, 1620},
				.lv140 = {6670, 2020, 1800},
				.lv160 = {7130, 2222, 1980}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 27001,
			.name = "夏希のキオク",
			.element = Element::Aqua,
			.role = Role::Buffer,
			.spd = 119,
			.mp = 0,
			.stats = {
				.lv1   = {275, 90, 93},
				.lv120 = {4897, 1578, 1628},
				.lv140 = {5314, 1809, 1867},
				.lv160 = {5680, 1990, 2053}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 27501,
			.name = "アトモ・アラビアート",
			.element = Element::Forest,
			.role = Role::Debuffer,
			.spd = 120,
			.mp = 75,
			.stats = {
				.lv1   = {322, 99, 100},
				.lv120 = {5794, 1778, 1800},
				.lv140 = {6223, 1976, 2000},
				.lv160 = {6653, 2174, 2200}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 28001,
			.name = "白椿",
			.element = Element::Forest,
			.role = Role::Attacker,
			.spd = 97,
			.mp = 90,
			.stats = {
				.lv1   = {235, 121, 76},
				.lv120 = {4180, 2112, 1330},
				.lv140 = {4536, 2421, 1525},
				.lv160 = {4849, 2663, 1677}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 28002,
			.name = "ななかのキオク",
			.element = Element::Forest,
			.role = Role::Buffer,
			.spd = 116,
			.mp = 0,
			.stats = {
				.lv1   = {276, 91, 93},
				.lv120 = {4911, 1589, 1614},
				.lv140 = {5012, 2182, 1670},
				.lv160 = {5139, 2400, 1837}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 28501,
			.name = "あーしのヤバめなハート",
			.element = Element::Flame,
			.role = Role::Healer,
			.spd = 102,
			.mp = 90,
			.stats = {
				.lv1   = {384, 71, 107},
				.lv120 = {6912, 1278, 1928},
				.lv140 = {7424, 1420, 2142},
				.lv160 = {7936, 1562, 2356}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 29001,
			.name = "かこのキオク",
			.element = Element::Forest,
			.role = Role::Attacker,
			.spd = 93,
			.mp = 0,
			.stats = {
				.lv1   = {231, 121, 78},
				.lv120 = {4113, 2107, 1353},
				.lv140 = {4463, 2416, 1552},
				.lv160 = {4771, 2657, 1707}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 30001,
			.name = "美雨のキオク",
			.element = Element::Aqua,
			.role = Role::Breaker,
			.spd = 113,
			.mp = 0,
			.stats = {
				.lv1   = {287, 83, 96},
				.lv120 = {5113, 1455, 1678},
				.lv140 = {5549, 1669, 1924},
				.lv160 = {5932, 1835, 2117}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 31001,
			.name = "サークル・オブ・ファイア",
			.element = Element::Flame,
			.role = Role::Healer,
			.spd = 109,
			.mp = 90,
			.stats = {
				.lv1   = {364, 61, 93},
				.lv120 = {6484, 1058, 1630},
				.lv140 = {7037, 1213, 1868},
				.lv160 = {7522, 1335, 2055}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 31002,
			.name = "れいらのキオク",
			.element = Element::Flame,
			.role = Role::Breaker,
			.spd = 114,
			.mp = 0,
			.stats = {
				.lv1   = {291, 82, 96},
				.lv120 = {5176, 1432, 1681},
				.lv140 = {6400, 1462, 1838},
				.lv160 = {6841, 1608, 2022}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 32001,
			.name = "ダイヤ・スプラッシュ",
			.element = Element::Aqua,
			.role = Role::Breaker,
			.spd = 113,
			.mp = 75,
			.stats = {
				.lv1   = {289, 82, 97},
				.lv120 = {5152, 1438, 1686},
				.lv140 = {5591, 1649, 1933},
				.lv160 = {5976, 1814, 2127}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 32002,
			.name = "せいかのキオク",
			.element = Element::Aqua,
			.role = Role::Breaker,
			.spd = 111,
			.mp = 0,
			.stats = {
				.lv1   = {291, 82, 97},
				.lv120 = {5185, 1426, 1686},
				.lv140 = {5627, 1634, 1933},
				.lv160 = {6015, 1798, 2127}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 33001,
			.name = "フリソソグミドリノハ",
			.element = Element::Forest,
			.role = Role::Breaker,
			.spd = 110,
			.mp = 75,
			.stats = {
				.lv1   = {290, 82, 96},
				.lv120 = {5171, 1433, 1683},
				.lv140 = {5612, 1643, 1930},
				.lv160 = {5999, 1808, 2123}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 33002,
			.name = "みとのキオク",
			.element = Element::Forest,
			.role = Role::Debuffer,
			.spd = 116,
			.mp = 0,
			.stats = {
				.lv1   = {275, 91, 93},
				.lv120 = {4897, 1594, 1614},
				.lv140 = {5575, 1647, 1940},
				.lv160 = {5959, 1812, 2134}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 34001,
			.name = "ディスクリート・レーザー",
			.element = Element::Light,
			.role = Role::Defender,
			.spd = 85,
			.mp = 120,
			.stats = {
				.lv1   = {277, 63, 120},
				.lv120 = {4930, 1094, 2099},
				.lv140 = {5351, 1255, 2407},
				.lv160 = {5720, 1380, 2647}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 34002,
			.name = "こころのキオク",
			.element = Element::Light,
			.role = Role::Defender,
			.spd = 87,
			.mp = 0,
			.stats = {
				.lv1   = {279, 61, 121},
				.lv120 = {4969, 1064, 2116},
				.lv140 = {5392, 1220, 2426},
				.lv160 = {5764, 1342, 2669}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 35001,
			.name = "ナイトメア・スティンガー",
			.element = Element::Dark,
			.role = Role::Debuffer,
			.spd = 118,
			.mp = 75,
			.stats = {
				.lv1   = {274, 93, 92},
				.lv120 = {4882, 1614, 1598},
				.lv140 = {5298, 1850, 1832},
				.lv160 = {5664, 2035, 2016}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 35002,
			.name = "帆奈のキオク",
			.element = Element::Dark,
			.role = Role::Breaker,
			.spd = 113,
			.mp = 0,
			.stats = {
				.lv1   = {288, 84, 96},
				.lv120 = {5132, 1457, 1674},
				.lv140 = {5309, 1850, 1829},
				.lv160 = {5675, 2035, 2012}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 36001,
			.name = "メテオ・フィスト",
			.element = Element::Flame,
			.role = Role::Debuffer,
			.spd = 120,
			.mp = 75,
			.stats = {
				.lv1   = {279, 88, 94},
				.lv120 = {4964, 1542, 1642},
				.lv140 = {5387, 1768, 1883},
				.lv160 = {5759, 1944, 2071}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 36002,
			.name = "ひみかのキオク",
			.element = Element::Flame,
			.role = Role::Buffer,
			.spd = 120,
			.mp = 0,
			.stats = {
				.lv1   = {277, 89, 94},
				.lv120 = {4940, 1548, 1645},
				.lv140 = {5361, 1775, 1886},
				.lv160 = {5731, 1952, 2075}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 37001,
			.name = "ソウル・サルベーション",
			.element = Element::Void,
			.role = Role::Debuffer,
			.spd = 117,
			.mp = 75,
			.stats = {
				.lv1   = {304, 103, 102},
				.lv120 = {5465, 1850, 1838},
				.lv140 = {5870, 2056, 2042},
				.lv160 = {6274, 2262, 2246}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 37002,
			.name = "セイクリッド・ギフト",
			.element = Element::Aqua,
			.role = Role::Breaker,
			.spd = 111,
			.mp = 75,
			.stats = {
				.lv1   = {347, 91, 109},
				.lv120 = {6242, 1643, 1966},
				.lv140 = {6705, 1826, 2184},
				.lv160 = {7167, 2009, 2402}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 38001,
			.name = "このはのキオク",
			.element = Element::Aqua,
			.role = Role::Healer,
			.spd = 109,
			.mp = 0,
			.stats = {
				.lv1   = {366, 62, 92},
				.lv120 = {6522, 1075, 1598},
				.lv140 = {7078, 1233, 1832},
				.lv160 = {7566, 1356, 2016}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 39001,
			.name = "サンダー・トレント",
			.element = Element::Light,
			.role = Role::Buffer,
			.spd = 116,
			.mp = 75,
			.stats = {
				.lv1   = {277, 89, 94},
				.lv120 = {4940, 1546, 1645},
				.lv140 = {5361, 1773, 1886},
				.lv160 = {5731, 1950, 2075}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 39002,
			.name = "葉月のキオク",
			.element = Element::Light,
			.role = Role::Debuffer,
			.spd = 116,
			.mp = 0,
			.stats = {
				.lv1   = {281, 88, 93},
				.lv120 = {5007, 1543, 1628},
				.lv140 = {5434, 1769, 1867},
				.lv160 = {5809, 1946, 2053}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 40001,
			.name = "未確認飛行ファイヤー",
			.element = Element::Flame,
			.role = Role::Debuffer,
			.spd = 118,
			.mp = 75,
			.stats = {
				.lv1   = {272, 94, 91},
				.lv120 = {4848, 1634, 1587},
				.lv140 = {5262, 1874, 1820},
				.lv160 = {5625, 2061, 2002}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 40002,
			.name = "あやめのキオク",
			.element = Element::Flame,
			.role = Role::Buffer,
			.spd = 118,
			.mp = 0,
			.stats = {
				.lv1   = {281, 90, 92},
				.lv120 = {5007, 1573, 1598},
				.lv140 = {5434, 1804, 1832},
				.lv160 = {5809, 1984, 2016}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 41001,
			.name = "インビジブル・アサシン",
			.element = Element::Void,
			.role = Role::Attacker,
			.spd = 98,
			.mp = 90,
			.stats = {
				.lv1   = {233, 122, 76},
				.lv120 = {4146, 2123, 1327},
				.lv140 = {4500, 2434, 1521},
				.lv160 = {4810, 2677, 1673}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 41002,
			.name = "まさらのキオク",
			.element = Element::Light,
			.role = Role::Attacker,
			.spd = 97,
			.mp = 0,
			.stats = {
				.lv1   = {235, 120, 77},
				.lv120 = {4180, 2101, 1338},
				.lv140 = {4536, 2408, 1534},
				.lv160 = {4849, 2649, 1687}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 42001,
			.name = "キラ盛りビーム",
			.element = Element::Flame,
			.role = Role::Buffer,
			.spd = 119,
			.mp = 75,
			.stats = {
				.lv1   = {279, 89, 94},
				.lv120 = {4969, 1546, 1634},
				.lv140 = {5392, 1773, 1874},
				.lv160 = {5764, 1950, 2061}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 42002,
			.name = "梨花のキオク",
			.element = Element::Flame,
			.role = Role::Attacker,
			.spd = 96,
			.mp = 0,
			.stats = {
				.lv1   = {231, 121, 78},
				.lv120 = {4113, 2107, 1356},
				.lv140 = {4985, 2154, 1735},
				.lv160 = {5329, 2261, 1909}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 42003,
			.name = "キラハピ！スノードーム",
			.element = Element::Flame,
			.role = Role::Healer,
			.spd = 110,
			.mp = 100,
			.stats = {
				.lv1   = {370, 84, 109},
				.lv120 = {6653, 1516, 1953},
				.lv140 = {7146, 1684, 2170},
				.lv160 = {7638, 1852, 2387}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 43001,
			.name = "食べごろハンター",
			.element = Element::Light,
			.role = Role::Buffer,
			.spd = 117,
			.mp = 75,
			.stats = {
				.lv1   = {277, 91, 92},
				.lv120 = {4930, 1590, 1603},
				.lv140 = {5351, 1823, 1838},
				.lv160 = {5720, 2006, 2022}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 43002,
			.name = "理子のキオク",
			.element = Element::Light,
			.role = Role::Breaker,
			.spd = 114,
			.mp = 0,
			.stats = {
				.lv1   = {289, 83, 96},
				.lv120 = {5142, 1449, 1677},
				.lv140 = {5319, 1841, 1832},
				.lv160 = {5686, 2026, 2016}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 44001,
			.name = "漆黒のアルカナ",
			.element = Element::Forest,
			.role = Role::Breaker,
			.spd = 103,
			.mp = 150,
			.stats = {
				.lv1   = {332, 85, 111},
				.lv120 = {5972, 1525, 1994},
				.lv140 = {6415, 1694, 2216},
				.lv160 = {6857, 1863, 2438}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 45001,
			.name = "刻み込む桜の物語",
			.element = Element::Aqua,
			.role = Role::Attacker,
			.spd = 90,
			.mp = 90,
			.stats = {
				.lv1   = {260, 133, 87},
				.lv120 = {4671, 2385, 1566},
				.lv140 = {5017, 2650, 1740},
				.lv160 = {5363, 2915, 1914}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 46001,
			.name = "無思考",
			.element = Element::Dark,
			.role = Role::Debuffer,
			.spd = 112,
			.mp = 75,
			.stats = {
				.lv1   = {279, 113, 100},
				.lv120 = {5022, 2034, 1800},
				.lv140 = {5394, 2260, 2000},
				.lv160 = {5766, 2486, 2200}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 47001,
			.name = "オシャンティックハリケーン",
			.element = Element::Void,
			.role = Role::Breaker,
			.spd = 114,
			.mp = 75,
			.stats = {
				.lv1   = {290, 82, 97},
				.lv120 = {5161, 1430, 1689},
				.lv140 = {5601, 1640, 1937},
				.lv160 = {5987, 1804, 2130}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 47002,
			.name = "アシュリーのキオク",
			.element = Element::Dark,
			.role = Role::Attacker,
			.spd = 95,
			.mp = 0,
			.stats = {
				.lv1   = {232, 120, 78},
				.lv120 = {4132, 2090, 1367},
				.lv140 = {5267, 2120, 1838},
				.lv160 = {5630, 2203, 2022}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 47003,
			.name = "デンジャラスグロかわカーニバル",
			.element = Element::Void,
			.role = Role::Buffer,
			.spd = 122,
			.mp = 75,
			.stats = {
				.lv1   = {333, 95, 111},
				.lv120 = {5999, 1701, 1989},
				.lv140 = {6444, 1890, 2210},
				.lv160 = {6888, 2079, 2431}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 48001,
			.name = "粛清天使",
			.element = Element::Dark,
			.role = Role::Attacker,
			.spd = 94,
			.mp = 90,
			.stats = {
				.lv1   = {236, 120, 77},
				.lv120 = {4204, 2096, 1338},
				.lv140 = {4562, 2403, 1534},
				.lv160 = {4877, 2643, 1687}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 48002,
			.name = "クシュのキオク",
			.element = Element::Dark,
			.role = Role::Attacker,
			.spd = 96,
			.mp = 0,
			.stats = {
				.lv1   = {230, 120, 78},
				.lv120 = {4103, 2101, 1364},
				.lv140 = {4453, 2408, 1564},
				.lv160 = {4760, 2649, 1721}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 48003,
			.name = "赫奕の福音",
			.element = Element::Dark,
			.role = Role::Attacker,
			.spd = 90,
			.mp = 90,
			.stats = {
				.lv1   = {261, 133, 87},
				.lv120 = {4698, 2385, 1557},
				.lv140 = {5046, 2650, 1730},
				.lv160 = {5394, 2915, 1903}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 49001,
			.name = "ラ・リュミエール",
			.element = Element::Light,
			.role = Role::Defender,
			.spd = 94,
			.mp = 90,
			.stats = {
				.lv1   = {294, 74, 134},
				.lv120 = {5292, 1337, 2408},
				.lv140 = {5684, 1486, 2676},
				.lv160 = {6076, 1635, 2944}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 50001,
			.name = "ロンブル",
			.element = Element::Dark,
			.role = Role::Buffer,
			.spd = 121,
			.mp = 100,
			.stats = {
				.lv1   = {287, 110, 101},
				.lv120 = {5157, 1980, 1811},
				.lv140 = {5539, 2200, 2012},
				.lv160 = {5921, 2420, 2213}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 51001,
			.name = "デサンテグラシオン",
			.element = Element::Aqua,
			.role = Role::Debuffer,
			.spd = 114,
			.mp = 90,
			.stats = {
				.lv1   = {279, 109, 104},
				.lv120 = {5022, 1962, 1872},
				.lv140 = {5394, 2180, 2080},
				.lv160 = {5766, 2398, 2288}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 52001,
			.name = "ラ・ダンス・マカブル",
			.element = Element::Void,
			.role = Role::Debuffer,
			.spd = 116,
			.mp = 75,
			.stats = {
				.lv1   = {314, 115, 87},
				.lv120 = {5643, 2070, 1557},
				.lv140 = {6061, 2300, 1730},
				.lv160 = {6479, 2530, 1903}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 53001,
			.name = "オラクルレイ",
			.element = Element::Light,
			.role = Role::Attacker,
			.spd = 96,
			.mp = 100,
			.stats = {
				.lv1   = {256, 135, 86},
				.lv120 = {4601, 2432, 1544},
				.lv140 = {4942, 2702, 1716},
				.lv160 = {5282, 2972, 1888}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 54001,
			.name = "ヴァンパイアファング",
			.element = Element::Dark,
			.role = Role::Defender,
			.spd = 87,
			.mp = 75,
			.stats = {
				.lv1   = {305, 71, 134},
				.lv120 = {5481, 1274, 2408},
				.lv140 = {5887, 1416, 2676},
				.lv160 = {6293, 1558, 2944}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 55001,
			.name = "リンクスインパクト",
			.element = Element::Forest,
			.role = Role::Healer,
			.spd = 104,
			.mp = 90,
			.stats = {
				.lv1   = {365, 60, 93},
				.lv120 = {6503, 1055, 1627},
				.lv140 = {7057, 1210, 1865},
				.lv160 = {7544, 1331, 2051}
			},
			.actions = nullptr,
			.rarity = 4,
		},
		{
			.id = 55002,
			.name = "ゆまのキオク",
			.element = Element::Forest,
			.role = Role::Healer,
			.spd = 101,
			.mp = 0,
			.stats = {
				.lv1   = {363, 62, 93},
				.lv120 = {6465, 1079, 1616},
				.lv140 = {7016, 1237, 1852},
				.lv160 = {7500, 1360, 2037}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 56001,
			.name = "天翔ける語り部",
			.element = Element::Void,
			.role = Role::Defender,
			.spd = 93,
			.mp = 75,
			.stats = {
				.lv1   = {322, 64, 134},
				.lv120 = {5794, 1157, 2417},
				.lv140 = {6223, 1286, 2686},
				.lv160 = {6653, 1415, 2955}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 57001,
			.name = "もう絶望する必要なんてない！",
			.element = Element::Light,
			.role = Role::Attacker,
			.spd = 98,
			.mp = 100,
			.stats = {
				.lv1   = {282, 140, 82},
				.lv120 = {4531, 2522, 1478},
				.lv140 = {5446, 2802, 1642},
				.lv160 = {5822, 3082, 1806}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 58001,
			.name = "事象の改竄",
			.element = Element::Dark,
			.role = Role::Attacker,
			.spd = 100,
			.mp = 150,
			.stats = {
				.lv1   = {284, 136, 86},
				.lv120 = {5114, 2439, 1543},
				.lv140 = {5493, 2710, 1714},
				.lv160 = {5871, 2981, 1885}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 59001,
			.name = "ティロフィナーレリベレーション",
			.element = Element::Dark,
			.role = Role::Attacker,
			.spd = 97,
			.mp = 90,
			.stats = {
				.lv1   = {269, 127, 90},
				.lv120 = {4833, 2277, 1620},
				.lv140 = {5191, 2530, 1800},
				.lv160 = {5549, 2783, 1980}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 60001,
			.name = "ラ・ポルトゥ・ドゥ・パラディ",
			.element = Element::Light,
			.role = Role::Attacker,
			.spd = 98,
			.mp = 150,
			.stats = {
				.lv1   = {289, 135, 85},
				.lv120 = {5200, 2430, 1526},
				.lv140 = {5585, 2700, 1696},
				.lv160 = {5971, 2970, 1866}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 61001,
			.name = "フローラル・アイアンスパイク",
			.element = Element::Forest,
			.role = Role::Attacker,
			.spd = 100,
			.mp = 150,
			.stats = {
				.lv1   = {287, 134, 87},
				.lv120 = {5162, 2412, 1557},
				.lv140 = {5545, 2680, 1730},
				.lv160 = {5927, 2948, 1903}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 62002,
			.name = "ファイナル・フェイトブルーム",
			.element = Element::Dark,
			.role = Role::Breaker,
			.spd = 114,
			.mp = 90,
			.stats = {
				.lv1   = {331, 94, 112},
				.lv120 = {5951, 1688, 2016},
				.lv140 = {6392, 1876, 2240},
				.lv160 = {6832, 2064, 2464}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 63001,
			.name = "例外のほうが多い規則",
			.element = Element::Forest,
			.role = Role::Breaker,
			.spd = 112,
			.mp = 75,
			.stats = {
				.lv1   = {331, 96, 109},
				.lv120 = {5962, 1733, 1967},
				.lv140 = {6403, 1926, 2186},
				.lv160 = {6845, 2119, 2405}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 64001,
			.name = "螺旋帰道",
			.element = Element::Aqua,
			.role = Role::Defender,
			.spd = 95,
			.mp = 90,
			.stats = {
				.lv1   = {319, 63, 146},
				.lv120 = {5740, 1139, 2633},
				.lv140 = {6165, 1266, 2926},
				.lv160 = {6591, 1393, 3219}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 65001,
			.name = "怪異殺し",
			.element = Element::Flame,
			.role = Role::Attacker,
			.spd = 100,
			.mp = 100,
			.stats = {
				.lv1   = {294, 132, 86},
				.lv120 = {5287, 2376, 1552},
				.lv140 = {5678, 2640, 1724},
				.lv160 = {6070, 2904, 1896}
			},
			.actions = nullptr,
			.rarity = 5,
		},
		{
			.id = 66001,
			.name = "まどか先輩のキオク",
			.element = Element::Light,
			.role = Role::Attacker,
			.spd = 116,
			.mp = 0,
			.stats = {
				.lv1   = {88, 88, 88},
				.lv120 = {4325, 2074, 1442},
				.lv140 = {4646, 2304, 1602},
				.lv160 = {4966, 2534, 1762}
			},
			.actions = nullptr,
			.rarity = 3,
		},
		{
			.id = 70001,
			.name = "アクア・テンペスト",
			.element = Element::Aqua,
			.role = Role::Buffer,
			.spd = 120,
			.mp = 90,
			.stats = {
				.lv1   = {324, 101, 107},
				.lv120 = {5832, 1809, 1933},
				.lv140 = {6264, 2010, 2148},
				.lv160 = {6696, 2211, 2363}
			},
			.actions = nullptr,
			.rarity = 5,
		},
	}
};

/**
 * @brief MPではなくBPを用いて必殺技を打つキオク一覧。
 * - first: キオクのID
 * - second: 必殺技に必要なBP
 */
inline constexpr std::array<std::pair<int, int>, 2> BP_Kioku_List = {{ { 1005, 12 }, { 2008, 15 } }};

/**
 * @brief BP_Kioku_Listから指定キオクIDの最大BPを取得する
 * @param id キオクID
 * @return 登録済みならその最大BP、未登録なら0
 */
inline int getMaxBP(int id) {
    for (const auto& entry : BP_Kioku_List) {
        if (entry.first == id) return entry.second;
    }
    return 0;
}


/**
 * @brief 指定されたキオクIDからCharacterマスターデータを検索する
 * @param id キオクID
 * @return 見つかった場合はCharacterポインタ、存在しない場合はnullptr
 */
inline const Character* FindCharacterById(int id) {
    auto it = std::find_if(Kioku_data.begin(), Kioku_data.end(),
                          [id](const auto& ch) { return ch.id == id; });
    return it != Kioku_data.end() ? &*it : nullptr;
}

#endif // KIOKU_DATA