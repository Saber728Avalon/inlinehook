#pragma once
/*
* 思路：先利用debug版本，从内存抓取生成Transfer的代码。每次inline 都用其作为模板，生成HOOK 后的执行代码。
*/

#include <Windows.h>
#include <stdint.h>

//为了方便生成指令。这里是调试函数。一旦转成relase版本时候，就直接采用数据了。没必须每次都编译代码。
#define TEST_CREATE_HOOK_TEMPLATE
#if defined(TEST_CREATE_HOOK_TEMPLATE)
	#if defined(_WIN64) || defined(WIN64)
		extern "C" void Transfer_x64_template();
	#elif defined(_WIN32)
		__declspec(naked) void Transfer(){
			__asm{
				PUSHA;
				MOV EAX, 0x12345678;//Replace Fun Addr
				CALL EAX;
				POPA;
				NOP; //backup Origin Instruct
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				NOP;
				JMP Transfer;// 0x87654321; //HOOK Over JMP Origin Address
			}
		}
	#endif
#endif //end TEST_CREATE_HOOK_TEMPLATE

//替换的实现函数指针。
typedef  void (*InlineHookCallback)();

//HOOK 模板代码
#if defined(_WIN64) || defined(WIN64)
	static unsigned char Transfer_x64[] = {   0x50, 0x53, 0x51, 0x52, 0x41, 0x50, 0x41, 0x51, 0x48, 0xb8, 0x21, 0x43, 0x65, 0x87, 0x78, 0x56
											, 0x34, 0x12, 0xff, 0xd0, 0x41, 0x59, 0x41, 0x58, 0x5a, 0x59, 0x5b, 0x58, 0x90, 0x90, 0x90, 0x90
											, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90
											, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x68, 0x78, 0x56, 0x34, 0x12, 0xc7
											, 0x44, 0x24, 0x04, 0x21, 0x43, 0x65, 0x87, 0xc3};

	bool StartHOOK_X64(BYTE *pOrignAddress, InlineHookCallback pfuncNewAddress/**/, int nLen/*replace instruct lengh*/) {
		//分配Transfer内存
		BYTE *pReplaceFunc = (BYTE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);

		//因为x64intel禁止了远程跳转。因此只能采用其他方式来跳转
		unsigned char chJmpTransfer[] = {0x48, 0xb8, 0x78, 0x56, 0x34, 0x12, 0x21, 0x43, 0x65, 0x87, 0xff, 0xe0};//MOV RAX, ADDR; JMP RAX;
		uint64_t un64JmpAddress = (uint64_t)pReplaceFunc;
		memcpy(chJmpTransfer + 2, &un64JmpAddress, 8);	

		//copy Transfer
		int nJmpOldLow = 0x3B;//回到原始函数执行,低八位。之所有这里要拆分成两个指令。是因为x64，不支持8字节的立即数
		int nJmpOldHight = 0x43;//回到原始函数执行，高八位。
		int nInstructReplaceFuncAddr = 0x0A;//替换到我实现的函数
		int nNopAddress = 0x1C;//nop位置备份，被替换的代码
		BYTE *pTransfer = (BYTE*)&Transfer_x64;

		memcpy(pReplaceFunc, &Transfer_x64, sizeof(Transfer_x64));
		//replace new address
		uint64_t un64NewAddress = (uint64_t)pfuncNewAddress;// call指令，不需要计算偏移
		memcpy(pReplaceFunc + nInstructReplaceFuncAddr, &un64NewAddress, sizeof(char *));
		//backup origin instruct
		memcpy(pReplaceFunc + nNopAddress, pOrignAddress, nLen);
		//replace Hook Next Address
		uint64_t un64NextAddress = (uint64_t)(pOrignAddress + nLen);// ret指令，不需要计算偏移
		uint32_t un32NextAddressLow = un64NextAddress & 0xFFFFFFFF;// 
		uint32_t un32NextAddressHieght = (un64NextAddress >> 32) & 0xFFFFFFFF;//
		memcpy(pReplaceFunc + nJmpOldLow , &un32NextAddressLow, 4);
		memcpy(pReplaceFunc + nJmpOldHight , &un32NextAddressHieght, 4);
		//赋予内存执行权限
		DWORD dwOld;
		VirtualProtect(pReplaceFunc, 16, PAGE_EXECUTE_READWRITE, &dwOld);
		//修改HOOK点内存权限
		VirtualProtect(pOrignAddress, 16, PAGE_EXECUTE_READWRITE, &dwOld);
		//替换HOOK点指令
		memcpy(pOrignAddress, chJmpTransfer, sizeof(chJmpTransfer));
		//恢复HOOK点内存权限
		VirtualProtect(pOrignAddress, 16, dwOld, &dwOld);

		return true;
	}
#elif defined(_WIN32)
	static unsigned char Transfer_x86[] = {   0x66, 0x60, 0xb8, 0x78, 0x56, 0x34, 0x12, 0xff, 0xd0, 0x66, 0x61, 0x90, 0x90, 0x90, 0x90, 0x90
											, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0xe9, 0x87, 0x65, 0x43, 0x21};
	//x86实现的inline hook
	bool StartHOOK_X86(BYTE *pOrignAddress, InlineHookCallback pfuncNewAddress/**/, int nLen/*replace instruct lengh*/) {
	
		//分配Transfer内存
		BYTE *pReplaceFunc = (BYTE *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);

		//产生HOOK位置的跳转指令
		unsigned char chJmpTransfer[] = {0xE9, 0x00, 0x00, 0x00, 0x00};//JMP Addr;
		uint64_t un64JmpAddress = (uint64_t)pReplaceFunc - (uint64_t)pOrignAddress - sizeof(chJmpTransfer);//EIP总是指向下一条将要执行的指令。因此必须减去jmp自身长度。
		memcpy(chJmpTransfer + 1, &un64JmpAddress, 4);	

		//copy Transfer
		int nJmpOld = 0x1A;//回到原始函数执行
		int nInstructReplaceFuncAddr = 0x03;//替换到我实现的函数
		int nNopAddress = 0x0b;//nop位置备份，被替换的代码
		BYTE *pTransfer = (BYTE*)&Transfer_x86;

		memcpy(pReplaceFunc, &Transfer_x86, sizeof(Transfer_x86));
		//replace new address
		uint64_t un64NewAddress = (uint64_t)pfuncNewAddress;// call指令，不需要计算偏移
		memcpy(pReplaceFunc + nInstructReplaceFuncAddr, &un64NewAddress, sizeof(char *));
		//backup origin instruct
		memcpy(pReplaceFunc + nNopAddress, pOrignAddress, nLen);
		//replace Hook Next Address
		uint64_t un64NextAddress = (uint64_t)(pOrignAddress + nLen) - (uint64_t)(pReplaceFunc + sizeof(Transfer_x86));
		memcpy(pReplaceFunc + nJmpOld, &un64NextAddress, sizeof(char *));
		//赋予内存执行权限
		DWORD dwOld;
		VirtualProtect(pReplaceFunc, 8, PAGE_EXECUTE_READWRITE, &dwOld);
		//修改HOOK点内存权限
		VirtualProtect(pOrignAddress, 8, PAGE_EXECUTE_READWRITE, &dwOld);
		//替换HOOK点指令
		memcpy(pOrignAddress, chJmpTransfer, sizeof(chJmpTransfer));
		//恢复HOOK点内存权限
		VirtualProtect(pOrignAddress, 8, dwOld, &dwOld);
		return true;
	}

#endif






//start inline hook 
bool StartHOOK(BYTE *pOrignAddress, InlineHookCallback pfuncNewAddress/**/, int nLen/*replace instruct lengh*/) {	
	//存在跳转指令，就直接禁止HOOK
	if (0xE9 == pOrignAddress[0]/*长跳转*/ || 0xEB == pOrignAddress[0]/*跳转*/) {
		return false;
	}
#if defined(_WIN64) || defined(WIN64)
	//Transfer_x64_template();
	StartHOOK_X64(pOrignAddress, pfuncNewAddress, nLen);
#elif defined(_WIN32)
	StartHOOK_X86(pOrignAddress, pfuncNewAddress, nLen);
#endif
	
	return true;
}




