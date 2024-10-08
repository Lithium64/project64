#pragma once
#include "MIPSInterfaceHandler.h"
#include "MemoryHandler.h"
#include <Project64-core\Logging.h>
#include <Project64-core\Settings\DebugSettings.h>
#include <Project64-core\Settings\GameSettings.h>
#include <Project64-rsp-core/cpu/RSPRegisterHandler.h>
#include <stdint.h>

class SPRegistersReg
{
protected:
    SPRegistersReg(uint32_t * SignalProcessorInterface);

public:
    uint32_t & SP_MEM_ADDR_REG;
    uint32_t & SP_DRAM_ADDR_REG;
    uint32_t & SP_RD_LEN_REG;
    uint32_t & SP_WR_LEN_REG;
    uint32_t & SP_STATUS_REG;
    uint32_t & SP_DMA_FULL_REG;
    uint32_t & SP_DMA_BUSY_REG;
    uint32_t & SP_SEMAPHORE_REG;
    uint32_t & SP_PC_REG;
    uint32_t & SP_IBIST_REG;

private:
    SPRegistersReg();
    SPRegistersReg(const SPRegistersReg &);
    SPRegistersReg & operator=(const SPRegistersReg &);
};

class CRegisters;
class CMipsMemoryVM;
class CN64System;

class SPRegistersHandler :
    public MemoryHandler,
    private RSPRegisterHandler,
    private CGameSettings,
    private MIPSInterfaceReg,
    private CDebugSettings,
    private CLogging
{
public:
    SPRegistersHandler(CN64System & N64System, CMipsMemoryVM & MMU, CRegisters & Reg);
    bool Read32(uint32_t Address, uint32_t & Value);
    bool Write32(uint32_t Address, uint32_t Value, uint32_t Mask);

    uint8_t * Imem()
    {
        return &m_IMEM[0];
    }

    uint8_t * Dmem()
    {
        return &m_DMEM[0];
    }

private:
    SPRegistersHandler();
    SPRegistersHandler(const SPRegistersHandler &);
    SPRegistersHandler & operator=(const SPRegistersHandler &);

    static void stSystemReset(SPRegistersHandler * _this)
    {
        _this->SystemReset();
    }

    void ClearSPInterrupt(void);
    void SetSPInterrupt(void);
    void SetHalt(void);
    void DmaReadDone(uint32_t End);
    void SystemReset(void);

    uint8_t m_IMEM[0x1000];
    uint8_t m_DMEM[0x1000];
    CN64System & m_System;
    CMipsMemoryVM & m_MMU;
    CRegisters & m_Reg;
    uint32_t & m_RspIntrReg;
    uint64_t & m_PC;
};
