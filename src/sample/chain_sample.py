import puyothon as puyo

board = puyo.makeBoard()
print(board[0, ::-1, :])

puyo1, puyo2 = 1, 1 # ぷよの色（0～4で指定）
col, rot = 3, 0 # 設置する列と回転（時計回りに90度回転させた回数）
puyo.putPuyo(board, puyo1, puyo2, col, rot)
print(board[0, ::-1, :])

puyo1, puyo2 = 1, 1
col, rot = 3, 0
puyo.putPuyo(board, puyo1, puyo2, col, rot)
print(board[0, ::-1, :])

chain_num, score = puyo.chainAuto(board)
print(board[0, ::-1, :])
print(f"chain_num:{chain_num}, score:{score}")