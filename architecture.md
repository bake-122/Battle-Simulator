# アーキテクチャ

## 概要

この文書は、Exedra Sim のソースコード構成、主要コンポーネントの責務、およびコンポーネント間の依存関係を説明する。個々のゲームルールの詳細ではなく、変更を加えるときに「どのファイルを確認すべきか」を判断するための地図として扱う。

アプリケーションは C++ によるデスクトップアプリで、GLFW がウィンドウと入力イベントを扱い、Dear ImGui が UI を構築し、OpenGL が描画を行う。キオクの編成画面とバトルシミュレーション画面は、シーン単位で切り替わる。

## 実行時の構成

大きく分けると、次の層で構成される。

- **アプリケーション制御** — 初期化、メインループ、シーン切り替え、終了処理。
- **画面/UI** — 編成画面とシミュレーション画面の表示およびユーザー入力。
- **シミュレーションロジック** — チーム状態、行動タイムライン、行動ログ、ターン進行、履歴。
- **行動定義** — Process が Effect の列を保持し、Effect が戦闘状態に対する処理を実行する。
- **共有データとリソース** — キオクのマスターデータ、共通型、画像テクスチャ、ウィンドウ設定。

## ディレクトリ構成

以下は、アプリケーション固有のファイルを中心とした構成である。Dear ImGui、GLFW、WebP のソースやライブラリ、および個々の埋め込み画像ヘッダーは省略している。

```text
.
├── main.cpp
├── scene_type.hpp
├── window_config.h
├── unit_types.h
├── kioku_data.h
├── image_loader.hpp
├── image_loader.cpp
├── formation/
│   ├── formationView.h
│   └── formationView.cpp
├── simulation/
│   ├── simulationView.hpp
│   ├── simulationView.cpp
│   ├── simulationLogic.hpp
│   ├── simulationLogic.cpp
│   ├── ProcessList.hpp
│   ├── ProcessList.cpp
│   ├── EffectList.hpp
│   └── EffectList.cpp
├── embedded_images/
│   ├── units/
│   ├── elements/
│   └── roles/
├── images/
├── resources/
├── imgui/
├── glfw/
├── webp/
├── lib/
├── CMakeLists.txt
├── Makefile
├── unity.cpp
├── build.bat
└── build-release.bat
```

## 主要ファイルの責務

### アプリケーションの起動と共有設定

| ファイル | 責務 |
|---|---|
| [main.cpp](main.cpp) | GLFW、Dear ImGui、フォント、画像管理の初期化と終了処理を行う。メインループで現在のシーンを更新・描画し、シーンが変わった場合はウィンドウタイトルを更新する。 |
| [scene_type.hpp](scene_type.hpp) | `SceneType::Formation` と `SceneType::Simulation` を定義する。 |
| [window_config.h](window_config.h) | ウィンドウの共通サイズ（現在は 1280 × 720）を定義する。 |
| [unit_types.h](unit_types.h) | `Element`、`Role`、`Stats`、`CharacterStats`、`Character` など、キオクとステータスに関する共有型を定義する。 |
| [kioku_data.h](kioku_data.h) | `Kioku_data` としてキオクのマスターデータを保持する。配列には未使用エントリもあるため、選択可能なデータは `id != 0` など既存の有効判定を通して扱う。 |

### 編成画面

| ファイル | 責務 |
|---|---|
| [formation/formationView.h](formation/formationView.h) | `FormationView` のインターフェース、味方・敵のスロット状態、キオクのパラメータ、選択状態、および描画・操作関数を宣言する。 |
| [formation/formationView.cpp](formation/formationView.cpp) | 5枠ずつの編成、キオク選択・解除・入れ替え、パラメータ編集、ステータス計算、アイコンやツールチップの描画を実装する。シミュレーション開始時には編成内容を `TeamState` に変換して渡す。 |

### シミュレーション画面と戦闘処理

| ファイル | 責務 |
|---|---|
| [simulation/simulationView.hpp](simulation/simulationView.hpp) | シミュレーション画面の公開インターフェースを宣言する。チーム状態の受け渡し関数と画面描画関数を公開し、ロジック側の `TeamState` を参照する。 |
| [simulation/simulationView.cpp](simulation/simulationView.cpp) | 戦闘画面、タイムラインや行動ログの表示、画面レイアウト、操作 UI を実装する。戦闘処理の詳細はシミュレーションロジック側に委譲する。 |
| [simulation/simulationLogic.hpp](simulation/simulationLogic.hpp) | `TeamState`、`UnitStatDetails`、`TimelineNode`、`SkillType` などの戦闘状態・インターフェースを宣言する。 |
| [simulation/simulationLogic.cpp](simulation/simulationLogic.cpp) | チーム状態、行動タイムライン、行動ログ、行動者とターゲット、ターン進行、および履歴スナップショットを管理する。Process を呼び出して行動を解決する。 |
| [simulation/ProcessList.hpp](simulation/ProcessList.hpp) | Process の ID、定義、Effect 列、および検索・実行インターフェースを宣言する。 |
| [simulation/ProcessList.cpp](simulation/ProcessList.cpp) | Process 定義の登録表と実行処理を実装する。現時点ではデバッグ用 BA / BS / SA の Process が登録されている。 |
| [simulation/EffectList.hpp](simulation/EffectList.hpp) | Effect の種類、対象範囲、実行コンテキスト、引数、ディスパッチ用インターフェースを定義する。 |
| [simulation/EffectList.cpp](simulation/EffectList.cpp) | Effect のディスパッチと、現在登録されている `DealDMG` の処理を実装する。 |

### 画像と外部リソース

| ファイル / ディレクトリ | 責務 |
|---|---|
| [image_loader.hpp](image_loader.hpp) | `ImageManager` と画像ロード関数のインターフェースを宣言する。 |
| [image_loader.cpp](image_loader.cpp) | 埋め込み WebP データをデコードして OpenGL テクスチャを作成し、キオク・属性・ロールの画像を登録・取得・解放する。 |
| `embedded_images/units/` | キオクの画像データを C++ ヘッダーとして保持する。集約・登録用ヘッダーもこのディレクトリにある。 |
| `embedded_images/elements/` | 属性アイコンの画像データと登録用ヘッダーを保持する。 |
| `embedded_images/roles/` | ロールアイコンの画像データと登録用ヘッダーを保持する。 |
| `images/` | ビルド設定から参照される画像リソース（アプリケーションアイコンなど）を保持する。 |
| `resources/` | 日本語フォントと Windows リソース定義など、実行ファイルに関連するリソースを保持する。 |

### ビルドと配布

| ファイル | 責務 |
|---|---|
| [Makefile](Makefile) | GCC による Debug / Release ビルドを定義する。Debug は各ソースを個別にコンパイルし、Release は `unity.cpp` を使う Unity Build を行う。 |
| [CMakeLists.txt](CMakeLists.txt) | CMake によるビルド構成を定義する。プラットフォーム別のリンク設定や macOS バンドル、フォント配置も扱う。 |
| [unity.cpp](unity.cpp) | Release 用にアプリケーションおよび Dear ImGui の実装ファイルをまとめてインクルードする。 |
| [build.bat](build.bat)、[build-release.bat](build-release.bat) | Windows でのビルドを補助するバッチファイル。 |
| `.github/workflows/` | デスクトップ向けおよび macOS 向けのビルドワークフローを保持する。 |

## コンポーネント間の依存関係

主要なデータと呼び出しの流れは次のとおり。

```text
main.cpp
 ├── FormationView
 │    ├── kioku_data.h
 │    ├── unit_types.h
 │    ├── window_config.h
 │    └── SetSimulationTeamState(...)
 │              │
 │              ▼
 ├── RenderSimulationScene()
 │    └── simulationView.cpp
 │              │
 │              ▼
 │       simulationLogic.cpp
 │          ├── ProcessList.cpp
 │          │      └── EffectList.cpp
 │          └── TeamState / Timeline / Action Log
 │
 └── ImageManager
      └── image_loader.cpp
           └── embedded_images/
```

- `main.cpp` がシーンを切り替え、各画面の実装を呼び出す。
- 編成画面は `Kioku_data` と編成ごとのパラメータから `TeamState` を構築し、シミュレーション画面へ渡す。
- シミュレーション画面はロジック層の公開関数を介して状態を表示・操作する。
- ロジック層は Process を実行し、Process は登録された Effect を順番に呼び出す。
- 画像 UI は `ImageManager` を通じてテクスチャを取得する。埋め込み画像ヘッダーは画像データと登録情報を提供する。

## 重要な境界と前提

- **画面と戦闘処理を分ける。** UI のレイアウトや操作を変更する場合は主に `simulationView.cpp`、ターン進行や戦闘状態を変更する場合は主に `simulationLogic.cpp` を確認する。
- **Process と Effect は別の識別体系を持つ。** `ProcessId` は 32-bit、`EffectType` は 16-bit であり、Process は Effect の列を実行する。新しい定義を追加するときは、各登録表と ID の規約を合わせて確認する。
- **Effect の対象情報はコンテキスト経由で渡す。** `EffectContext` は行動者・対象・スロット占有情報やステータス参照を持ち、Effect が `TeamState` の具体的な内部構造へ直接依存しすぎないようにしている。
- **空スロットは有効なキオクと区別する。** キオクのマスターデータには未使用エントリがあり、編成スロットにも未選択状態がある。ID やスロットの有効性を確認してから処理する。
- **戦闘状態は `TeamState` を中心に扱う。** 編成画面から渡された初期状態をロジック層が保持し、HP、タイムライン、行動者、ログ、履歴などを更新する。
- **画像の読み込みは OpenGL コンテキスト初期化後に行う。** `ImageManager::Init()` は WebP データを OpenGL テクスチャへ変換するため、起動時の初期化順序に依存する。

## 変更時の参照先

| 変更したい内容 | 最初に確認するファイル |
|---|---|
| シーン切り替え、起動・終了、ウィンドウタイトル | `main.cpp`、`scene_type.hpp` |
| 編成枠、キオク選択・交換・解除、パラメータ編集 | `formation/formationView.h`、`formation/formationView.cpp` |
| キオクの基本情報や共有型 | `kioku_data.h`、`unit_types.h` |
| 戦闘画面の表示、タイムラインやログの UI | `simulation/simulationView.hpp`、`simulation/simulationView.cpp` |
| ターン進行、行動順、履歴、戦闘状態 | `simulation/simulationLogic.hpp`、`simulation/simulationLogic.cpp` |
| 行動内容の組み立て・Process 登録 | `simulation/ProcessList.hpp`、`simulation/ProcessList.cpp` |
| 効果の追加・効果実行の引数 | `simulation/EffectList.hpp`、`simulation/EffectList.cpp` |
| キオク・属性・ロール画像 | `image_loader.hpp`、`image_loader.cpp`、該当する `embedded_images/` 配下 |
| ウィンドウサイズ | `window_config.h` |
| コンパイル対象やリンク設定 | `Makefile`、`CMakeLists.txt`、必要に応じて `unity.cpp` |

## 文書の保守

この文書は実装構成の概要に限定し、戦闘ルールの詳細や数値仕様は別の仕様文書で管理する。ソースファイルの追加・移動、依存関係の変更、責務の変更があった場合に更新する。関数の細かな実装や頻繁に変わる UI 定数を網羅する一覧にはしない。
