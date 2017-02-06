//CAPIx 2.0

/***********************************************************
* Hook:http://blog.chinaunix.net/uid-660282-id-2414901.html
* Call:http://blog.csdn.net/yhz/article/details/1484073
************************************************************/

//TODO:调整if的顺序(也许会略微提高速度)
//TODO:int? uint?

#include <windows.h>
#include <process.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <objbase.h>

#define DLL_EXPORT __declspec(dllexport)
#define wtoi _wtoi
#define itow _itow
#define wtof(x) wcstod(x, NULL); 
#define MAX_LENGTH_OF_ENVVARIABLE 8192
#define MAX_ARG_NUMBERS 32

#define TYPE_BYTE   '.'
#define TYPE_SHORT  ':'
#define TYPE_INT    ';'
#define TYPE_FLOAT  '~'
#define TYPE_DOUBLE '`'
#define TYPE_ASTR   '#'
#define TYPE_USTR   '$'
#define TYPE_PTR    '*'
#define TYPE_MOVEP  '@'

#define INT_FUNCTION 1
#define FLOAT_FUNCTION 2
#define DOUBLE_FUNCTION 4

#define CALL_FUNCTION 8
#define EXEC_FUNCTION 16

#define LIB_FROM_MEM (HMODULE)1
//11111

#define SetEnvW ((PFNSETENVIRONMENTVARIABLE)bakSetEnv)
#define GetEnvW ((PFNGETENVIRONMENTVARIABLE)bakGetEnv)


//利用共用体共享一段内存的特性来实现强制类型转换
typedef union {
    double _double;
    int    _int[2];
    float  _float[2];
} CAPI_Ret;

#undef memcpy

#pragma comment(lib,"th32.lib")
#pragma comment(linker, "/OPT:nowin98")




// 定义SetEnvironmentVariableW函数原型
typedef bool  (WINAPI *PFNSETENVIRONMENTVARIABLE)(wchar_t *, wchar_t *);
typedef DWORD (WINAPI *PFNGETENVIRONMENTVARIABLE)(wchar_t *, wchar_t *, DWORD);

bool WINAPI SetCall_CAPI(wchar_t *, wchar_t *);
bool CAPI(wchar_t *);
CAPI_Ret* APIStdCall(void *, int *, int, short);
CAPI_Ret* APICdecl(void *, int *, int, short);

wchar_t* GetVar(wchar_t *);
void SetVar(wchar_t *, int);
void HookAPI(void *, void *);//线程函数
void MemPut  (int, wchar_t **);
void MemPrint(int, wchar_t **);
void MemCopy (int, wchar_t **);

char* WcharToChar(wchar_t *);

bool *bakSetEnv  = (bool  *)SetEnvironmentVariableW;     //保存函数的入口地址
DWORD *bakGetEnv = (DWORD *)GetEnvironmentVariableW;
//bool *NewAddr = (bool *)CallCAPI;


//-------------------------------------------------------主函数开始

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
    //OleInitialize(NULL); //初始化COM调用功能
    
	if (dwReason == DLL_PROCESS_ATTACH)
	{
        HookAPI(SetEnvironmentVariableW, SetCall_CAPI);
		DisableThreadLibraryCalls(hModule);
		/*if(Load())
		{
			//LpkEditControl这个数组有14个成员，必须将其复制过来    
			//memcpy((LPVOID)(LpkEditControl+1), (LPVOID)((int*)GetAddress("LpkEditControl") + 1),52);   
			//_beginthread(Init,NULL,NULL);
		}
		else
			return FALSE;*/
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		//Free();
	}
	return TRUE;
}

//随便给个导出函数,方便加载
extern "C" DLL_EXPORT int Init(void)
{
    return 0;
}
//结束进程的函数


void HookAPI(void *OldFunc, void *NewFunc)
{
    PIMAGE_DOS_HEADER pDosHeader;
    PIMAGE_NT_HEADERS pNTHeaders;
    PIMAGE_OPTIONAL_HEADER    pOptHeader;
    PIMAGE_IMPORT_DESCRIPTOR  pImportDescriptor;
    PIMAGE_THUNK_DATA         pThunkData;
    //PIMAGE_IMPORT_BY_NAME     pImportByName;
    HMODULE hMod;

    //------------hook api----------------
    hMod = GetModuleHandle(NULL);

    pDosHeader = (PIMAGE_DOS_HEADER)hMod;
    pNTHeaders = (PIMAGE_NT_HEADERS)((BYTE *)hMod + pDosHeader->e_lfanew);
    pOptHeader = (PIMAGE_OPTIONAL_HEADER)&(pNTHeaders->OptionalHeader);

    pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE *)hMod + pOptHeader->DataDirectory[1].VirtualAddress);

    while(pImportDescriptor->FirstThunk)
    {
        //char * dllname = (char *)((BYTE *)hMod + pImportDescriptor->Name);

        pThunkData = (PIMAGE_THUNK_DATA)((BYTE *)hMod + pImportDescriptor->OriginalFirstThunk);

        int no = 1;
        while(pThunkData->u1.Function)
        {
            //char * funname = (char *)((BYTE *)hMod + (DWORD)pThunkData->u1.AddressOfData + 2);
            PDWORD lpAddr = (DWORD *)((BYTE *)hMod + (DWORD)pImportDescriptor->FirstThunk) +(no-1);

            //修改内存的部分
            if((*lpAddr) == (unsigned int)OldFunc)
            {
                //修改内存页的属性
                DWORD dwOLD;
                MEMORY_BASIC_INFORMATION mbi;
                VirtualQuery(lpAddr,&mbi,sizeof(mbi));
                VirtualProtect(lpAddr,sizeof(DWORD),PAGE_READWRITE,&dwOLD);

                WriteProcessMemory(GetCurrentProcess(),
                lpAddr, &NewFunc, sizeof(DWORD), NULL);
                //恢复内存页的属性
                VirtualProtect(lpAddr,sizeof(DWORD),dwOLD,0);
            }
            //---------
            no++;
            pThunkData++;
        }

        pImportDescriptor++;
    }
    //-------------------HOOK END-----------------
}

/* 判断变量名是否为CAPI, 是则调用CAPI */
bool WINAPI SetCall_CAPI(wchar_t *varName, wchar_t *varValue)
{
    if (!wcsicmp(varName, L"CAPI")) {
        
        CAPI(varValue);
    } else {
        
        SetEnvW(varName, varValue);
    }
    return true;
}

DWORD WINAPI GetCall_CAPI(wchar_t *varName, wchar_t* varValue, DWORD size)
{
    if (!wcsnicmp(varName, L"CAPI", 4)) { //4 || 8 ??
        CAPI(varName + 5);
        wchar_t *ret = GetVar(L"CAPI_Ret");
        wcscpy(varValue, ret);
        free(ret);
        return lstrlenW(varValue);
    }
    return GetEnvW(varName, varValue, size);
}

/* 用完用内存需要手动释放! */
inline char* WcharToChar(wchar_t *wstr)
{
    int len   = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *str = (char *)malloc(len * sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
    return str;
}

/* varValue所占内存用完后记得释放 */
wchar_t* GetVar(wchar_t *varName)
{
    wchar_t *varValue = (wchar_t *)malloc(8192 * sizeof(wchar_t));
    memset(varValue, 0, 8192 * sizeof(wchar_t));
    GetEnvironmentVariableW(varName, varValue, 8192);

    return varValue;
}

/* 设置返回值 */
inline void SetVar(wchar_t *varName, int _varValue)
{
    wchar_t *varValue = (wchar_t *)malloc(64 * sizeof(wchar_t));
    itow(_varValue, varValue, 10);
    SetEnvW(varName, varValue);
    free(varValue);

    return;
}

inline void SetVar(wchar_t *varName, double _varValue)
{
    wchar_t *varValue = (wchar_t *)malloc(64 * sizeof(wchar_t));
    swprintf(varValue, L"%f", _varValue);
    SetEnvW(varName, varValue);
    free(varValue);
}

inline void MemPut(int argc, wchar_t *argv[])
{
    void *dst = NULL;
    bool  fromVar = false; //如果dst是一个变量的地址,那么修改完后执行SetEnvironmentVariable替换原来的变量值

    if (argv[2][0] == TYPE_INT) {
        dst = (void *)wtoi((wchar_t *)&argv[2][1]);
    } else if (argv[2][0] == TYPE_PTR) {
        fromVar = true;
        dst = (void *)GetVar((wchar_t *)&argv[2][1]);
    } else {
        fprintf(stderr, "ERROR:未知标识符 %c", argv[2][0]); //can't exit in case of memory overfloat
        return;
    }
    void *dstbak = dst;

    int i;
    int step = 0;

    int   i_data;
    void *p_data = NULL;
    bool can_free = false;
    for (i = 3; i < argc; ++i) {
        can_free = false;
        //printf("%d\n", (int)dst);
        switch (argv[i][0]) {
        case TYPE_BYTE:
            step = 1;
            i_data = (char)wtoi((wchar_t *)&argv[i][1]);
            p_data = &i_data;
            break;
        case TYPE_SHORT:
            step = 2;
            i_data = (short)wtoi((wchar_t *)&argv[i][1]);
            p_data = &i_data;
            break;
        case TYPE_INT:
            step = 4;
            i_data = wtoi((wchar_t *)&argv[i][1]);
            p_data = &i_data;
            break;
        case TYPE_FLOAT:
            {
                step = 4;
                float tmp = (float)wtof((wchar_t *)&argv[i][1]);
                i_data = *(int *)&tmp;
                p_data = &i_data;
            }
            break;
        case TYPE_DOUBLE:
            {
                step = 0; //double类型占8个字节,无法用int储存,单独处理
                double tmp = wtof((wchar_t *)&argv[i][1]);
                memcpy(dst, &tmp, 8);
                dst = (void *)((int)dst + 8);
            }
            break;
        case TYPE_ASTR:
            can_free = true;
            p_data = WcharToChar((wchar_t *)&argv[i][1]);
            step   = lstrlenA((char *)p_data);
            break;
        case TYPE_USTR:
            p_data = &argv[i][1];
            step   = 2 * lstrlenW((wchar_t *)&argv[i][1]);
            break;
        case TYPE_PTR:
            can_free = true;
            p_data = GetVar((wchar_t *)&argv[i][1]);
            step   = 2 * lstrlenW((wchar_t *)p_data);
            break;
        case TYPE_MOVEP:
            step  = 0;
            dst = (void *)((int)dst + wtoi((wchar_t *)&argv[i][1]));
            break;
        default:
            fprintf(stderr, "ERROR:未知标识符 %c", argv[i][0]);
            // goto CLEAN_AND_EXIT;
            break;
        }

        if (step != 0) {
            memcpy(dst, p_data, step);
            if (can_free) {
                free(p_data);
            }
            dst = (void *)((int)dst + step);
        }

    }
    //CLEAN_AND_EXIT:
    if (fromVar) {
        SetEnvW((wchar_t *)&argv[2][1], (wchar_t *)dstbak);
        free(dstbak);
    }

    return;
}

void MemPrint(int argc, wchar_t *argv[])
{
    void *SrcMem = NULL;
    bool SrcFromVar = false;
    if (argv[2][0] == TYPE_INT) {
        SrcMem = (void *)wtoi((wchar_t *)&argv[2][1]);
    } else if (argv[2][0] == TYPE_PTR) {
        SrcFromVar = true;
        SrcMem = (void *)GetVar((wchar_t *)&argv[2][1]);
    } else {
        fprintf(stderr, "ERROR:未知标识符 %c", argv[2][0]);
        return;
    }
    void *dstbak = SrcMem;

    int i, step = 0;
    char *varName = NULL;
    for (i = 3; i < argc; ++i) {
        step = 0;
        switch (argv[i][0]) {
        case TYPE_BYTE:
            SetVar((wchar_t *)&argv[i][1], *(char *)SrcMem);
            step = 1;
            break;
        case TYPE_SHORT:
            SetVar((wchar_t *)&argv[i][1], *(short *)SrcMem);
            step = 2;
            break;
        case TYPE_INT:
            SetVar((wchar_t *)&argv[i][1], *(int *)SrcMem);
            step = 4;
            break;
        case TYPE_FLOAT:
            SetVar((wchar_t *)&argv[i][1], *(float *)SrcMem);
            step = 4;
            break;
        case TYPE_DOUBLE:
            SetVar((wchar_t *)&argv[i][1], *(double *)SrcMem);
            step = 8;
            break;
        case TYPE_ASTR:
            varName = WcharToChar((wchar_t *)&argv[i][1]);
            SetEnvironmentVariableA(varName, (char *)SrcMem);
            step = lstrlenA((char *)SrcMem) + 1; //不要忘了'\0'
            free(varName);
            break;
        case TYPE_USTR:
            SetEnvW((wchar_t *)&argv[i][1], (wchar_t *)SrcMem);
            step = 2 * lstrlenW((wchar_t *)SrcMem) + 2; // 不要忘了'\0'  Unicode一个字符2字节,所以*2
            break;
        case TYPE_MOVEP:
            step = wtoi((wchar_t *)&argv[i][1]);
            break;
        default:
            fprintf(stderr, "ERROR:未知标识符 %c", argv[i][0]);
            //goto CLEAN_AND_EXIT;
            break;
        }
        SrcMem = (void *)((int)SrcMem + step);
    }
    //CLEAN_AND_EXIT:
    if (SrcFromVar) {
        free(dstbak);
    }
}

void MemCopy(int argc, wchar_t *argv[])
{
    void *dst = NULL;
    void *src = NULL;
    bool dst_fromVar = false;
    bool src_fromVar = false;

    if (argv[2][0] == TYPE_INT) {
        dst = (void *)wtoi((wchar_t *)&argv[2][1]);
    } else if (argv[2][0] == TYPE_PTR) {
        dst_fromVar = true;
        dst = (void *)GetVar((wchar_t *)&argv[2][1]);
        if (argv[2][1] == TYPE_ASTR) {
            /*free(dst);
            wchar_t *tmp = GetVar((wchar_t *)&argv[2][2]);
            dst = WcharToChar(tmp);
            free(tmp);*/  //这段代码有bug,因为dst的大小可能没有达到8192,会溢出
            free(dst);
            dst = GetVar((wchar_t *)&argv[2][2]);
            char *tmp = WcharToChar((wchar_t *)dst);
            memcpy(dst, tmp, (strlen(tmp) + 1) * sizeof(char));
            free(tmp);
        }
    } else {
        fprintf(stderr, "ERROR:未知标识符 %c", argv[2][0]);
        return;
    }

    if (argv[3][0] == TYPE_INT) {
        src = (void *)wtoi((wchar_t *)&argv[3][1]);
    } else if (argv[3][0] == TYPE_PTR) {
        src_fromVar = true;
        src = (void *)GetVar((wchar_t *)&argv[3][1]);
        if (argv[3][1] == TYPE_ASTR) {
            /*free(src);
            wchar_t *tmp = GetVar((wchar_t *)&argv[3][2]);
            src = WcharToChar(tmp);
            free(tmp);*/  //这段代码有bug,因为src的大小可能没有达到8192,会溢出
            free(src);
            src = GetVar((wchar_t *)&argv[3][2]);
            char *tmp = WcharToChar((wchar_t *)src);
            memcpy(src, tmp, (strlen(tmp) + 1) * sizeof(char));
            free(tmp);
        }
    } else {
        fprintf(stderr, "ERROR:未知标识符 %c", argv[3][0]);
        return;
    }
    if (argc == 5) {
        memcpy(dst, src, wtoi(argv[4]));
    } else {
        memcpy(dst, (void *)((int)src + wtoi(argv[4])), wtoi(argv[5]));
    }


    if (dst_fromVar) {
        if (argv[2][1] == TYPE_ASTR) {
            char *tmpname = WcharToChar((wchar_t *)&argv[2][2]);
            SetEnvironmentVariableA(tmpname, (char *)dst);
            free(tmpname);
        } else {
            SetEnvW((wchar_t *)&argv[2][1], (wchar_t *)dst);
        }
        free(dst);
    }
    if (src_fromVar)
    free(src);
}

void APICallAndExec(int argc, wchar_t *argv[])
{
    HMODULE hLib = GetModuleHandleW(argv[2]);
    if (!hLib) {
        hLib = LoadLibraryW(argv[2]);
        if (!wcscmp(argv[2], L"0")) { //dll名为0的话从内存加载函数
            //printf("s");
            hLib = LIB_FROM_MEM;
        }
    }

    int *hProc;
    if (hLib) {
        char *ProcName;
        short function_type = INT_FUNCTION;
        switch (argv[3][0]) {
        case TYPE_FLOAT:
            ProcName = WcharToChar((wchar_t *)&argv[3][1]);
            function_type = FLOAT_FUNCTION;
            break;
        case TYPE_DOUBLE:
            ProcName = WcharToChar((wchar_t *)&argv[3][1]);
            function_type = DOUBLE_FUNCTION;
            break;
        default:
            ProcName = WcharToChar(argv[3]);
            break;
        }

        char *tmp;

        int       ArgList[MAX_ARG_NUMBERS]; //最多32个参数,没有进行越界检查(应该不会越界吧...)
        void     *ArgList_VarVal[MAX_ARG_NUMBERS]  = {NULL};
        wchar_t  *ArgList_VarName[MAX_ARG_NUMBERS] = {NULL};
        void     *ArgNeedFree[MAX_ARG_NUMBERS]	   = {NULL};

        int i, j;
        int tofree_i  = 0;
        int arglistvar_i = 0;

        //判断是根据函数名加载 还是直接将参数转化为地址
        hProc    = (int)hLib == 1 ? (int *)wtoi(argv[3]) : (int *)GetProcAddress(hLib, ProcName);
        //printf("proc:%d\n", hProc);
        if (hProc != NULL) {

            for (i = 4, j = 0; i < argc; ++i, ++j) {

                switch (argv[i][0]) {
                case TYPE_INT:
                    ArgList[j] = wtoi((wchar_t *)&argv[i][1]);
                    break;
                case TYPE_FLOAT:
                    { //in this case can i use float_arg
                        float float_arg = (float)wtof((wchar_t *)&argv[i][1]);
                        ArgList[j] = *(int *)&float_arg;
                    }
                    break;
                case TYPE_DOUBLE:
                    union {
                        double db;
                        int i[2];
                    } double_int;
                    double_int.db = wtof((wchar_t *)&argv[i][1]);
                    //printf("f:%f\n", double_int.db);
                    //printf("f:%d-%d\n", double_int.i[0], double_int.i[1]);
                    ArgList[j] = double_int.i[0];
                    ArgList[j + 1] = double_int.i[1];
                    ++j;
                    break;
                case TYPE_USTR:
                    ArgList[j] = (int)&argv[i][1];
                    break;
                case TYPE_ASTR:
                    ArgList[j]      = (int)WcharToChar(&argv[i][1]);
                    ArgNeedFree[tofree_i++] = (void *)ArgList[j];
                    break;
                case TYPE_PTR:
                    ArgList_VarName[arglistvar_i] = &argv[i][1]; //保留标识符
                    ArgList_VarVal[arglistvar_i]  = GetVar(&ArgList_VarName[arglistvar_i][1]); //取变量名,丢弃标识符

                    switch (argv[i][1]) {
                    case TYPE_INT: //申请4字节的内存保存在ArgList[j]
                        ArgList[j]  = (int)malloc(sizeof(int)); //
                        *(int *)ArgList[j] = wtoi((wchar_t *)ArgList_VarVal[arglistvar_i]);
                        ArgList_VarVal[arglistvar_i]  = (void *)ArgList[j];
                        //ArgNeedFree[tofree_i++]    = (void *)ArgList[j]; //ArgList_VarVal会释放一次，再释放会出问题
                        break;                   
                        
                    case TYPE_ASTR:
                        tmp = WcharToChar((wchar_t *)ArgList_VarVal[arglistvar_i]);
                        memcpy(ArgList_VarVal[arglistvar_i], tmp, (strlen(tmp) + 1) * sizeof(char));
                        free(tmp);
                        ArgList[j] = (int)ArgList_VarVal[arglistvar_i];
                        break;
                        
                    case TYPE_USTR:
                        ArgList[j] = (int)ArgList_VarVal[arglistvar_i];
                        break;
                        
                    default:
                        argv[i][0] = TYPE_USTR;
                        free(ArgList_VarVal[arglistvar_i]);
                        ArgList_VarName[arglistvar_i] = &argv[i][0];
                        ArgList_VarVal[arglistvar_i]  = GetVar(&ArgList_VarName[arglistvar_i][1]);
                        ArgList[j] = (int)ArgList_VarVal[arglistvar_i];
                        //fprintf(stderr, "ERROR:未知标识符 %c%c", argv[i][0], argv[i][1]);
                        //ArgList[l] = malloc(MAX_LENGTH_OF_ENVVARIABLE * sizeof(wchar_t));
                        break;
                    }
                    ++arglistvar_i;
                    break;
                default:
                    fprintf(stderr, "ERROR:未知标识符 %c", argv[i][0]);
                    //goto CLEAN_AND_EXIT; //随意设置标签貌似导致了莫名的BUG?? free(ProcName) 总是会被执行
                    break;
                }
            }

            // 由于参数是从右往左压栈的,所以第二个参数为数组的尾地址
            // 第三个参数为数组长度
            CAPI_Ret *capi_ret;
            
            if (argv[1][0] == 'C' || argv[1][0] == 'c') {
                capi_ret = APIStdCall(hProc, &ArgList[j - 1], j, function_type);
                //SetVar(L"CAPI_Ret", j == 0 ? hProc() : APIStdCall(hProc, &ArgList[j - 1], j));
            } else {
                capi_ret = APICdecl(hProc, &ArgList[j - 1], j, function_type);
                //SetVar(L"CAPI_Ret", j == 0 ? hProc() : APICdecl(hProc, &ArgList[j - 1], j));
            }
            
            //设置返回值
            SetVar(L"CAPI_Err", (int)GetLastError());
            
            switch (function_type) {
            case INT_FUNCTION:
                SetVar(L"CAPI_Ret", capi_ret->_int[0]);
                break;
            case FLOAT_FUNCTION:
            case DOUBLE_FUNCTION:
                SetVar(L"CAPI_Ret", capi_ret->_double);
                break;
            }

            //CLEAN_AND_EXIT:
            free(capi_ret);
            
            for (i = 0; i < arglistvar_i; ++i) {
                switch (ArgList_VarName[i][0]) {
                case TYPE_INT:
                    SetVar(&ArgList_VarName[i][1], *((int *)ArgList_VarVal[i]));
                    break;
                case TYPE_USTR:
                    SetEnvW(&ArgList_VarName[i][1], (wchar_t *)ArgList_VarVal[i]);
                    break;
                case TYPE_ASTR:
                    tmp = WcharToChar(&ArgList_VarName[i][1]);
                    SetEnvironmentVariableA(tmp, (char *)ArgList_VarVal[i]);
                    free(tmp);
                    break;
                }

                //free(ArgList_VarName[i]);
                free(ArgList_VarVal[i]);
            }
            for (i = 0; i < tofree_i; ++i) { //释放申请的内存
                free(ArgNeedFree[i]);
            }

        } else {
            fprintf(stderr, "[ERROR]cannot load API %S\n", argv[3]);
        }
        free(ProcName);
    } else {
        fprintf(stderr, "[ERROR]cannot load DLL %S\n", argv[2]);
    }
}


//改成函数貌似效率下降了
bool CAPI(wchar_t *CmdLine)
{
    int argc;
    wchar_t **argv;

    argv = CommandLineToArgvW(CmdLine, &argc);
    if (argc <= 1) {
        return false;
    }


    if (!wcsicmp(argv[0], L"API") && (!wcsicmp(argv[1], L"Call") || !wcsicmp(argv[1], L"Exec"))) {
        APICallAndExec(argc, argv);
    } else if (!wcsicmp(argv[0], L"Mem")) { //Mem
        if (!wcsicmp(argv[1], L"Alloc")) {
            int sz = wtoi(argv[2]);
            int lp = (int)LocalAlloc(LPTR, sz);
            memset((void *)lp, 0, sz);
            SetVar(L"CAPI_Ret", lp);
        } else if (!wcsicmp(argv[1], L"Free")) { //Mem Free
            LocalFree((void *)wtoi(argv[2]));
        } else if (!wcsicmp(argv[1], L"Put")) {  //Mem Put
            MemPut(argc, argv);
        } else if (!wcsicmp(argv[1], L"Print")) { //Mem Print
            MemPrint(argc, argv);
        } else if (!wcsicmp(argv[1], L"Copy")) { //Mem Copy
            MemCopy(argc, argv);
        }
    } else if (!wcsicmp(argv[0], L"Com")) {
        //com(argc, argv);
    } else if (!wcsicmp(argv[0], L"Var")) {
        if (!wcsicmp(argv[1], L"SetCall")) {
            if (!wcsicmp(argv[2], L"Enable")) {
                HookAPI(SetEnvironmentVariableW, SetCall_CAPI);
            } else if (!wcsicmp(argv[2], L"Disable")) {
                HookAPI(SetEnvironmentVariableW, bakSetEnv);
            }
        } else if (!wcsicmp(argv[1], L"GetCall")) {
            if (!wcsicmp(argv[2], L"Enable")) {
                HookAPI(GetEnvironmentVariableW, GetCall_CAPI);
            } else if (!wcsicmp(argv[2], L"Disable")) {
                HookAPI(GetEnvironmentVariableW, bakGetEnv);
            }
        }
    } else if (!wcsicmp(argv[0], L"CAPIDll")) {
        if (!wcsicmp(argv[1], L"/?")) {
            printf(
            "\nCAPIx.dll (ver 2.0)\n"
            "License:LGPL v3+\n"
            "Compiled By VC++ 6.0\n"
            "Code By aiwozhonghuaba\n\n"
            );
        } else if (!wcsicmp(argv[1], L"Ver")) {
            SetEnvironmentVariableA("CAPI_Ret", "2.0");
        }
    }
    LocalFree(argv);
    return true;
}


CAPI_Ret* APIStdCall(void *hProc, int *arr, int len, short type)
{
    //int _high;
    int _low;
    double _double ;
    __asm
    {
        mov ebx, dword ptr [arr]  ;//把arr指向的地址（参数列表的尾地址）放入ebx
        mov ecx, dword ptr [len]  ;//把len的值放入ecx，作为循环控制变量
        dec ecx                   ;//递减ecx

LOOP1: 

        mov eax, dword ptr [ebx]  ;//倒序把数组arr（ebx指向的内容）的内容加载到eax
        sub ebx, 4                ;//把ebx的内容递减4（ebx指向的前移一位）
        push eax                  ;//把eax压栈
        dec ecx                   ;//递减ecx

        jns LOOP1                 ;//如果ecx不为负值，则跳转到LOOP1:

        call dword ptr [hProc]    ;//调用API
        fstp _double;
        mov _low, eax              ;//返回值存入result
        //mov _high, edx             ;

        mov ebx, dword ptr [len]  ;//把len的值放入ebx
        SHL ebx, 2                ;//左移两位，这是可变参数的大小
        //add esp, ebx              ;//恢复堆栈指针 //API use __stdcall  needn't to add esp
        xor eax, eax              ;//清空eax
    }
    
    CAPI_Ret *ret = (CAPI_Ret *)malloc(sizeof(CAPI_Ret));;
    if (type == INT_FUNCTION) {
        ret->_int[0] = _low;
    } else {
        ret->_double = _double;
    }
    return ret;
}

CAPI_Ret* APICdecl(void *hProc, int *arr, int len, short type)
{
    //int _high;
    int _low;
    double _double ;
    __asm
    {
        mov ebx, dword ptr [arr]  ;//把arr指向的地址（参数列表的尾地址）放入ebx
        mov ecx, dword ptr [len]  ;//把len的值放入ecx，作为循环控制变量
        dec ecx                   ;//递减ecx

LOOP1: 

        mov eax, dword ptr [ebx]  ;//倒序把数组arr（ebx指向的内容）的内容加载到eax
        sub ebx, 4                ;//把ebx的内容递减4（ebx指向的前移一位）
        push eax                  ;//把eax压栈
        dec ecx                   ;//递减ecx

        jns LOOP1                 ;//如果ecx不为负值，则跳转到LOOP1:

        call dword ptr [hProc]    ;//调用API
        fstp _double;
        mov _low, eax              ;//返回值存入result
        //mov _high, edx             ;

        mov ebx, dword ptr [len]  ;//把len的值放入ebx
        SHL ebx, 2                ;//左移两位，这是可变参数的大小
        add esp, ebx              ;//恢复堆栈指针 //API use __stdcall  needn't to add esp
        xor eax, eax              ;//清空eax
    }
    
    CAPI_Ret *ret = (CAPI_Ret *)malloc(sizeof(CAPI_Ret));;
    if (type == INT_FUNCTION) {
        ret->_int[0] = _low;
    } else {
        ret->_double = _double;
    }
    return ret;
}
