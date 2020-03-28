// IsaacBugDocRedir.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <fileapi.h>
#include <wchar.h>

//#define DIRLINK_CHAR "mklink /D \"%s\" \"%s\""
#define LOCAL_LEN 2048

#define GAME_MOD_PATH L"/Documents/My Games/Binding of Isaac Afterbirth+ Mods"

inline int putsnb(const char * str) {
	return printf("%s", str);
}
WCHAR boxBuffer[8192];
void PutBuffer(LPCWSTR text) {
	wcscat(boxBuffer, text);
}

#define bprintf(fmt, ... ) do{ WCHAR buf[8192];wsprintf(buf,TEXT(fmt), __VA_ARGS__);PutBuffer(buf); }while(0) 


int ShowBox(LPCWSTR title, UINT uType) {
	int r =
		MessageBox(NULL, boxBuffer, title, uType);
	boxBuffer[0] = '\0';
	return r;
}

typedef
BOOL WINAPI GetUserProfileDirectoryA_Type(
	_In_                            HANDLE  hToken,
	_Out_writes_opt_(*lpcchSize)    LPSTR lpProfileDir,
	_Inout_                         LPDWORD lpcchSize);

BOOLEAN makeLink(const WCHAR * link, const WCHAR * target) {
	/*
	char buff[4096];
	sprintf_s(buff, DIRLINK_CHAR, link, target);

	system(buff);
	*/
	return CreateSymbolicLinkW(link, target, SYMBOLIC_LINK_FLAG_DIRECTORY);
}

#define ENSURE(x,errinfo,errinfoL) if(!(x)) {fputs(errinfo,stderr);PutBuffer(errinfoL);ShowBox(L"错误",MB_ICONERROR);}else

BOOL GetDocDir(char buff[], DWORD size) {
	/*
	strcpy_s(buff, 2048,"C:\\TEST\\中文测试\\中文2\\中文3");
	return TRUE;
	*/
	BOOL retValue = FALSE;
	HMODULE dllmoudle = LoadLibraryA("userenv.dll");
	GetUserProfileDirectoryA_Type * GetDir;
	ENSURE(dllmoudle != NULL, "LoadLibrary \"userenv.dll\" failed\n",L"不能加载动态链接库\"userenv.dll\"") {
		GetDir = (GetUserProfileDirectoryA_Type *)GetProcAddress(dllmoudle, "GetUserProfileDirectoryA");
		ENSURE(GetDir != NULL, "Get function \"GetUserProfileDirectoryA\" failed\n",L"动态链接库userenv.dll中没有符号GetUserProfileDirectoryA") {
			HANDLE pToken;
			OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &pToken);
			retValue = GetDir(pToken, buff, &size);
			CloseHandle(pToken);
			ENSURE(retValue, "GetDirFailed!\n",L"获取文档目录失败，请关注控制台上打印的目录编号") {}
			if (retValue == FALSE) {
				printf("%ld", GetLastError());
			}
		}
		FreeLibrary(dllmoudle);
	}
	return retValue;

}

void CharToWCharBuggy(int len, const char * ch, WCHAR * wch) {
	for (int i = 0; i < len; i++) {
		if (ch[i] == '\0') {
			wch[i] = ch[i];
			return;
		}
		wch[i] = 0xFF & ch[i];
	}
}

void CharToWCharNormal(int len, const char * ch, WCHAR * wch) {
	MultiByteToWideChar(CP_ACP, 0, ch, -1, wch, len);
}
void PrintWChar(const WCHAR * wch) {
	char buff[LOCAL_LEN];
	char defch = '?';
	BOOL notcomplete = FALSE;
	WideCharToMultiByte(CP_ACP, 0, wch, -1, buff, LOCAL_LEN, &defch, &notcomplete);
	putsnb(buff);
}
void PrintWDir(const WCHAR * wch) {
	putsnb("[");
	PrintWChar(wch);
	putsnb("]");
}

BOOL CreateDir(const WCHAR * dirpath) {
	WCHAR buff[LOCAL_LEN];
	wsprintfW(buff, L"%ls\\..", dirpath);
	//GetFullPathNameW(buff, LOCAL_LEN, buff, 0);
	if (GetFileAttributesW(buff) == INVALID_FILE_ATTRIBUTES) {
		if (!CreateDir(buff))
			return FALSE;
	}
	return CreateDirectoryW(dirpath, NULL);
}

void FullUpperPath(WCHAR * path) {
	WCHAR tmp[LOCAL_LEN];
	wcscpy(tmp, path);
	wcscat(tmp,L"\\..");
	GetFullPathName(tmp, LOCAL_LEN, path, nullptr);
}

BOOL LinkFileR(WCHAR * pathFrom, WCHAR * pathTo) {
	WCHAR from_up[LOCAL_LEN], to_up[LOCAL_LEN];
	
	wcscpy(from_up, pathFrom);
	wcscpy(to_up, pathTo);

	FullUpperPath(from_up);
	FullUpperPath(to_up);

	DWORD up_attr = GetFileAttributes(from_up);
	if (up_attr == INVALID_FILE_ATTRIBUTES && ! LinkFileR(from_up,to_up)) {
		//连接失败
		return FALSE;
	}



	if (GetFileAttributes(pathFrom) != INVALID_FILE_ATTRIBUTES) {
		bprintf("尝试链接[%ls]，目录已存在", pathFrom);
		return TRUE;
	}

	bprintf("即将连接目录[%ls]->[%ls]\n", pathFrom, pathTo);
	
	if (makeLink(pathFrom, pathTo)) {
		bprintf("成功\n");
		return TRUE;
	}
	bprintf("失败(0x%x)\n",GetLastError());
	return FALSE;
}

int main()
{
	boxBuffer[0] = '\0';

	char userbuff[LOCAL_LEN];
	char smpath[LOCAL_LEN];
	char smpath_up[LOCAL_LEN];
	WCHAR userbuff_w[LOCAL_LEN];
	WCHAR smpath_normw[LOCAL_LEN + sizeof(GAME_MOD_PATH)];
	WCHAR smpath_w[LOCAL_LEN + sizeof(GAME_MOD_PATH)];

	printf("关闭此窗口以随时终止程序\n");
	PutBuffer(L"以撒的结合AB+Mod中文路径BUG修复工具 v2\n");
	PutBuffer(L"制作 frto027(tieba:602706150)\n");
	PutBuffer(L"点击确定继续");
	if (ShowBox(L"版权声明", MB_OKCANCEL) == IDCANCEL)
		return 0;

	ENSURE(GetDocDir(userbuff, 2048), "can not get user dir\n",L"无法获得用户目录") {
		//smpath
		for (int i = 0; i < LOCAL_LEN; i++) {
			smpath[i] = tolower(userbuff[i]);//|(0x80 & userbuff[i]);
											 //printf("%d->%d\n", userbuff[i], smpath[i]);
			if (userbuff[i] == '\0')
				break;
		}
		/*
		//smpath_up
		sprintf_s(smpath_up, "%s\\..", smpath);
		GetFullPathNameA(smpath_up, LOCAL_LEN, smpath_up, 0);
		*/

		CharToWCharNormal(LOCAL_LEN, userbuff, userbuff_w);

		CharToWCharBuggy(LOCAL_LEN, smpath, smpath_w);
		//CharToWCharBuggy(LOCAL_LEN, smpath_up, smpath_up_w);
		CharToWCharNormal(LOCAL_LEN, smpath, smpath_normw);

		bprintf("现在将探测游戏使用的文档目录\n");
		bprintf("游戏使用的文档目录 [%ls]\n", userbuff_w);
		bprintf("正常逻辑下，游戏应使用[%ls]\n", smpath_normw);
		bprintf("但实际读取MOD会用到的是[%ls]\n",smpath_w);

		DWORD SmPathAttr = GetFileAttributesW(smpath_w);
		DWORD SmPathAttr_norm = GetFileAttributesW(smpath_normw);

		if (SmPathAttr != INVALID_FILE_ATTRIBUTES && (SmPathAttr & FILE_ATTRIBUTE_DIRECTORY)) {
			bprintf("[%ls]是一个很正常的目录，游戏会使用它,可能是有同样BUG的其它软件创建了这一目录，现在将探测目标修改为游戏的Mod目录\n",smpath_w);

			wcscat(smpath_normw, GAME_MOD_PATH);
			wcscat(smpath_w, GAME_MOD_PATH);

			bprintf("[更正]正常逻辑下，游戏应使用[%ls]\n",smpath_normw);
			bprintf("[更正]但实际读取MOD会用到的是[%ls]\n",smpath_w);

			SmPathAttr = GetFileAttributesW(smpath_w);
			SmPathAttr_norm = GetFileAttributesW(smpath_normw);
			//return 0;/* error 1 */
		}



		if (SmPathAttr == INVALID_FILE_ATTRIBUTES) {
			if (SmPathAttr_norm == INVALID_FILE_ATTRIBUTES) {
				bprintf("\n错误：正常目录[%ls]不存在，请确认游戏已经启动过一次，且游戏版本支持mod\n", smpath_normw);
				ShowBox(L"运行错误", MB_ICONERROR);
				return -1;
			}
			bprintf("即将重定向异常目录到正常目录，点击继续");
			if (ShowBox(L"继续?", MB_OKCANCEL) == IDCANCEL)
				return 0;
		}
		else if (SmPathAttr & FILE_ATTRIBUTE_DIRECTORY) {
			bprintf("[%ls]是一个目录，且直接被游戏使用，请您检查这个目录是否有效。如果已经确认效果，请删除这个目录后再次运行这个工具。如果您已经修复过，也会看到这行提示。", smpath_w);
			ShowBox(L"已有目录", MB_OK);
			return 0;
		}
		else {
			bprintf("[%ls]是一个文件，你不删掉它，游戏是没办法工作的\n请删掉它之后再运行这个程序(delete it)\n请您确认文件内容后手动删除（安全起见此工具不会删除您的文件）", smpath_w);
			ShowBox(L"游戏的mod路径被文件占用", MB_ICONERROR);
			return 0; /* error*/
		}

		/* 重定向 */
		
		if (LinkFileR(smpath_w, smpath_normw)) {
			ShowBox(L"成功", MB_OK);
		}
		else {
			ShowBox(L"失败", MB_ICONERROR);
		}



		/*
		DWORD SmPathAttr = GetFileAttributesW(smpath_w);
		if (SmPathAttr == INVALID_FILE_ATTRIBUTES) {
			PrintWDir(smpath_w);
			puts("不存在，尝试符号链接到正常目录");
			system("pause");
		}
		else if (SmPathAttr & FILE_ATTRIBUTE_DIRECTORY) {
			PrintWDir(smpath_w);
			puts("是一个很正常的目录，游戏会使用它，或者你可以删掉它再运行这个脚本，再见~");
			system("pause");
			return 0;
		}
		else {
			PrintWDir(smpath_w);
			puts("是一个文件，你不删掉它，游戏是没办法工作的\n请删掉它之后再运行这个程序(delete it)");
			system("pause");
			return 0;
		}
		putsnb("上层目录是");
		PrintWDir(smpath_up_w);
		putsnb("\n这个目录");
		DWORD UpAttribute = GetFileAttributesW(smpath_up_w);
		if (UpAttribute == INVALID_FILE_ATTRIBUTES) {
			puts("不存在，尝试创建...");

			if (CreateDir(smpath_up_w) == FALSE) {
				printf("创建目录失败(Create directory failed)，错误代码(Error code):%ld", GetLastError());
				system("pause");
				return 0;
			}
			else {
				puts("创建成功");
			}
			system("pause");
		}
		else if (UpAttribute & FILE_ATTRIBUTE_DIRECTORY) {
			puts("是一个目录，可以被使用");
		}
		puts("创建链接...\n从");
		PrintWDir(smpath_normw);
		puts("\n到");
		PrintWDir(smpath_w);
		putchar('\n');
		if (makeLink(smpath_w, smpath_normw)) {
			puts("创建成功(Success)，再会");
		}
		else {
			printf("创建失败(Link failed),错误代码(Error code):%ld", GetLastError());
		}
		system("pause");*/
	}
	return 0;
}