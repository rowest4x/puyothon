#ifndef _PUYO_FUNC_H_
#define _PUYO_FUNC_H_

//配列の大きさ
#define ARRS_NUM 2
#define ROWS_NUM 15
#define COLS_NUM 8

//配列番号
#define PUYO 0
#define STATE 1

//ぷよのID
#define OJAMA -2
#define BLOCK -1
#define EMPTY 0

//ぷよの色数
#define COLOR_NUM 5

//STATE配列の状態
#define IDLE 0
#define NEW 1

int canPut(int (*board)[ROWS_NUM][COLS_NUM], int col, int rot);
int putPuyo(int (*board)[ROWS_NUM][COLS_NUM], int col, int rot, int parent_puyo, int child_puyo);
int eraseLinkingPuyos(int (*board)[ROWS_NUM][COLS_NUM], int i, int j);
int fallPuyos(int (*board)[ROWS_NUM][COLS_NUM]);
int oneChain(int (*board)[ROWS_NUM][COLS_NUM], int chain_num);
void allChain(int (*board)[ROWS_NUM][COLS_NUM], int *n_chains, int *score);

#endif //_PUYO_FUNC_H_