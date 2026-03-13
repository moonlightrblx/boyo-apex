#include "driver.h"
#define Assert( _exp ) ((void)0)







struct matrix3x4_t
{
    matrix3x4_t() {}
    matrix3x4_t(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23)
    {
        m_flMatVal[0][0] = m00;	m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
        m_flMatVal[1][0] = m10;	m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
        m_flMatVal[2][0] = m20;	m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
    }

    float* operator[](int i) { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
    const float* operator[](int i) const { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
    float* Base() { return &m_flMatVal[0][0]; }
    const float* Base() const { return &m_flMatVal[0][0]; }

    float m_flMatVal[3][4];
};

Vector3 GetBonePositionByHitBox(int ID, uintptr_t entity, Vector3 Position) {
    Vector3 origin = Position;


    uint64_t Model = read< uint64_t >(entity + offsets::OFF_STUDIOHDR); 

  
    uint64_t StudioHdr = read< uint64_t >(Model + 0x8);

  
    uint16_t HitboxCache = read< uint16_t >(StudioHdr + 0x34);
    uint64_t HitBoxsArray = StudioHdr + ((uint16_t)(HitboxCache & 0xFFFE) << (4 * (HitboxCache & 1)));

    uint16_t IndexCache = read < uint16_t >(HitBoxsArray + 0x4);
    int HitboxIndex = ((uint16_t)(IndexCache & 0xFFFE) << (4 * (IndexCache & 1)));

    uint16_t Bone = read < uint16_t >(HitBoxsArray + HitboxIndex + (ID * 0x20));

    if (Bone < 0 || Bone > 255)
        return Vector3();


    uint64_t BoneArray = read < uint64_t >(entity + offsets::m_nForceBone);
    uintptr_t Add = BoneArray + Bone * sizeof(matrix3x4_t);
    matrix3x4_t Matrix = read < matrix3x4_t >(Add);

    return Vector3(Matrix.m_flMatVal[0][3] + origin.x, Matrix.m_flMatVal[1][3] + origin.y, Matrix.m_flMatVal[2][3] + origin.z);
}


inline DWORD64 GetEntityById(int Ent, DWORD64 Base)
{
    DWORD64 EntityList = Base + offsets::cl_entitylist;
    DWORD64 BaseEntity = read<DWORD64>(EntityList);
    if (!BaseEntity)
        return NULL;
    return read<DWORD64>(EntityList + (Ent << 5));
}

uintptr_t GetMs()
{
    return static_cast<uintptr_t>(GetTickCount64());
}
struct VisibleTime
{
    float lastTime[100]{};
    bool lastState[100]{};
    uintptr_t lastCheck[100]{};
} VisibleStructure;


namespace SDK {



    struct Vector3 WorldToScreen(const struct Vector3 pos, struct Matrix matrix) {
        struct Vector3 out;
        float _x = matrix.matrix[0] * pos.x + matrix.matrix[1] * pos.y + matrix.matrix[2] * pos.z + matrix.matrix[3];
        float _y = matrix.matrix[4] * pos.x + matrix.matrix[5] * pos.y + matrix.matrix[6] * pos.z + matrix.matrix[7];
        out.z = matrix.matrix[12] * pos.x + matrix.matrix[13] * pos.y + matrix.matrix[14] * pos.z + matrix.matrix[15];

        _x *= 1.f / out.z;
        _y *= 1.f / out.z;


        int width = GetSystemMetrics(SM_CXSCREEN);  
        int height = GetSystemMetrics(SM_CYSCREEN); 

        out.x = width * 0.5f;
        out.y = height * 0.5f;

        out.x += 0.5f * _x * width + 0.5f;
        out.y -= 0.5f * _y * height + 0.5f;

        return out;
    }



    class Entity {
    public:
        Entity(uintptr_t base) : base(base) {}


        bool IsVisible(int index)
        {


            uintptr_t currentTime = GetMs();

            if (currentTime >= VisibleStructure.lastCheck[index] + 1)
            {
                float visTime = read<float>(base + offsets::lastVisibleTime);

                bool becameVisible = visTime > VisibleStructure.lastTime[index];
                bool resetVisibility = visTime < 0.f && VisibleStructure.lastTime[index] > 0.f;

                VisibleStructure.lastState[index] = becameVisible || resetVisibility;
                VisibleStructure.lastTime[index] = visTime;
                VisibleStructure.lastCheck[index] = currentTime;
            }

            return VisibleStructure.lastState[index];
        }

        


        int GetHealth() {
            return read<int>(base + offsets::m_iHealth);
        }

        int Team() {
            return read<int>(base + offsets::OFF_TEAM_NUMBER);
        }

        bool IsAlive() {
            return GetHealth() > 0;
        }

        std::string GetIdentifier() {
            uintptr_t namePtr = read<uintptr_t>(base + offsets::OFF_NAME);
            return read<std::string>(namePtr);
        }

        Vector3 GetPosition() {
            return read<Vector3>(base + offsets::m_vecAbsOrigin);
        }

        Vector3 GetHeadPosition() {
            Vector3 pos = GetBonePositionByHitBox(0, base, GetPosition());
           
            return pos;
        }

        Vector3 GetViewAngles() {
            return read<Vector3>(base + offsets::ViewAngles);
        }

        uintptr_t GetBase() {
            return base;
        }

    private:
        uintptr_t base;
    };

    Entity* GetEntity(int index) {
        uintptr_t ent = GetEntityById(index, virtualaddy);
        if (!ent) return nullptr;
        return new Entity(ent);
    }

    Entity* GetLocalPlayer() {
        uintptr_t local = read<uintptr_t>(virtualaddy + offsets::OFF_LOCAL_PLAYER);
        if (!local) return nullptr;
        return new Entity(local);
    }

    Matrix GetViewMatrix() {
        uint64_t viewRenderer = read<uint64_t>(virtualaddy + offsets::ViewRender);
        uint64_t viewMatrix = read<uint64_t>(viewRenderer + offsets::ViewMatrix);
        return read<Matrix>(viewMatrix);
    }



} 