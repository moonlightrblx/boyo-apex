#include "Windows.h"

namespace offsets {
    constexpr ULONG64 OFF_LOCAL_PLAYER = 0x26a37b8;
    constexpr ULONG64 OFF_STUDIOHDR = 0xfe8;
    constexpr ULONG64 OFF_NAME = 0x479;
    constexpr ULONG64 OFF_TEAM_NUMBER = 0x334;
    constexpr ULONG64 OFF_LIFE_STATE = 0x690;
    uintptr_t lastVisibleTime = 0x1a54;
    uintptr_t OFF_TIME_BASE = 0x2150;
    uintptr_t ViewAngles = 0x2594 - 0x14;
    uintptr_t cl_entitylist = 0x6191e68;
    uintptr_t m_vecAbsOrigin = 0x017c;

    uintptr_t ViewRender = 0x3c83110;

    uintptr_t ViewMatrix = 0x11a350;

    uintptr_t m_iHealth = 0x324;

    uintptr_t m_nForceBone = 0x0da0 + 0x48;
}