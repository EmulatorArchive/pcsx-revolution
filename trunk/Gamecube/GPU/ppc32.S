/*asm u16 ppc32_BGR24to16(u32 BGR)*/
        .text
        .align  4
        .globl  ppc32_BGR24to16
ppc32_BGR24to16:
	rlwinm  9, 3, 29, 27, 31
	rlwinm  7, 3, 23, 17, 21
	rlwinm  3, 3, 26, 22, 26
	or      7, 7, 9
	or      3, 7, 3
	blr
/*
} 
*/