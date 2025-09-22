# STM32_BMM350

- 地磁気センサ『BMM350』をSTM32HALライブラリで使用するためのサンプルコードです
- そこら辺の人が作った非公式のサンプルです 

## 使用しているライブラリについて

BMM350SensorAPI
https://github.com/boschsensortec/BMM350_SensorAPI

- Core/Lib/BMM350_SensorAPIはこのリポジトリのものを使用しています
- この部分のライセンスは、このリポジトリの記述に従います

- STM32CubeIDEによる自動生成ファイル（ライブラリなど）については
- ST公式のライセンスを参照してください

## 実装環境について

- STM32CubeIDE v1.18.1
- STM32F446RE
- BMM350 (DFROBOT SEN0622)

## 使い方

1. このリポジトリをクローン
2. ```void init()```内の各種設定を入力
3. STMに書き込み
