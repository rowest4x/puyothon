# puyothon

Python 向けの 高速ぷよぷよ盤面操作ライブラリです。
C 拡張によって実装されており、盤面更新・落下・連鎖判定などを高速に行うことができます。
NumPy 配列で盤面を表現しており、AI 学習に適した構成になっています。

研究用途にのみお使いください。


## インストール方法

gcc 等の C コンパイラが必要です。  
動作確認は python 3.12.3、numpy 2.1.3 で行いました。

```
pip install git+https://github.com/rowest4x/puyothon.git
```


## サンプルプログラム

```python
import numpy as np
import puyothon as puyo

# 盤面の作成
board = puyo.makeBoard()

# 盤面形状を確認
print(board.shape)  # (2, 15, 8)

# ぷよを置いてみる（親ぷよ=1, 子ぷよ=1, 列=3, 回転=0）
puyo.putPuyo(board, 1, 2, col=3, rot=0)

# 落下処理
puyo.fallPuyo(board)

# 連鎖処理
n_chain, score = puyo.chainAuto(board)
print(f"{n_chain} chain, {score} pts")

# モデル入力用テンソルに変換
x = puyo.cvtBoardForModel(board)
print(x.shape)  # (1, 14, 6, 4)
```



## 提供される関数

| 関数名                                           | 説明                              |
| --------------------------------------------- | ------------------------------- |
| `makeBoard()`                                 | 新しい空の盤面を生成します。                  |
| `cvtBoardForModel(board)`                     | 盤面をニューラルネット入力形式（one-hot）に変換します。 |
| `getAbleBoardsForModel(board, parent, child)` | 置ける全手を列挙し、それぞれの盤面をモデル入力に変換します。  |
| `putPuyo(board, parent, child, col, rot)`     | 指定位置にぷよを設置します。                  |
| `fallPuyo(board)`                             | 落下処理を1ステップ行います。                 |
| `erasePuyo(board, chain_count)`               | 4つ以上繋がったぷよを消去します。               |
| `chainAuto(board)`                            | 連鎖が終わるまで自動的に処理します。              |
| `isDead(board)`                               | ゲームオーバー状態かを判定します。               |


## 定数一覧

| 定数名         | 説明                      |
| ----------- | ----------------------- |
| `ARRS_NUM`  | 配列の層数。値は 2（ぷよ面・状態面）。    |
| `ROWS_NUM`  | 盤面の総段数（壁を含む）。値は 15。     |
| `COLS_NUM`  | 盤面の総列数（壁を含む）。値は 8。      |
| `COLOR_NUM` | 使用するぷよの色数。値は 4。         |
| `PUYO`      | ぷよ面インデックス。              |
| `STATE`     | 状態面インデックス。連鎖処理に使用。      |
| `OJAMA`     | おじゃまぷよを表す定数（未実装）。値は -2。 |
| `BLOCK`     | 壁セルを表す定数。値は -1。         |
| `EMPTY`     | 空マスを表す定数。値は 0。          |
| `IDLE`      | 通常状態のぷよ。値は 0。           |
| `NEW`       | 新しく落下したぷよ。値は 1。         |


## 盤面仕様

* 盤面は NumPy の `int32` ndarray
  `shape = (2, 15, 8)`（ぷよ面＋状態面）
* 壁を含めたサイズで、内部盤面は `14 × 6`
* ぷよ色は `1..COLOR_NUM` で表現
* 空白・壁は `EMPTY` / `BLOCK` で表現


## モデル入力変換

`cvtBoardForModel()` は、
内部盤面 `(14 × 6)` を one-hot エンコードして
学習モデルに入力できる `float32 (1, 14, 6, 4)` を返します。

```python
x = puyo.cvtBoardForModel(board)
print(x.shape)  # (1, 14, 6, 4)
```