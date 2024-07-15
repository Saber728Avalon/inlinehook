.CODE

Transfer_x64_template PROC
		PUSH RAX;
		PUSH RBX;
		PUSH RCX;
		PUSH RDX;
		PUSH R8;
		PUSH R9
		MOV RAX, 1234567887654321h;//Replace Fun Addr
		CALL RAX;
		POP R9;
		POP R8;
		POP RDX;
		POP RCX;
		POP RBX;
		POP RAX;
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
		NOP;
		PUSH 12345678h;
		MOV  dword ptr [rsp + 4], 87654321h;
		RET;
Transfer_x64_template ENDP

END