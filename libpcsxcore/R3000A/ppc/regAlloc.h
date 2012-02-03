#pragma once

namespace R3000A {
namespace JIT {

#define START_REG r14
#define END_REG r30
#define HW_REGS (END_REG - START_REG)

#define PSXREGS r31

enum {
	HW_READ = 1<<0,
	HW_WRITE = 1<<1,
	HW_SPECIAL = 1<<2
};

struct ppcReg {
	u32 psxreg;
	bool mapped;
	u32 usage;
	u32 pc;
#ifdef REG_PASS_0
	u32 reads;
	u32 writes;
#endif
};

class RegAlloc {
	ppcReg regs[HW_REGS];

	void FlushReg(int hwreg) {
		int reg = regs[hwreg].psxreg;
		if(reg > 0 && reg < NUM_REGISTERS) {
#ifdef DEBUG_REGS
			SysPrintf("FlushReg: \t %i \t %i \t %i\n", reg, hwreg, regs[hwreg].usage);
#endif
			// store only modified regs
			if(regs[hwreg].usage & HW_WRITE)
			{
				STW(START_REG+hwreg, OFFSET(&psxRegs, &psxRegs.GPR.r[reg].d), PSXREGS);
			}
		}
		DisposeReg(hwreg);
	}
	
	void DisposeReg(int reg) {
		memset((regs + reg), 0, sizeof(ppcReg));
	}
	
	void FlushUnused() {
		int i;
		int index = -1;
		int least = HW_REGS-1;
		for(i = HW_REGS-1; i >= 0; --i) {
			if(regs[i].usage & HW_WRITE) {
				if(regs[least].pc > regs[i].pc) {
					least = i;
				}
				continue;
			}
			if(regs[i].pc < pc) {
				index = i;
				break;
			}
		}
		if(index == -1) {
			index = least;
		}
#ifdef DEBUG_REGS
		SysPrintf("FlushUnused: \t %i \n", index);
#endif
		FlushReg(index);
	}

	/*int GetSpecial(int reg) {
		int ret = 0;
		switch(reg) {
			case PSXREGS:
				ret = PSXREGS;
				break;
// 			case REG_ZERO: 
// 				ret = GetFree();
// 				break;
			default: 
				exit(0);
		}
		return ret;
	}*/

	int Find(int reg) {
		int ret = -1;
		int i;
		for(i = HW_REGS-1; i >= 0; --i) {
			if(regs[i].mapped == true && regs[i].psxreg == reg) {
				ret = i;
				break;
			}
		}
		return ret;
	}
	
	int GetFree(int reg) {
		int ret = -1;
		int i;
		for(i = HW_REGS-1; i >= 0; --i) {
			if(regs[i].mapped == false) {
				ret = i;
				regs[i].psxreg = reg;
				regs[i].mapped = true;
				break;
			}
		}
#ifdef DEBUG_REGS
		SysPrintf("GetFree: \t\t %i \t %i\n", reg, ret);
#endif
		if(ret == -1) {
			FlushUnused();
			ret = GetFree(reg);
		}
		return ret;
	}
	
	int Allocate(int reg) {
		int ret = GetFree(reg);

		if(reg == 0) {
			LI(START_REG+ret, 0);
		}
		else if(IsConst(reg)) {
			LIW(START_REG+ret, iRegs[reg].k);
		}
		else {
			LWZ(START_REG+ret, OFFSET(&psxRegs, &psxRegs.GPR.r[reg].d), PSXREGS);
		}

#ifdef DEBUG_REGS
		SysPrintf("Allocate: \t %i \t %i\n", reg, ret);
#endif
		return ret;
	}
#ifdef DEBUG_REGS
	void Wait(u32 msec) {
		SysPrintf("Waiting for %i msec's\n", msec);
		while(msec--) {
			usleep(1000);
		}
	}
#endif
public:
	RegAlloc() {
		Reset();
	}
	
	int Put(int reg) {
		int ret = Find(reg);
		if(ret == -1) {
			ret = GetFree(reg);
		}
		if(reg != 0) {
			iRegs[reg].state = ST_UNK;
		}
#ifdef DEBUG_REGS
		SysPrintf("Put: \t\t\t %i \t %i\n", reg, ret);
#endif
		regs[ret].pc = pc;
		regs[ret].usage |= HW_WRITE;
		return START_REG+ret;
	}
	
	int Get(int reg) {
		int ret = Find(reg);
		if(ret == -1) {
			ret = Allocate(reg);
		}
#ifdef DEBUG_REGS
		SysPrintf("Get: \t\t\t %i \t %i\n", reg, ret);
#endif
		regs[ret].pc = pc;
		regs[ret].usage |= HW_READ;
		return START_REG+ret;
	}
	
	void FlushRegs() {
		int i;
		for(i = 0; i < HW_REGS; ++i) {
			if(regs[i].mapped) {
				FlushReg(i);
			}
#ifdef DEBUG_REGS
			Wait(400);
			SysPrintf("\n\n");
#endif
		}
	}

	void Dispose(int reg) {
		int hwreg = Find(reg);
		if(hwreg != -1) {
			DisposeReg(hwreg);
		}
	}

	void Reset() {
		memset(regs, 0, sizeof(regs));
#ifdef DEBUG_REGS
		int i;
		for(i = 0; i < HW_REGS; ++i) {
			if(regs[i].psxreg != 0 || regs[i].mapped) {
				printf("\n error!!! \n");
				Wait(10 * 1000);

				exit(0);
			}
		}
#endif
	}
	
	RegAlloc & operator=(const RegAlloc &copy) {
		if (this != &copy) {
			int i;
			for(i = 0; i < HW_REGS; ++i) {
				memcpy((regs + i), copy[i], sizeof(ppcReg));
			}
		}
		return *this;
	}
	
	ppcReg * operator[](const int index)
	{
		return (regs + index);
	}
};

}; // JIT
}; // R3000A