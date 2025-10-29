import numpy as np

# C拡張モジュールをインポート
from .puyothon import (
    makeBoard as _makeBoard,
    cvtBoardForModel as _cvtBoardForModel,
    getAbleBoardsForModel as _getAbleBoardsForModel,
    putPuyo as _putPuyo,
    fallPuyo as _fallPuyo,
    erasePuyo as _erasePuyo,
    chainAuto as _chainAuto,
    isDead as _isDead,
    ARRS_NUM as _ARRS_NUM,
    ROWS_NUM as _ROWS_NUM,
    COLS_NUM as _COLS_NUM,
    COLOR_NUM as _COLOR_NUM,
    PUYO as _PUYO,
    STATE as _STATE,
    OJAMA as _OJAMA,
    BLOCK as _BLOCK,
    EMPTY as _EMPTY,
    IDLE as _IDLE,
    NEW as _NEW,
)


def makeBoard() -> np.ndarray:
    """
    新しく空の盤面を作成する関数.

    Returns:
        np.ndarray: iint32 ndarray, shape = (ARRS_NUM, ROWS_NUM, COLS_NUM) = (2, 15, 8).
                    puyo面とstate面を含む. 内部はそれぞれ EMPTY, IDLE で初期化.
                    puyo面は実際の盤面, state面は連鎖の処理に使われる.
                    puyo面の外側は BLOCKになっているため, ぷよぷよの盤面本体は 14 x 6 配列.
    """
    return _makeBoard()

def cvtBoardForModel(board:np.ndarray) -> np.ndarray:
    """
    モデルに入力するために盤面を変換する関数.

    Args:
        board (np.ndarray): int32 ndarray, shape = (ARRS_NUM, ROWS_NUM, COLS_NUM) = (2, 15, 8).

    Returns:
        np.ndarray: float32 ndarray, shape = (1, ROWS_NUM-1, COLS_NUM-2, COLOR_NUM) = (1, 14, 6, 4).
                    one-hot(色数=4)でpuyo面をエンコード.
    """
    return _cvtBoardForModel(board)

def getAbleBoardsForModel(board:np.ndarray, parent_puyo:int, child_puyo:int) -> tuple[np.ndarray, np.ndarray]:
    """
    ぷよを設置できるすべての有効手を列挙し, 
    各手でぷよを設置した後の盤面をモデル入力に変換する関数.

    Args:
        board (np.ndarray): 現在のboard.int32 ndarray, shape = (ARRS_NUM, ROWS_NUM, COLS_NUM) = (2, 15, 8).
        parent_puyo (int): 親ぷよの色ID (1..COLOR_NUM).
        child_puyo (int): 子ぷよの色ID (1..COLOR_NUM).

    Returns:
        tuple[np.ndarray, np.ndarray]:
            - result: float32 ndarray, shape = (K, ROWS_NUM-1, COLS_NUM-2, COLOR_NUM) = (K, 14, 6, 4).
                      各有効手を打った後の盤面を one-hot で表現したテンソル.
            - able_actions: int ndarray, shape (K,).
                      各手のアクション番号 (0..21の22通り).
    """
    return _getAbleBoardsForModel(board, parent_puyo, child_puyo)

def putPuyo(board:np.ndarray, parent_puyo:int, child_puyo:int, col:int, rot:int) -> bool:
    """
    指定した場所にぷよを設置する関数
    
    Args:
        board (np.ndarray): int32 ndarray, shape = (ARRS_NUM, ROWS_NUM, COLS_NUM) = (2, 15, 8).
        parent_puyo (int): 親ぷよの色ID.
        child_puyo (int): 子ぷよの色ID.
        col (int): 列 (1-origin, 左右の壁を除く実列).
        rot (int): 回転 (0, 1, 2, 3).

    Returns:
        bool: 置けたら True. 置けない配置なら False.
              ぷよが置けるかどうかは「まわし」も考慮され, 「ぷよぷよ通」基準で判断される. 
    
    """
    return _putPuyo(board, parent_puyo, child_puyo, col, rot)

def fallPuyo(board:np.ndarray) -> int:
    """
    ぷよを下に落とす関数.

    Args:
        board (np.ndarray): int32 ndarray, shape = (ARRS_NUM, ROWS_NUM, COLS_NUM) = (2, 15, 8).

    Returns:
        int: 最も落下したぷよの落下段数.
    """
    return _fallPuyo(board)

def erasePuyo(board:np.ndarray, chain_count:int) -> int:
    """
    4つ以上つながったぷよを消す関数.

    Args:
        board (np.ndarray): int32 ndarray, shape = (ARRS_NUM, ROWS_NUM, COLS_NUM) = (2, 15, 8).
        chain_count (int): この消去を行う前の連鎖数 (得点計算に使用).

    Returns:
        int: このステップで得られたスコア.
    """
    return _erasePuyo(board, chain_count)

def chainAuto(board:np.ndarray) -> tuple[int, int]:
    """
    連鎖を最後まで実行する関数.

    Args:
        board (np.ndarray): int32 ndarray, shape = (ARRS_NUM, ROWS_NUM, COLS_NUM) = (2, 15, 8).

    Returns:
        tuple[int, int]: (連鎖数, スコア)
    """
    return _chainAuto(board)

def isDead(board:np.ndarray) -> bool:
    """
    ゲームオーバーになっているかを判定する関数.

    Args:
        board (np.ndarray): int32 ndarray, shape = (ARRS_NUM, ROWS_NUM, COLS_NUM) = (2, 15, 8).

    Returns:
        bool: ゲームオーバーならTrue, そうでないならFalse.
    """
    return _isDead(board)

# ---- 定数 (説明付きラッパー)  ----
class _Const(int):
    """int の派生クラス：定数に docstring を持たせるためのヘルパー"""
    def __new__(cls, value: int, doc: str):
        obj = int.__new__(cls, value)
        obj.__doc__ = doc
        return obj


ARRS_NUM: int = _ARRS_NUM
"""配列の層数. 値は 2 (ぷよ面と状態面)."""

ROWS_NUM: int = _ROWS_NUM
"""盤面の総段数 (壁を含む). 値は 15."""

COLS_NUM: int = _COLS_NUM
"""盤面の総列数 (壁を含む). 値は 8."""

COLOR_NUM: int = _COLOR_NUM
"""使用するぷよの色数. 値は 4."""

PUYO: int = _PUYO
"""盤面配列のうち, ぷよの色を保持する面のインデックス."""

STATE: int = _STATE
"""盤面配列のうち, ぷよの状態 (IDLE, NEW など) を保持する面のインデックス. 
状態面は連鎖の処理に使われる. 
"""

OJAMA: int = _OJAMA
"""おじゃまぷよを表す定数. 値は -2. 
現在はおじゃまぷよの処理は実装されていない. 
"""

BLOCK: int = _BLOCK
"""ブロック (壁) を表す定数. 値は -1. """

EMPTY: int = _EMPTY
"""空マスを表す定数. 値は 0. """

IDLE: int = _IDLE
"""通常状態のぷよを表す状態定数. 値は 0. """

NEW: int = _NEW
"""新しく落下したぷよを表す状態定数. 値は 1. """

__all__ = [
    "cvtBoardForModel",
    "getAbleBoardsForModel",
    "putPuyo",
    "fallPuyo",
    "erasePuyo",
    "chainAuto",
    "makeBoard",
    "isDead",
    "ARRS_NUM",
    "ROWS_NUM",
    "COLS_NUM",
    "COLOR_NUM",
    "PUYO",
    "STATE",
    "OJAMA",
    "BLOCK",
    "EMPTY",
    "IDLE",
    "NEW",
]

__version__ = "1.0"