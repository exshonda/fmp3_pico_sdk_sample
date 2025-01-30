
# TOPPERS/FMP3×pico-sdk

[TOPPERS/FMP3](https://github.com/toppers/fmp3_raspberrypi_pico)を[pico-sdk](https://github.com/raspberrypi/pico-sdk)と使えるようにしたプロジェクトの例です。

[Visual Studio Code](https://code.visualstudio.com/)の[Raspberry Pi Pico 拡張機能](https://github.com/raspberrypi/pico-vscode)で作成したプロジェクトに、`fmp3`を追加することで、マルチコア対応 RTOS API が使えます。

pico-sdk でサポートされるボードで使えます。

## リポジトリのクローン

`fmp3`フォルダはサブモジュールとなっているので、下記のコマンドでサブモジュールもクローンしてください。

```bash
git clone https://github.com/exshonda/fmp3_pico_sdk_sample.git
cd fmp3_pico_sdk_sample
git submodule update --init
```

## ビルド方法

Pico 拡張機能のビルド機能をそのまま使用します。

![Pico 拡張機能](images/pico_extention_compile.png)

## 新規プロジェクトへの適用方法

Pico 拡張機能で作成したプロジェクトに`fmp3`フォルダをビルド対象に含めるよう`CMakeLists.txt`を編集します。

```cmake
# TOPPERS/FMP3 の Raspberry Pi Pico SDK 用のコンパイラ定義をインクルード
include(fmp3/fmp3_pico_sdk.cmake)
```

タスクやセマフォなどを静的APIで定義したcfgファイルを指定します。

```cmake
# TOPPERS/FMP3 のカーネルオブジェクト定義のcfgファイルを設定
set(FMP3_APP_CFG_FILE ${PROJECT_SOURCE_DIR}/fmp3_pico_sdk.cfg)
```

fmp3をサブプロジェクトとして追加します。

- プロジェクトにfmp3フォルダをコピーした場合

    ```cmake
    # TOPPERS/FMP3 のライブラリを追加
    add_subdirectory(fmp3)
    ```

- 別のフォルダに置いた場合

    ```cmake
    # TOPPERS/FMP3 のライブラリを追加
    add_subdirectory(path/to/fmp3 fmp3)
    ```

必要に応じて、FMP3付属のソースファイルを追加します。

```cmake
# TOPPERS/FMP3 付属のソースファイルを追加
include(fmp3/library/library.cmake)
include(fmp3/syssvc/syssvc.cmake)
```

FMP3のライブラリ`fmp3`を追加します。

```cmake
# Add the standard library to the build

target_link_libraries(fmp3_pico_sdk
    pico_stdlib
    pico_multicore
    fmp3
)
```

最後に「`fmp3_set_pico_sdk_options`」関数を呼び出します。

```cmake
# TOPPERS/FMP3 を使うための Raspberry Pi Pico SDK の設定
fmp3_set_pico_sdk_options(fmp3_pico_sdk)
```

TOPPERS/FMP3 で使用する変数が想定通りのメモリ配置になっているかチェックが行えますが、pico-sdk では使用していないシンボルを削除するリンクオプションが付いてているので、チェックは行えないため省略します。

```cmake
# TOPPERS/FMP3 のチェックを行う（シンボルがGCされるとエラーになるので省略）
#fmp3_cfg_check(fmp3_pico_sdk)
```
