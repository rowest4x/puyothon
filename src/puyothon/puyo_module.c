#define PY_SSIZE_T_CLEAN
#include <Python.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
#include "puyo_func.h"


// PyObjectから配列を取得しboardにポインタを渡す関数（読み取り専用）
int toBoard_ro(PyObject *obj, int (**board)[ROWS_NUM][COLS_NUM]) {
    if (!PyArray_Check(obj)) {
        PyErr_SetString(PyExc_TypeError, "ndarray is required");
        return -1;
    }
    PyArrayObject *arr = (PyArrayObject *)obj;

    /* dtype: int32 */
    if (PyArray_TYPE(arr) != NPY_INT32) {
        PyErr_SetString(PyExc_TypeError, "dtype must be int32");
        return -1;
    }
    /* 次元数: 3 */
    if (PyArray_NDIM(arr) != 3) {
        PyErr_SetString(PyExc_ValueError, "array must be 3D");
        return -1;
    }
    /* 形状チェック */
    npy_intp const *dims = PyArray_DIMS(arr);
    if (dims[0] != ARRS_NUM || dims[1] != ROWS_NUM || dims[2] != COLS_NUM) {
        PyErr_Format(PyExc_ValueError, "shape must be (%d,%d,%d), got (%" NPY_INTP_FMT ",%" NPY_INTP_FMT ",%" NPY_INTP_FMT ")", ARRS_NUM, ROWS_NUM, COLS_NUM, dims[0], dims[1], dims[2]);
        return -1;
    }
    /* 連続 & アライン（※コピー禁止なので必須） */
    unsigned int flags = PyArray_FLAGS(arr);
    if (!(flags & NPY_ARRAY_C_CONTIGUOUS)) {
        PyErr_SetString(PyExc_ValueError, "array must be C-contiguous");
        return -1;
    }
    if (!(flags & NPY_ARRAY_ALIGNED)) {
        PyErr_SetString(PyExc_ValueError, "array must be aligned");
        return -1;
    }

    *board = (int (*)[ROWS_NUM][COLS_NUM])PyArray_DATA(arr);
    return 0;
}

// PyObjectから配列を取得しboardにポインタを渡す関数（読み書き可）
int toBoard_rw(PyObject *obj, int (**board)[ROWS_NUM][COLS_NUM]) {
    if (toBoard_ro(obj, board) != 0)
        return -1;

    /* 書き込み可能か確認（read-only のビューは拒否） */
    PyArrayObject *arr = (PyArrayObject *)obj;
    if (!(PyArray_FLAGS(arr) & NPY_ARRAY_WRITEABLE)) {
        PyErr_SetString(PyExc_ValueError, "array must be writeable");
        return -1;
    }
    return 0;
}

void toBoardForModel(int (*board)[ROWS_NUM][COLS_NUM], float (*x)[COLS_NUM-2][4]){
    memset(x, 0, sizeof(float)*4*(ROWS_NUM-1)*(COLS_NUM-2));
    for (int i = 1; i < ROWS_NUM; i++) {
        for(int j = 1; j < COLS_NUM-1; j++){
            int p = board[PUYO][i][j];
            if(1 <= p && p <= 4){
                x[i-1][j-1][p-1] = 1.0f;
            }
        }
    }
}

static PyObject* cvtBoardForModel(PyObject *self, PyObject *args){
    PyObject *input_array_obj;
    
    if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &input_array_obj)) {
        PyErr_SetString(PyExc_TypeError, "Failed to parse.");
        return NULL;
    }
    
    //配列を取得
    int (*board)[ROWS_NUM][COLS_NUM];
    if(toBoard_ro(input_array_obj, &board) != 0){
        return NULL;
    }

    // 配列の次元とサイズを設定
    npy_intp dims[4] = {1, ROWS_NUM-1, COLS_NUM-2, 4};
    
    // NumPy配列を作成
    PyArrayObject *x = (PyArrayObject*)PyArray_SimpleNew(4, dims, NPY_FLOAT32);

    if (x == NULL) {
        return PyErr_NoMemory();
    }

    PyArray_FILLWBYTE(x, 0);

    float (*x_data)[ROWS_NUM-1][COLS_NUM-2][4] = (float (*)[ROWS_NUM-1][COLS_NUM-2][4])PyArray_DATA(x);

    toBoardForModel(board, x_data[0]);

    return (PyObject*)x;
}

static PyObject* getAbleBoardsForModel(PyObject *self, PyObject *args){
    PyObject *board;
    int parent_puyo, child_puyo;
    if (!PyArg_ParseTuple(args, "O!ii", &PyArray_Type, &board, &parent_puyo, &child_puyo)) {
        PyErr_SetString(PyExc_TypeError, "Failed to parse.");
        return NULL;
    }
    
    //配列を取得
    int (*board_data)[ROWS_NUM][COLS_NUM];
    if(toBoard_ro(board, &board_data) != 0){
        return NULL;
    }

    int able_actions_num = 0;
    int able_actions[22];
    int action_col[22];
    int action_rot[22];
    for(int action = 0; action < 22; action++){
        int a = action;
        if(a >= 3) a++;
        if(a >= 21) a++;
        
        int col = a / 4 + 1;
        int rot = a % 4;

        if(canPut(board_data, col, rot)){
            able_actions[able_actions_num] = action;
            action_col[able_actions_num] = col;
            action_rot[able_actions_num] = rot;
            able_actions_num++;
        }
    }

    // 配列の次元とサイズを設定
    npy_intp result_dims[4] = {able_actions_num, ROWS_NUM-1, COLS_NUM-2, 4};
    npy_intp able_actions_obj_dims[1] = {able_actions_num};
    
    // NumPy配列を作成
    PyArrayObject *result = (PyArrayObject*)PyArray_SimpleNew(4, result_dims, NPY_FLOAT32);
    PyArrayObject *able_actions_obj = (PyArrayObject*)PyArray_SimpleNew(1, able_actions_obj_dims, NPY_INT);
    if (result == NULL) {
        return PyErr_NoMemory();
    }
    float (*result_data)[ROWS_NUM-1][COLS_NUM-2][4] = (float (*)[ROWS_NUM-1][COLS_NUM-2][4])PyArray_DATA(result);
    int *able_actions_data = (int*)PyArray_DATA(able_actions_obj);
    for(int i = 0; i < able_actions_num; i++){
        int tmp_board_data[ARRS_NUM][ROWS_NUM][COLS_NUM];
        memcpy(tmp_board_data, board_data, sizeof(int)*ARRS_NUM*ROWS_NUM*COLS_NUM);
        putPuyo(tmp_board_data, action_col[i], action_rot[i], parent_puyo, child_puyo);
        toBoardForModel(tmp_board_data, result_data[i]);
        able_actions_data[i] = able_actions[i];
    }

    // 2つの配列を含むタプルを作成
    PyObject *tuple = PyTuple_New(2);
    if (!tuple) {
        Py_DECREF(result);
        Py_DECREF(able_actions_obj);
        return NULL;
    }

    // タプルに配列をセット
    if (PyTuple_SetItem(tuple, 0, (PyObject*)result) != 0) {
        Py_DECREF(result);
        Py_DECREF(able_actions_obj);
        Py_DECREF(tuple);
        return NULL;
    }
    if (PyTuple_SetItem(tuple, 1, (PyObject*)able_actions_obj) != 0) {
        // 0番目は既に tuple に所有権が移っているので tuple を片付ければOK
        Py_DECREF(tuple);
        return NULL;
    }

    return tuple;
}

static PyObject* pyPutPuyo(PyObject *self, PyObject *args) {
    PyObject *input_array_obj;
    int parent_puyo, child_puyo;
    int col, rot;
    
    if (!PyArg_ParseTuple(args, "O!iiii", &PyArray_Type, &input_array_obj, &parent_puyo, &child_puyo, &col, &rot)) {
        PyErr_SetString(PyExc_TypeError, "Failed to parse.");
        return NULL;
    }
    
    //配列を取得
    int (*board)[ROWS_NUM][COLS_NUM];
    if(toBoard_rw(input_array_obj, &board) != 0){
        return NULL;
    }

    //置けなかった場合はfalseを返す
    if(!canPut(board, col, rot))
        Py_RETURN_FALSE;
    
    putPuyo(board, col, rot, parent_puyo, child_puyo);

    // 置けた場合はtrueを返す
    Py_RETURN_TRUE;
}

//連鎖処理を1つ進める関数
static PyObject* fall(PyObject *self, PyObject *args) {
    PyObject *input_array_obj;
    if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &input_array_obj)) {
        PyErr_SetString(PyExc_TypeError, "Failed to parse.");
        return NULL;
    }

    //配列を取得
    int (*board)[ROWS_NUM][COLS_NUM];
    if(toBoard_rw(input_array_obj, &board) != 0){
        return NULL;
    }

    
    int fall_max = fallPuyos(board);

    // 連鎖数とスコアをPythonのタプルとして返す
    return Py_BuildValue("i", fall_max);
}

//実際に連鎖を行い、連鎖数とスコアを返す関数
static PyObject* erasePuyo(PyObject *self, PyObject *args) {
    PyObject *input_array_obj;
    int chain_count;
    if (!PyArg_ParseTuple(args, "O!i", &PyArray_Type, &input_array_obj, &chain_count)) {
        PyErr_SetString(PyExc_TypeError, "Failed to parse.");
        return NULL;
    }

    //配列を取得
    int (*board)[ROWS_NUM][COLS_NUM];
    if(toBoard_rw(input_array_obj, &board) != 0){
        return NULL;
    }

    //4つ以上つながったぷよを消す
    int score = oneChain(board, chain_count);

    // スコアを返す
    return Py_BuildValue("i", score);
}

//実際に連鎖を行い、連鎖数とスコアを返す関数
static PyObject* chainAuto(PyObject *self, PyObject *args) {
    PyObject *input_array_obj;
    if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &input_array_obj)) {
        PyErr_SetString(PyExc_TypeError, "Failed to parse.");
        return NULL;
    }

    //配列を取得
    int (*board)[ROWS_NUM][COLS_NUM];
    if(toBoard_rw(input_array_obj, &board) != 0){
        return NULL;
    }

    //最後まで連鎖を実行
    int n_chains, score;
    allChain(board, &n_chains, &score);

    // 連鎖数とスコアをPythonのタプルとして返す
    return Py_BuildValue("(ii)", n_chains, score);
}

static PyObject* makeBoard(PyObject *self, PyObject *args){
    // 配列の次元とサイズを設定
    npy_intp dims[3] = {ARRS_NUM, ROWS_NUM, COLS_NUM};
    
    // NumPy配列を作成（ここでは整数型）
    PyArrayObject *board = (PyArrayObject*)PyArray_SimpleNew(3, dims, NPY_INT32);

    if (board == NULL) {
        return PyErr_NoMemory();
    }

    // 配列のデータを取得し、初期化
    int *data = (int *)PyArray_DATA(board);
    for (int i = 0; i < ROWS_NUM * COLS_NUM; i++) {
        if(i % COLS_NUM == 0 || i % COLS_NUM == COLS_NUM - 1 || i < COLS_NUM)
            data[i] = BLOCK;
        else
            data[i] = EMPTY;
        
        data[ROWS_NUM*COLS_NUM + i] = IDLE;
    }

    return (PyObject*)board;
}

static PyObject* isDead(PyObject *self, PyObject *args) {
    PyObject *input_array_obj;
    if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &input_array_obj)) {
        PyErr_SetString(PyExc_TypeError, "Failed to parse.");
        return NULL;
    }

    //配列を取得
    int (*board)[ROWS_NUM][COLS_NUM];
    if(toBoard_ro(input_array_obj, &board) != 0){
        return NULL;
    }

    int is_dead = (board[PUYO][12][3] != EMPTY);
    
    if(is_dead)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}

//モジュールの作成-------------------------------------------------------------------------------------------------

static int addIntConstants(PyObject *module){
    if (PyModule_AddIntMacro(module, ARRS_NUM) < 0) return -1;
    if (PyModule_AddIntMacro(module, ROWS_NUM) < 0) return -1;
    if (PyModule_AddIntMacro(module, COLS_NUM) < 0) return -1;

    if (PyModule_AddIntMacro(module, PUYO) < 0) return -1;
    if (PyModule_AddIntMacro(module, STATE) < 0) return -1;

    if (PyModule_AddIntMacro(module, OJAMA) < 0) return -1;
    if (PyModule_AddIntMacro(module, BLOCK) < 0) return -1;
    if (PyModule_AddIntMacro(module, EMPTY) < 0) return -1;

    if (PyModule_AddIntMacro(module, COLOR_NUM) < 0) return -1;

    if (PyModule_AddIntMacro(module, IDLE) < 0) return -1;
    if (PyModule_AddIntMacro(module, NEW) < 0) return -1;

    return 0;
}

static PyMethodDef puyo_methods[] = {
    {"cvtBoardForModel",  cvtBoardForModel,  METH_VARARGS, ""},
    {"getAbleBoardsForModel", getAbleBoardsForModel, METH_VARARGS, ""},
    {"putPuyo",           pyPutPuyo,         METH_VARARGS,
        "Put puyos at the specified location and number of rotations."},
    {"fallPuyo",          fall,              METH_VARARGS, "Fall puyos in board."},
    {"erasePuyo",         erasePuyo,         METH_VARARGS,
        "Erase 4 or more connected Puyos and return the score."},
    {"chainAuto",         chainAuto,         METH_VARARGS,
        "Executes chains and returns the number of chains and the score."},
    {"makeBoard",         makeBoard,         METH_NOARGS,  "Make new game board."},
    {"isDead",            isDead,            METH_VARARGS,
        "Return True if player of given board is dead."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef puyo_module = {
    PyModuleDef_HEAD_INIT,
    "puyothon",
    NULL,
    -1,
    puyo_methods
};

PyMODINIT_FUNC PyInit_puyothon(void) {
    PyObject *module = PyModule_Create(&puyo_module);
    if (module == NULL) return NULL;

    import_array();
    if (PyErr_Occurred()) {
        Py_DECREF(module);
        return NULL;
    }

    if (addIntConstants(module) < 0) {
        Py_DECREF(module);
        return NULL;
    }

    return module;
}