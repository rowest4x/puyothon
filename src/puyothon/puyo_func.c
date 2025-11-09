#include "puyo_func.h"

// 指定した場所にぷよを設置できるか調べる関数
// ぷよ通基準．まわしも考慮して判定する．
int canPut(int (*board)[ROWS_NUM][COLS_NUM], int col, int rot){
    //死亡時は操作を受け付つけない
    if(board[PUYO][12][3] != EMPTY) return 0;

    //指定範囲外の操作は受け付けない
    if(col <= 0 || col >= COLS_NUM-1) return 0;
    //回転数を0~3にする
    rot = (rot + 40000) % 4;

    //子ぷよが置かれる列を求める
    int col_child = col;
    if(rot == 1) col_child++;
    else if(rot == 3) col_child--;

    //置きたい列が13段目（幽霊ぷよ）まで埋まっていると置けない
    if(board[PUYO][13][col] != EMPTY || board[PUYO][13][col_child] != EMPTY) return 0;
    
    //14段目に軸ぷよは設置できない
    if(rot == 2 && board[PUYO][12][col] != EMPTY) return 0;

    //置きたい列が12段目まで埋まっていて、14段目も埋まっているとき、縦置きは設置できない
    if((rot == 0 || rot == 2) && board[PUYO][12][col] != EMPTY && board[PUYO][14][col]) return 0;

    //ぷよを移動させる向きを求める
    //軸ぷよを1、2列目に置くなら左、4～6列目に置くなら右に移動させる必要がある（ぷよは3列目から降ってくるので）
    int way;
    if(col < 3) way = -1;
    else if(col >= 3) way = 1;

    //壁の高さを調べる
    int col_wall12 = -1;//軸ぷよの経路上で12段まで埋まっている列のうち最も3列目に近い列
    for(int i = col*way; i >= 3*way; i--){
        if(board[PUYO][12][i*way] != EMPTY)
            col_wall12 = i*way;
        if(board[PUYO][13][i*way] != EMPTY)
            return 0; //13段の壁があるときは置けない
    }

    //軸ぷよの経路上で12段まで埋まっている列がなく、子ぷよを置きたい列も埋まっていないとき、置ける
    if(col_wall12 < 0 && board[PUYO][12][col_child] == EMPTY)
        return 1;
    
    //11段まで埋まっている列を足場として使えれば回せる
    int c;
    if(col_wall12 < 0){
        c = col;
    }else{
        c = col_wall12 - way;
    }

    while(board[PUYO][12][c] == EMPTY){
        if(board[PUYO][11][c] != EMPTY)
            return 1;
        c -= way;
    }

    //谷間を利用できるときは回せる
    if(board[PUYO][12][2] != EMPTY && board[PUYO][12][4] != EMPTY)
        return 1;

    return 0;
}

// ぷよを設置する関数．置けないのに実行するとバグる
// 戻り値は設置の際にぷよを落下させた段数
int putPuyo(int (*board)[ROWS_NUM][COLS_NUM], int col, int rot, int parent_puyo, int child_puyo){
    //回転数を0~3にする
    rot = ((rot % 4) + 4) % 4;

    switch (rot){
        case 0:
            if(board[PUYO][12][col] == EMPTY){
                board[PUYO][12][col] = parent_puyo;
                board[PUYO][13][col] = child_puyo;
                if(board[PUYO][11][col] != EMPTY)
                    board[STATE][12][col] = NEW;
            }else{
                board[PUYO][13][col] = parent_puyo;
                board[PUYO][14][col] = child_puyo;
            }
            break;
        case 1:
            board[PUYO][13][col] = parent_puyo;
            board[PUYO][13][col+1] = child_puyo;
            break;
        case 2:
            board[PUYO][12][col] = child_puyo;
            board[PUYO][13][col] = parent_puyo;
            if(board[PUYO][11][col] != EMPTY)
                board[STATE][12][col] = NEW;
            break;
        case 3:
            board[PUYO][13][col] = parent_puyo;
            board[PUYO][13][col-1] = child_puyo;
            break;
        default:
            return -1;
    }
    //ぷよを落とす
    return fallPuyos(board);
}

int countNeighborPuyos(int (*board)[ROWS_NUM][COLS_NUM], int i, int j){
    int puyo = board[PUYO][i][j];
    if(puyo <= 0) return 0;
    if(i >= ROWS_NUM - 2) return 0;

    int count = 0;
    if(i + 1 < ROWS_NUM - 2 && board[PUYO][i+1][j] == puyo) count ++;
    if(board[PUYO][i-1][j] == puyo) count ++;
    if(board[PUYO][i][j+1] == puyo) count ++;
    if(board[PUYO][i][j-1] == puyo) count ++;

    return count;
}

// そのぷよを消すか高速に判断する関数
int isLinkingSeed(int (*board)[ROWS_NUM][COLS_NUM], int i, int j){
    int puyo = board[PUYO][i][j];
    if(puyo <= 0) return 0;

    int count = countNeighborPuyos(board, i, j);

    if(count < 2) return 0; // 隣接する同色ぷよの数が1つ以下の場合は（消せる場合もあるが）ここでは消さない．このぷよが消せる場合は隣接する同色ぷよでisLinkingSeedがTrueになる．
    if(count > 2) return 1; // 隣接する同色ぷよの数が3つ以上の場合は，自分含めて4つ以上つながっていることが確定する

    // 「隣接する同色ぷよの数が2つ」を満たすぷよ同士が隣り合っている場合は消せることが確定する
    if(board[PUYO][i+1][j] == puyo && countNeighborPuyos(board, i+1, j) >=2) return 1;
    if(board[PUYO][i-1][j] == puyo && countNeighborPuyos(board, i-1, j) >=2) return 1;
    if(board[PUYO][i][j+1] == puyo && countNeighborPuyos(board, i, j+1) >=2) return 1;
    if(board[PUYO][i][j-1] == puyo && countNeighborPuyos(board, i, j-1) >=2) return 1;
    
    return 0;
}

int eraseLinkingPuyos(int (*board)[ROWS_NUM][COLS_NUM], int i, int j){
    int puyo = board[PUYO][i][j];
    if(puyo <= 0) return 0;
    board[PUYO][i][j] = EMPTY;
    
    int count = 1;
    if(board[PUYO][i+1][j] == puyo)
        count += eraseLinkingPuyos(board, i+1, j);
    if(board[PUYO][i-1][j] == puyo)
        count += eraseLinkingPuyos(board, i-1, j);
    if(board[PUYO][i][j+1] == puyo)
        count += eraseLinkingPuyos(board, i, j+1);
    if(board[PUYO][i][j-1] == puyo)
        count += eraseLinkingPuyos(board, i, j-1);
    
    return count;
}


int fallPuyos(int (*board)[ROWS_NUM][COLS_NUM]){
    int fall_max = 0;
    for(int col = 1; col < COLS_NUM-1; col++){
        int bottom = -1;
        for(int row = 1; row < ROWS_NUM-1; row++){//最上段のぷよは落とさない
            switch (board[PUYO][row][col]){
                case EMPTY:
                    if(bottom < 0) bottom = row;
                    break;
                case BLOCK:
                    bottom = -1;
                    break;
                default:
                    if(bottom < 0) break;
                    board[PUYO][bottom][col] = board[PUYO][row][col];
                    board[STATE][bottom][col] = NEW;
                    board[PUYO][row][col] = EMPTY;
                    board[STATE][row][col] = IDLE;

                    int fall = row - bottom;
                    if(fall > fall_max) fall_max = fall;

                    bottom++;
                    break;
            }
            
        }
    }
    return fall_max;
}

//連鎖を一つ進め、スコアを返す関数
//chain_numは既に実行された連鎖数
int oneChain(int (*board)[ROWS_NUM][COLS_NUM], int chain_num){
    int total_erased_count = 0;
    int linking_bonus = 0;

    int color_flg[COLOR_NUM];
    for(int i = 0; i < COLOR_NUM; i++) color_flg[i] = 0;

    for(int i = 1; i < ROWS_NUM-2; i++){
        for(int j = 1; j < COLS_NUM-1; j++){
            if(board[STATE][i][j] != NEW) continue;
            if(!isLinkingSeed(board, i, j)) continue;

            int puyo = board[PUYO][i][j];
            int erased_count = eraseLinkingPuyos(board, i, j);
            if(erased_count >= 4){
                total_erased_count += erased_count;

                //色フラグを立てる
                color_flg[puyo-1] = 1;

                //連結ボーナスに加算
                if(erased_count >= 11) linking_bonus += 10;
                else if(erased_count >= 5) linking_bonus += erased_count - 3;
            }
            
        }
    }

    //ひとつも消えてなければスコアは0
    if(total_erased_count <= 0) return 0;

    //連鎖ボーナスを計算
    int chain_bonus = 0;
    if(chain_num <= 3) chain_bonus = 8*(chain_num - 1);
    else chain_bonus = 32*(chain_num - 3);

    //色数ボーナスを計算
    int color_bonus = 0;
    int color_num = 0;
    for(int i = 0; i < COLOR_NUM; i++)
        if(color_flg[i]) color_num++;
    if(color_num >= 2){
        color_bonus = 3;
        for(int i = 3; i <= color_num; i++)
            color_bonus *= 2;
    }

    //ボーナスの計算
    int bonus = chain_bonus + linking_bonus + color_bonus;
    if(bonus == 0) bonus = 1;

    //スコアの計算
    int score = total_erased_count * 10 * bonus;
    return score;
}

//最後まで連鎖を実行する関数
void allChain(int (*board)[ROWS_NUM][COLS_NUM], int *n_chains, int *score){
    fallPuyos(board);

    *n_chains = 0;
    *score = 0;

    while(1){
        int s = oneChain(board, *n_chains+1);

        //スコアが0以下なら連鎖終了
        if(s <= 0){
            return;
        }

        fallPuyos(board);
        (*n_chains)++;
        *score += s;
    }
}