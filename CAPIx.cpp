//CAPIx 2.0

/***********************************************************
* Hook:http://blog.chinaunix.net/uid-660282-id-2414901.html
* Call:http://blog.csdn.net/yhz/article/details/1484073
************************************************************/

//TODO:����if��˳��(Ҳ�����΢����ٶ�)
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


//���ù����干��һ���ڴ��������ʵ��ǿ������ת��
typedef union {
    double _double;
    int    _int[2];
    float  _float[2];
} CAPI_Ret;

#undef memcpy

#pragma comment(lib,"th32.lib")
#pragma comment(linker, "/OPT:nowin98")




// ����SetEnvironmentVariableW����ԭ��
typedef bool  (WINAPI *PFNSETENVIRONMENTVARIABLE)(wchar_t *, wchar_t *);
typedef DWORD (WINAPI *PFNGETENVIRONMENTVARIABLE)(wchar_t *, wchar_t *, DWORD);

bool WINAPI SetCall_CAPI(wchar_t *, wchar_t *);
bool CAPI(wchar_t *);
CAPI_Ret* APIStdCall(void *, int *, int, short);
CAPI_Ret* APICdecl(void *, int *, int, short);

wchar_t* GetVar(wchar_t *);
void SetVar(wchar_t *, int);
void HookAPI(void *, void *);//�̺߳���
void MemPut  (int, wchar_t **);
void MemPrint(int, wchar_t **);
void MemCopy (int, wchar_t **);

char* WcharToChar(wchar_t *);

bool *bakSetEnv  = (bool  *)SetEnvironmentVariableW;     //���溯������ڵ�ַ
DWORD *bakGetEnv = (DWORD *)GetEnvironmentVariableW;
//bool *NewAddr = (bool *)CallCAPI;


//-------------------------------------------------------��������ʼ

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
    //OleInitialize(NULL); //��ʼ��COM���ù���
    
	if (dwReason == DLL_PROCESS_ATTACH)
	{
        HookAPI(SetEnvironmentVariableW, SetCall_CAPI);
		DisableThreadLibraryCalls(hModule);
		/*if(Load())
		{
			//LpkEditControl���������14����Ա�����뽫�临�ƹ���    
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

//��������������,�������
extern "C" DLL_EXPORT int Init(void)
{
    return 0;
}
//�������̵ĺ���


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

            //�޸��ڴ�Ĳ���
            if((*lpAddr) == (unsigned int)OldFunc)
            {
                //�޸��ڴ�ҳ������
                DWORD dwOLD;
                MEMORY_BASIC_INFORMATION mbi;
                VirtualQuery(lpAddr,&mbi,sizeof(mbi));
                VirtualProtect(lpAddr,sizeof(DWORD),PAGE_READWRITE,&dwOLD);

                WriteProcessMemory(GetCurrentProcess(),
                lpAddr, &NewFunc, sizeof(DWORD), NULL);
                //�ָ��ڴ�ҳ������
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

/* �жϱ������Ƿ�ΪCAPI, �������CAPI */
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

/* �������ڴ���Ҫ�ֶ��ͷ�! */
inline char* WcharToChar(wchar_t *wstr)
{
    int len   = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char *str = (char *)malloc(len * sizeof(char));
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
    return str;
}

/* varValue��ռ�ڴ������ǵ��ͷ� */
wchar_t* GetVar(wchar_t *varName)
{
    wchar_t *varValue = (wchar_t *)malloc(8192 * sizeof(wchar_t));
    memset(varValue, 0, 8192 * sizeof(wchar_t));
    GetEnvironmentVariableW(varName, varValue, 8192);

    return varValue;
}

/* ���÷���ֵ */
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
    bool  fromVar = false; //���dst��һ�������ĵ�ַ,��ô�޸����ִ��SetEnvironmentVariable�滻ԭ���ı���ֵ

    if (argv[2][0] == TYPE_INT) {
        dst = (void *)wtoi((wchar_t *)&argv[2][1]);
    } else if (argv[2][0] == TYPE_PTR) {
        fromVar = true;
        dst = (void *)GetVar((wchar_t *)&argv[2][1]);
    } else {
        fprintf(stderr, "ERROR:δ֪��ʶ�� %c", argv[2][0]); //can't exit in case of memory overfloat
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
                step = 0; //double����ռ8���ֽ�,�޷���int����,��������
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
            fprintf(stderr, "ERROR:δ֪��ʶ�� %c", argv[i][0]);
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
        fprintf(stderr, "ERROR:δ֪��ʶ�� %c", argv[2][0]);
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
            step = lstrlenA((char *)SrcMem) + 1; //��Ҫ����'\0'
            free(varName);
            break;
        case TYPE_USTR:
            SetEnvW((wchar_t *)&argv[i][1], (wchar_t *)SrcMem);
            step = 2 * lstrlenW((wchar_t *)SrcMem) + 2; // ��Ҫ����'\0'  Unicodeһ���ַ�2�ֽ�,����*2
            break;
        case TYPE_MOVEP:
            step = wtoi((wchar_t *)&argv[i][1]);
            break;
        default:
            fprintf(stderr, "ERROR:δ֪��ʶ�� %c", argv[i][0]);
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
            free(tmp);*/  //��δ�����bug,��Ϊdst�Ĵ�С����û�дﵽ8192,�����
            free(dst);
            dst = GetVar((wchar_t *)&argv[2][2]);
            char *tmp = WcharToChar((wchar_t *)dst);
            memcpy(dst, tmp, (strlen(tmp) + 1) * sizeof(char));
            free(tmp);
        }
    } else {
        fprintf(stderr, "ERROR:δ֪��ʶ�� %c", argv[2][0]);
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
            free(tmp);*/  //��δ�����bug,��Ϊsrc�Ĵ�С����û�дﵽ8192,�����
            free(src);
            src = GetVar((wchar_t *)&argv[3][2]);
            char *tmp = WcharToChar((wchar_t *)src);
            memcpy(src, tmp, (strlen(tmp) + 1) * sizeof(char));
            free(tmp);
        }
    } else {
        fprintf(stderr, "ERROR:δ֪��ʶ�� %c", argv[3][0]);
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
        if (!wcscmp(argv[2], L"0")) { //dll��Ϊ0�Ļ����ڴ���غ���
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

        int       ArgList[MAX_ARG_NUMBERS]; //���32������,û�н���Խ����(Ӧ�ò���Խ���...)
        void     *ArgList_VarVal[MAX_ARG_NUMBERS]  = {NULL};
        wchar_t  *ArgList_VarName[MAX_ARG_NUMBERS] = {NULL};
        void     *ArgNeedFree[MAX_ARG_NUMBERS]	   = {NULL};

        int i, j;
        int tofree_i  = 0;
        int arglistvar_i = 0;

        //�ж��Ǹ��ݺ��������� ����ֱ�ӽ�����ת��Ϊ��ַ
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
                    ArgList_VarName[arglistvar_i] = &argv[i][1]; //������ʶ��
                    ArgList_VarVal[arglistvar_i]  = GetVar(&ArgList_VarName[arglistvar_i][1]); //ȡ������,������ʶ��

                    switch (argv[i][1]) {
                    case TYPE_INT: //����4�ֽڵ��ڴ汣����ArgList[j]
                        ArgList[j]  = (int)malloc(sizeof(int)); //
                        *(int *)ArgList[j] = wtoi((wchar_t *)ArgList_VarVal[arglistvar_i]);
                        ArgList_VarVal[arglistvar_i]  = (void *)ArgList[j];
                        //ArgNeedFree[tofree_i++]    = (void *)ArgList[j]; //ArgList_VarVal���ͷ�һ�Σ����ͷŻ������
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
                        //fprintf(stderr, "ERROR:δ֪��ʶ�� %c%c", argv[i][0], argv[i][1]);
                        //ArgList[l] = malloc(MAX_LENGTH_OF_ENVVARIABLE * sizeof(wchar_t));
                        break;
                    }
                    ++arglistvar_i;
                    break;
                default:
                    fprintf(stderr, "ERROR:δ֪��ʶ�� %c", argv[i][0]);
                    //goto CLEAN_AND_EXIT; //�������ñ�ǩò�Ƶ�����Ī����BUG?? free(ProcName) ���ǻᱻִ��
                    break;
                }
            }

            // ���ڲ����Ǵ�������ѹջ��,���Եڶ�������Ϊ�����β��ַ
            // ����������Ϊ���鳤��
            CAPI_Ret *capi_ret;
            
            if (argv[1][0] == 'C' || argv[1][0] == 'c') {
                capi_ret = APIStdCall(hProc, &ArgList[j - 1], j, function_type);
                //SetVar(L"CAPI_Ret", j == 0 ? hProc() : APIStdCall(hProc, &ArgList[j - 1], j));
            } else {
                capi_ret = APICdecl(hProc, &ArgList[j - 1], j, function_type);
                //SetVar(L"CAPI_Ret", j == 0 ? hProc() : APICdecl(hProc, &ArgList[j - 1], j));
            }
            
            //���÷���ֵ
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
            for (i = 0; i < tofree_i; ++i) { //�ͷ�������ڴ�
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


//�ĳɺ���ò��Ч���½���
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
        mov ebx, dword ptr [arr]  ;//��arrָ��ĵ�ַ�������б��β��ַ������ebx
        mov ecx, dword ptr [len]  ;//��len��ֵ����ecx����Ϊѭ�����Ʊ���
        dec ecx                   ;//�ݼ�ecx

LOOP1: 

        mov eax, dword ptr [ebx]  ;//���������arr��ebxָ������ݣ������ݼ��ص�eax
        sub ebx, 4                ;//��ebx�����ݵݼ�4��ebxָ���ǰ��һλ��
        push eax                  ;//��eaxѹջ
        dec ecx                   ;//�ݼ�ecx

        jns LOOP1                 ;//���ecx��Ϊ��ֵ������ת��LOOP1:

        call dword ptr [hProc]    ;//����API
        fstp _double;
        mov _low, eax              ;//����ֵ����result
        //mov _high, edx             ;

        mov ebx, dword ptr [len]  ;//��len��ֵ����ebx
        SHL ebx, 2                ;//������λ�����ǿɱ�����Ĵ�С
        //add esp, ebx              ;//�ָ���ջָ�� //API use __stdcall  needn't to add esp
        xor eax, eax              ;//���eax
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
        mov ebx, dword ptr [arr]  ;//��arrָ��ĵ�ַ�������б��β��ַ������ebx
        mov ecx, dword ptr [len]  ;//��len��ֵ����ecx����Ϊѭ�����Ʊ���
        dec ecx                   ;//�ݼ�ecx

LOOP1: 

        mov eax, dword ptr [ebx]  ;//���������arr��ebxָ������ݣ������ݼ��ص�eax
        sub ebx, 4                ;//��ebx�����ݵݼ�4��ebxָ���ǰ��һλ��
        push eax                  ;//��eaxѹջ
        dec ecx                   ;//�ݼ�ecx

        jns LOOP1                 ;//���ecx��Ϊ��ֵ������ת��LOOP1:

        call dword ptr [hProc]    ;//����API
        fstp _double;
        mov _low, eax              ;//����ֵ����result
        //mov _high, edx             ;

        mov ebx, dword ptr [len]  ;//��len��ֵ����ebx
        SHL ebx, 2                ;//������λ�����ǿɱ�����Ĵ�С
        add esp, ebx              ;//�ָ���ջָ�� //API use __stdcall  needn't to add esp
        xor eax, eax              ;//���eax
    }
    
    CAPI_Ret *ret = (CAPI_Ret *)malloc(sizeof(CAPI_Ret));;
    if (type == INT_FUNCTION) {
        ret->_int[0] = _low;
    } else {
        ret->_double = _double;
    }
    return ret;
}
