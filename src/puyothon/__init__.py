import numpy as np

from .puyothon import (
    cvtBoardForModel as _cvtBoardForModel,
    getAbleBoardsForModel as _getAbleBoardsForModel,
    putPuyo as _putPuyo,
    fallPuyo as _fallPuyo,
    erasePuyo as _erasePuyo,
    chainAuto as _chainAuto,
    makeBoard as _makeBoard,
    isDead as _isDead,
    ARRS_NUM,
    ROWS_NUM,
    COLS_NUM,
    PUYO,
    STATE,
    OJAMA,
    BLOCK,
    EMPTY,
    COLOR_NUM,
    IDLE,
    NEW,
)

def cvtBoardForModel(board:np.ndarray) -> np.ndarray:
    """
    モデルに入力するために盤面を変換する関数.

    Args:
        board (np.ndarray): int32 ndarray, shape (2, 16, 8).

    Returns:
        np.ndarray: float32 ndarray, shape (1, 14, 6, 4)=(1, 段数, 列数, 色数).
                    one-hot(色=4)で内部セルをエンコード．
    """
    return _cvtBoardForModel(board)

def getAbleBoardsForModel(board:np.ndarray, parent_puyo:int, child_puyo:int) -> tuple[np.ndarray, np.ndarray]:
    """
    ぷよを設置できる場所（有効手）を列挙し、ぷよ設置後の盤面を作成する関数
    cvtBoardForModelをすべての有効手に対して行ってくれる

    Args:
        board (np.ndarray): 現在のboard．
        parent_puyo (int): 親ぷよの色ID（1..COLOR_NUM）．
        child_puyo (int): 子ぷよの色ID（1..COLOR_NUM）．

    Returns:
        tuple[np.ndarray, np.ndarray]:
            - result: float32 ndarray, shape (K, 14, 6, 4)
                      各有効手を打った後の盤面を one-hot で表現したテンソル．
            - able_actions: int ndarray, shape (K,)
                      各手のアクション番号（0..21の22通り）．
    """
    return _getAbleBoardsForModel(board, parent_puyo, child_puyo)

def putPuyo(board:np.ndarray, parent_puyo:int, child_puyo:int, col:int, rot:int) -> bool:
    """
    指定した場所にぷよを設置する関数
    
    Args:
        board (np.ndarray): in-place で更新される．
        parent_puyo (int): 親ぷよの色ID．
        child_puyo (int): 子ぷよの色ID．
        col (int): 列（1-origin、左右の壁を除く実列）．
        rot (int): 回転（0, 1, 2, 3）．

    Returns:
        bool: 置けたら True。置けない配置なら False．
    
    """
    return _putPuyo(board, parent_puyo, child_puyo, col, rot)

def fallPuyo(board:np.ndarray) -> int:
    """
    ぷよを下に落とす関数．

    Args:
        board (np.ndarray): in-place で更新される．

    Returns:
        int: 最も落下したぷよの落下段数．
    """
    return _fallPuyo(board)

def erasePuyo(board:np.ndarray, chain_count:int) -> int:
    """
    4つ以上つながったぷよを消す関数．

    Args:
        board (np.ndarray): in-place 更新．
        chain_count (int): この消去を行う前の連鎖数（得点計算に使用）．

    Returns:
        int: このステップで得られたスコア．
    """
    return _erasePuyo(board, chain_count)

def chainAuto(board:np.ndarray) -> tuple[int, int]:
    """
    連鎖を最後まで実行する関数．

    Args:
        board (np.ndarray): in-place 更新．

    Returns:
        tuple[int, int]: (連鎖数, スコア)
    """
    return _chainAuto(board)

def makeBoard() -> np.ndarray:
    """
    新しく空の盤面を作成する

    Returns:
        np.ndarray: int32 ndarray with shape (2, 16, 8)=(puyo/state, 段数, 列数).
                    puyo面とstate面を含む．内部は EMPTY/IDLE で初期化．
                    puyo面は実際の盤面，state面は連鎖の処理に使われる．
                    puyo面の外側は BLOCKになっているため，ぷよぷよの盤面本体は 14 x 6 配列
    """
    return _makeBoard()

def isDead(board:np.ndarray) -> bool:
    """
    ゲームオーバーになっているかを判定する関数

    Args:
        board (np.ndarray): 盤面．

    Returns:
        bool: ゲームオーバーならTrue，そうでないならFalse
    """
    return _isDead(board)


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
    "PUYO",
    "STATE",
    "OJAMA",
    "BLOCK",
    "EMPTY",
    "COLOR_NUM",
    "IDLE",
    "NEW",
]

__version__ = "1.0"