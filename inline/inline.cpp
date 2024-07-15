// inline.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "inline_hook.hpp"

	void TestInlineHookCallback() {
		printf("111111111");
	}

int _tmain(int argc, _TCHAR* argv[])
{
	int nLen = 0;
#if defined(_WIN64) || defined(WIN64)
	nLen = 0xE;
#elif defined(_WIN32)
	nLen = 0x5;
#endif
	StartHOOK((BYTE *)&MessageBoxA, TestInlineHookCallback, nLen);
	MessageBoxA(NULL, "", "", MB_OK);

	getchar();
	return 0;
}

