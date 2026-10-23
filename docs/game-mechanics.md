# ゲームの仕様について

- 更新: 2026-10-23

> **確度の表記**
>
> - **[現行実装]**: 現行ソースコードで実装済み。
> - **[一部実装]**: 部分的に実装済み。
> - **[未実装]**: 完全に未実装またはスタブ。
> - **[設計案]**: 提案・検討中。

## 1. バトルの基本モデル

現行実装には編成、ステータス計算、AVおよびTimeline、Process / Effect 実行枠、履歴がある。一方、キオクごとの実スキルデータ、状態異常、バフ・デバフ、ブレイク、敵 AI などは未完成またはデバッグ用である。

| 用語 | 意味 |
|---|---|
| キオク | ユニットの一種。`Character` と呼ぶ箇所がある。 |
| Actor　| 行動者。Processを実行するユニットであり、基本TLの先頭のユニットと同義。 |
| Target | Process / Effectを受けるユニット。 |
| BA | 通常攻撃(Basic Attack) |
| BS | 戦闘スキル(Battle Skill) |
| SA | 必殺技(Special Attack) |
| BGD | ブレイクゲージ削減量(Break Gauge Depletion) |
| Effect | 基本的に戦闘状態へ作用する処理単位。 |
| Process | **[仮説・要確認]** 順序付Effect列を持つ処理単位。 |
| Action | **[仮説・要確認]** Processの一種。 |
| AV | 行動値(Action Value)。対象がActorになるまでの所要時間を表す。 |
| SPD | ユニットのスピード。 |
| Break | **TODO:** ブレイクゲージが0(以下)のときに発生する状態。Break状態は行動不可状態である。 |

## 2. 基本データ

### 2.1 キオク: `Character`

**[現行実装]**

`unit_types.h` の `Character` は静的マスターデータである。

- `id`, `name`: 識別用の整数値と名前
- `element`, `role`: 属性とロール
- `spd`: SPD
- `mp`: 最大MP
- `stats`: Lv1 / Lv120 / Lv140 / Lv160 の基礎ステータス
- `actions`: `const Actions*`。現行では `nullptr`
- `rarity`: レアリティ（3 / 4 / 5）

シミュレーションに用いるステータスは、必要なものを `UnitStatDetails` にコピーして分離する。

### 2.2 属性・ロール

**[現行実装]**

| 分類 | 値 |
|---|---|
| `Element` | `None`, `Flame`, `Forest`, `Aqua`, `Light`, `Dark`, `Void` |
| `Role` | `None`, `Attacker`, `Breaker`, `Healer`, `Buffer`, `Debuffer`, `Defender` |

画像シートのアイコン対応は A1=Aqua、A2=Dark、A3=Flame、A4=Forest、A5=Light、A6=Void。これはアイコンリソースの対応であり、enum の整数値と混同しない。

> [!NOTE]
>
> 属性相性、弱点・耐性、ロールによる補正は現在の `DealDMG` には入っていない。

### 2.3 マスターデータの有効性

**[現行実装]**

`Kioku_data` の型は `std::array<Character, 128>`。現在 115 件が有効 ID を持つ。`id != 0` で未使用エントリを除外し、実際の配列インデックスを維持して選択リストを作る。編成の未選択スロットは `-1` で表し、マスターデータの未使用エントリとは区別する。

## 3. 編成

### 3.1 編成枠

**[現行実装]**
- 味方・敵それぞれ 5 スロット。画面左が味方、右が敵。
- 同陣営内で、別スロットに存在するキオクを選ぶと、キオクと育成パラメータを交換する。陣営が異なる場合は交換しない。
- キオクを外すとスロットの育成値は既定値に戻る。
- キオク画像、属性、ロールのアイコンを重ねて表示し、ホバー時に基本情報・ステータスを表示する。

### 3.2 各パラメータ

**[現行実装]**

`FormationView::UnitParameters` の初期値:

| 項目 | 初期値 | 意味 |
|---|---:|---|
| `ascension` | 5 | 限界突破（0～5） |
| `level` | 160 | キオクレベル（1～160） |
| `heartphial` | 50 | こころの器レベル（1～50） |
| `magicLevel` | 140 | 魔力解放（0～`max_magic_level`） |
| `specialLvl` | 10 | 必殺技レベル（1～`max_special_level`） |

`max_magic_level`、および`max_special_level`はそれぞれ`FormationView::GetMagicLevelMax(level)`、`FormationView::GetSpecialLevelMax(ascension)`で上限を取得している。

### 3.3 ステータス計算

**[現行実装]**

`CalcStats` は Lv1 / Lv120 / Lv140 / Lv160 の値を基準点として、任意のレベルにおける HP / ATK / DEF を線形補間によって求める。

その手順は以下のとおりである。

1. 対象レベル `L` を含む区間を選択する。
2. 区間の両端の基準点を用いて、HP / ATK / DEF をそれぞれ線形補間する。
3. 限界突破倍率を適用する。
4. 魔力解放・こころの器によるステータスボーナスを加算する。

線形補間の計算式は以下のとおりである。

$$
S(L)=S_0+
\left\lfloor
\frac{(S_1-S_0)(L-L_0)}{L_1-L_0}
\right\rfloor
$$

各記号の意味は以下のとおりである。

| 記号 | 意味 |
|---|---|
| `S(L)` | レベル `L` における補間後のステータス値 |
| `L` | ステータスを求める対象レベル |
| `L_0` | 対象レベルを含む区間の下側の基準レベル |
| `L_1` | 対象レベルを含む区間の上側の基準レベル |
| `S_0` | 基準レベル `L_0` におけるステータス値 |
| `S_1` | 基準レベル `L_1` におけるステータス値 |

つまり、下側の基準レベルから対象レベルまでの進行割合に応じて、両端のステータス値の差分を加算する。補間計算は整数演算で行い、端数を切り捨てる。

限界突破倍率は `{100, 102, 104, 106, 108, 110}` % である。限界突破 0 が 100%、限界突破 5 が 110% に対応する。倍率適用時も整数演算で端数を切り捨てる。

魔力解放はlowRarity(3/4) 用と highRarity(5)用のテーブルを使い、選択レベル以下の段階を累積加算する。こころの器もテーブルを累積するが、データはrarityではなく、初期リリースで☆3でのみ実装されているキオク（夏希、かこ、美雨、このは）とそれ以外で分ける。専用のキオクポートレートが存在しないため、HPステータスで代替されているのが理由である。

以下はレベルと魔力解放上限の対応表である。

| Lv | `max_magic_level` |
|---|---:|
| 1–29 | 30 |
| 30–59 | 60 |
| 60–89 | 90 |
| 90–100 | 120 |
| 101–109 | 125 |
| 110–135 | 130 |
| 136–145 | 135 |
| 146–160 | 140 |

## 4. 戦闘開始と状態

### 4.1 `TeamState` / `UnitStatDetails`

**[現行実装]**

`TeamState` は `ally[5]`, `enemy[5]` と陣営別にSPを持つ。`UnitStatDetails` は `Character`、HP、最大HP、ATK、DEF、現在 MP、現在BP、有効なSPD効果リスト、最大MP、最大BP、AV、totalSPD、turnPriority を持つ。

編成画面はParametersから HP / ATK / DEF を計算し、`TeamState` にキオク情報をコピーする。currentMPは0、AVは`10000 / SPD`で初期化する。戦闘ロジックは初期HPを`maxHP` にmoveして、タイムラインを構築する。空スロットの HP / maxHPは0。

なお`maxHP`へのEffectによる影響として、最大HP上昇Effectが存在するが、現在未実装のため、完全固定な値である。

## 5. タイムラインについて

### 5.1 基本式

**[現行実装]**

$$
InitAV=\frac{10000}{SPD}
$$

Actor決定の原則として、AVの小さいキオクが先に行動する。SPDは正の値を前提としており、SPD減少によるゼロ除算は考慮されていない (Reference: [まきねね(@makinene071)氏によるXでの投稿](https://x.com/makinene071/status/2012910681868186039))

### 5.2 初期タイムライン

最初のTL構築は`initTimeline`で行われている。処理順序は以下の通り。

0. TL、行動ログ、スキル選択等の各状態を初期値にリセットし、TLの容量を確保する。
1. IDが有効(かつHP > 0)のスロットをTLに追加。
2. 全てのスロットに対しそのAV を InitAV に設定する(`setTurnAV`)。
3. バトル開始時に発動するPassive効果を、味方→敵、スロット順に適用する(`applyBattleStartAbility`)。1回の適用につき、turnPriorityを1度発行する。各効果の適用直後に、SPD系効果であればAV更新(`rescaleAVBySpeed`)を行う。
4. AV 昇順 → turnPriority 降順 → チーム順(味方が先行) → スロット順 に整列する(`compareTurnOrder`)。
5. 先頭の最小AVを全ユニットから減算し、先頭をActorとする(`shiftNextTurn`)。
6. 差し引いた値を累積時間 `accumTime` に加える。


### 5.3 行動後

Actor の AV を `10000.0f / SPD` に戻し、AV値比較のみでTLに安定挿入し、minAV を全員から差し引いて次の Actor を選ぶ。差し引いた時間は `accumTime` に加算される。

ログに残る時刻 `t` は累積時間を小数第 3 位の切り捨てで表示する。

> [!NOTE]
>
> `setTurnAV`ではturnPriorityは更新されないという仮定の下、このようなTLへの操作を行っている。担当する関数は`relocateTimelineNodeByAV`。
>
> 現在は、行動によりAV値が変わりうる効果を発動した場合、AV更新の処理で対象のturnPriorityが更新されるという構成にしている。



## 6. Action・Process・Effect

### 6.1 スキル種別

**[現行実装]**

 `SkillType`: SA=0、BS=1、BA=2、Ability=3。UI で切替できるのは BA / BS。またSPによる制約も未実装。
 
 `ProceedTurnWithHistory` は BA / BS だけを受け付ける。SA のデバッグ Process 登録はあるが、通常の行動経路から実行できる状態ではない。Ability は未実装。

### 6.2 Process と Effect

このプロジェクトにおいて、Process は Effect の順序付き列、Effect は個別の戦闘処理として扱っている。現行の `ProcessDefinition` は `ProcessId`、`EffectCall` 配列ポインタ、Effect 数を持つ。`ProcessId` は `uint32_t`(0 = unused)。`ExecuteProcess` は格納されている Effect 列を順に実行する。

`EffectType` は現状 `DealDMG = 0`, `BuffSPDFixed = 1`, `BuffSPDRatio = 2` の 3 種。`EffectCall` は型と引数ポインタを保持し、`EffectDefinition` の登録表から Invoker を引く。新しい Effect を追加する際は enum、引数構造体、Invoker、登録表、対象展開、戦闘状態更新を同期させる。

### 6.3 現行のDebug用 Process

> [!NOTE]
>
> 一部のDebug用のProcessは主にAIによる実験として導入されている。

| ID | 用途 | 設定 |
|---:|---|---|
| 1 | BA | 敵単体に100%無属性ダメージ(BGD15) |
| 2 | BS | 敵単体に150%無属性ダメージ(BGD12)、隣接する敵に120%無属性ダメージ(BGD10) |
| 3 | SA | 敵全体に180%無属性ダメージ(BGD10) |
| 4 | Ability (SPD+1) | 自身に SPD +1（BuffSPDFixed, Self, turns=-1） |
| 5 | Ability (SPD+10%) | 自身に SPD +10%（BuffSPDRatio, Self, turns=-1） |
| 6 | Ability (SPD+1, 1ターン) | 自身に SPD +1（BuffSPDFixed, Self, turns=1） |
| 7 | Ability (SPD+1, 1ターン, Single) | 対象指定（Single）に SPD +1（BuffSPDFixed, turns=1） |

現在は、バトル開始時にID 4 が発動するよう、有効な全スロットに対して付与される。


## 7. 対象範囲・戦闘不能

### 7.1 対象範囲

**[現行実装]**

Effect の対象の範囲は4つある。

| TargetRange | 対象 |
|---|---|
| `Self` | Actor自身 |
| `Single` | Actorが選択した単体 |
| `Proximity` | Actorが選択した主対象及び主対象に隣接するスロット |
| `All` | 対象陣営の全スロット |

近接範囲は編成上のスロット位置に依存する。空・戦闘不能スロットは処理対象外。

### 7.2 EffectContext と戦闘不能処理

`EffectContext` は Actor / Target の陣営・スロット、スロット占有配列、HP / DEF 解決コールバック、攻撃側の基礎 ATK・総 ATKなどを持つ。Effect はこのcontextから対象の値を解決する。

`DealDMG`により HP が 0 になった場合、戦闘不能コールバックで個人 MP/BP を 0 にし、スロット占有状態を false にし、タイムラインから削除する(Actorが戦闘不能になった場合はActorを次の先頭へ切り替える)。SPは戦闘不能で変更されない。`RemoveEffectsGivenOrReceived` : スタブ(何もしない)。

## 8. ダメージ計算

### 8.1 計算式

`EffectList.cpp` の `DealDMG` は次を計算する。

$$
BaseDMG=BaseATK\times\frac{(BaseATK/124)^{1.2}+12}{20}
$$

$$
AttackRatio=\min\left(\frac{TotalATK+10}{TotalDEF+10}\times0.12,\ 2.0\right)
$$

$$
RawDamage=\frac{dmg}{1000}\times BaseDMG\times AttackRatio
$$

$$
Damage=\max(1,\lceil RawDamage\rceil),\quad HP'=\max(0,HP-Damage)
$$

dmgはパーミル表記である。例えば180%のダメージはdmg=1800と表される。計算は float / `std::pow`、端数は切り上げ、ダメージ最低値は 1、HP最低値は 0。

### 8.2 現行の制限

`element`, `bgd`, `proximity_dmg`, `proximity_bgd` 等は引数として存在するが、BGD は未処理。属性耐性、弱点、クリティカル、バフ・デバフ、被ダメージ補正、バリア、回復、持続ダメージ も未実装。現行ロジックは `actorBaseAtk` と `actorTotalAtk` に同じ `actor.atk` を渡すため、基礎 ATK と 合計ATK の区別も存在しない。

## 9. SP・MP・BP

### 9.1 SP

**[設計案]**

SP はチーム共有で初期値 5、範囲は0～6。通常攻撃は +1、他のスキルは消費 SP を処理し、SP回復Effect も同じ上限を使う。

### 9.2 MP

**[設計案]**

MP回復の基礎値は以下の通りである。

| 条件・行動 | MP回復量 |
|---|---:|
| BA | +15 |
| BS | +30 |
| SA | +5 |
| 敵の撃破 | +10 |
| 持続ダメージ発生 | +2 |
| 被弾（HP40%以上） | +5 |
| 被弾（HP10%以上、40%未満） | +10 |
| 被弾（HP10%未満） | +15 |

この基礎値に対してMP回復効率による補正がかかり、最大MPを上限とする。

### 9.3 BP

**[設計案]**

一部キオクが MP の代わりに BP を使う。BP は 0 から始まり、BA +1、BS +2、MP回復系のEffect を受けた場合 +1される。

> [!NOTE]
>
> 非常に面倒になりそうなためBPを実装する予定はしばらくありません。

## 10. 履歴・行動ログ

### 10.1 行動フロー

**[現行実装]**

1. SkillTypeが BA / BS であること、Actor・スロット・HP の有効性を検証
2. 次の履歴行動とSkillType・Targetが一致すれば、再計算せず保存済み状態を復元。異なる行動を選んだ場合、未来の履歴・行動・ログを破棄
3. 対象を記録し、BAであればSP +1、MP +15 を行い、BSであればSP -1、MP +30 を行った後、Debug用 Process を実行
4. 行動ログを追加
5. AV更新・再ソート・最小AV 減算で次の Actor を決定
6. スナップショットを保存

### 10.2 Undo / Redo

履歴の index 0 は初期状態。各スナップショットは `TeamState`、タイムライン、Actor、選択対象、累積時間を保存する。Undo / Redo はスナップショットを復元する。同じ行動・対象は既存の次履歴を再生し、異なる行動を選ぶと未来を破棄して分岐する。

**[設計案]**

これは状態を復元する方式であり、乱数状態を含む完全な戦闘リプレイではない。将来 RNG を使うなら RNG 状態も保存対象にする必要がある。

### 10.3 ログ

ログは `A1`～`A5`（味方）、`E1`～`E5`（敵）のスロット表記、Actor、Target、BA/BS/SA、累積時刻 `t` を記録する。時刻は小数第 2 位まで切り捨てる。Effect ごとの詳細、ダメージ前後、状態変化、乱数判定などは未統合。

## 11. 状態効果と過去の設計論点

### 11.1 優先して実装する効果

**[設計案]**
- `BuffSPDFixed`, `BuffSPDRatio`
- `GainMPFixed`, `GainSPFixed`
- `AdvanceActionValue`, `DelayActionValue`

これらはタイムラインの構築に非常に大きな影響を与えるため最優先で導入する。また

- `BuffATKFixed`, `BuffATKRatio`
- `BuffDEFFixed`, `BuffDEFRatio`

などの効果は、タイムラインには直接的影響がないため後回しで実装する。

### 11.2 データモデル上の原則

- キオクの静的マスターデータと戦闘中の状態を分離する。
- Process を共有可能にする。
- Process は Effect の順序付き列とする。
- 効果の持続、条件、対象、重複、解除が必要になるため、将来的に固定値加算だけではなく状態モデルを導入する。
- Effect 追加時は対象解決、戦闘不能処理、ログ、履歴との整合性を確認する。

### 11.3 Effect 適用の設計案

- ステータスにかかる効果は バフ/デバフ効果量、additive → multiplicative、付与順の順に適用する。
- Effect 重複時は付与元、Effect 起源、状態型、条件等に応じて置換・統合・累積する。
- Effect の残りターンは原則として保持者のターン終了時に減る。自分のスキルで自分に付与した効果は、付与ターンの終了時では減少しない。
- アビリティ効果は常時効果そのものとは限らず、特定タイミング・条件で状態を付与するトリガーとして動く。
- Actor、主対象、Effect ごとの対象、スキル種別、スキル中の通知などを条件判定が参照することがある。
- 状態異常はバフ・デバフ効果と異なる。


## 12. ブレイク

**[設計案]**

 `DealDMG`の直後にブレイクボーナス増加または BGD が処理される。ブレイクゲージが 0 になるとブレイク回数(BreakCount)が増え(例: メインクエスト/女王の黄昏編/ステージ4-1/女王の黄昏のアビリティ効果などで参照される)、`DelayActionValue`とブレイク中の被ダメージ補正(ブレイクボーナス)が設定される。ブレイクボーナスはブレイク中の攻撃で上がる。上限があり、ユニットごとに設定されている。ブレイク中のユニットのターン開始時に行動可能(=気絶状態でない)なら回復して行動し、行動不能ならブレイク状態を継続する。ブレイクゲージの有無・最大値はユニット/敵、レアリティ・ロール等に依存する。

## 13. 仕様一覧（実装状況）

| 項目 | 状態 | 注記 |
|---|---|---|
| 味方・敵各 5 枠 | [現行実装] | 編成 UI と戦闘状態 |
| キオク基本データ | [現行実装] | 未使用 ID を除外 |
| Lv1/120/140/160 補間 | [現行実装] | 元ゲームの完全な式としては要検証 |
| 限界突破 / 魔力解放 / こころの器 | [現行実装] | テーブルに依存 |
| BA / BS 選択 | [現行実装] | デバッグ Process |
| SA / Ability 実行 | [設計案] | 通常行動では BA/BS のみ |
| AV = 10000 / SPD | [現行実装] | 簡易タイムライン |
| SP 初期 5、上限 6、行動前 +1 | [現行実装] | スキル消費未実装 |
| Process / Effect 分離 | [現行実装] | Effect は DealDMG のみ |
| ダメージ計算 | [一部実装] | 属性・状態・ブレイク未反映 |
| 対象範囲 | [現行実装] | 有効スロットのみ |
| 戦闘不能処理 | [一部実装] | 効果解除はダミー |
| Undo / Redo / 分岐 | [一部実装] | スナップショット復元。ログの分岐は未実装 |
| 状態効果・持続・重複・解除 | [一部実装] | SPD 効果の持続ターン数モデルを実装（`SPDEffect` / `SPDEffectList` / `tickUnitSPDEffects`）。重複・解除の再付与規則は未実装。 |
| タイムライン安定挿入 | [現行実装] | AV計算時に `relocateTimelineNodeByAV` で安定挿入。`shiftNextTurn` の `sortTimeline()` を削除し、同 SPD ユニットの交互行動を実現。 |
| BGD / Break | [未実装] | 引数のみ |
| クリティカル・耐性・バリア・回復・持続ダメージ | [未実装] |  |
| 履歴に乱数を含む | [未実装] | クリティカル実装と同時に実装予定 |

## 14. 今後の実装で優先したいこと (Wish list)

1. 実スキルの Process ID 規約とマスターデータ構造を確定し、デバッグ ID と分ける。
2. BA / BS / SA の各Processを主となるダメージ効果・回復効果のみ定義する。
3. Effect を一つずつ実装し、対象、状態更新、ログ、戦闘不能、履歴復元の整合性を確認する。
4. 状態効果を導入する場合、付与者・保持者・対象、Effect 起源、条件、残りターン/回数、重複の処理、解除、発動タイミングを分離する。
