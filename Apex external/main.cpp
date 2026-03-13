#include "Windows.h"
#include "iostream"
#include "thread"
#include "offsets.h"

#include "utils.h"
#include "imgui/imgui.h"
#include "esp.hpp"
#include "overlay.hpp"
#include <cassert>
#include "SDK.hpp"
#include "config.h"



#include <TlHelp32.h>
#include <vector>

#include <iomanip>
#include <string>

#include <chrono>
#include <urlmon.h>
#include <shlwapi.h>
#include <filesystem>
#include "Login.h"
#include <fstream>

using namespace std;


void DrawSnapLines(Vector3 ScreenRootPosition) {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    drawList->AddLine(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y), ImVec2(ScreenRootPosition.x, ScreenRootPosition.y), IM_COL32(255, 255, 255, 255));
}




void loop() {
    auto localPlayer = SDK::GetLocalPlayer();
    if (!localPlayer) return;

    auto localPos = localPlayer->GetPosition();
    auto viewMatrix = SDK::GetViewMatrix();

    float closestDist = config::aimbot_fov;
    Vector3 targetScreen = {};
    ImVec2 targetBoxMin = {}, targetBoxMax = {};
    bool foundTarget = false;

    for (int i = 1; i < 80; ++i) {
        auto entity = SDK::GetEntity(i);
   
        if (entity == localPlayer) continue;
        
        
        if (!entity || !entity->IsAlive()) continue;

        int Team = entity->Team();

        if (config::Team_Check && Team == localPlayer->Team()) continue;

        if (!strcmp(entity->GetIdentifier().c_str(), "player")) continue;

        Vector3 pos = entity->GetPosition();
        if (pos.x == 0 && pos.y == 0 && pos.z == 0) continue;

        float distance = (pos - localPos).Length() / 100.f;
        if (distance > config::MaxDistance) continue;

        Vector3 screenFeet = SDK::WorldToScreen(pos, viewMatrix);

        Vector3 screenHead = SDK::WorldToScreen(GetBonePositionByHitBox(0, entity->GetBase(), pos), viewMatrix);



        if (screenFeet.z <= 0.f || screenHead.z <= 0.f) continue;

        float boxHeight = fabs(screenFeet.y - screenHead.y);
        float boxWidth = boxHeight / 1.0f;
        ImVec2 boxMin(screenHead.x - boxWidth / 2.f, screenHead.y);
        ImVec2 boxMax(screenHead.x + boxWidth / 2.f, screenFeet.y);

        if (config::PlayerSkeleton) {
            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            float thickness = 1.4f;

            ImU32 color = entity->IsVisible(i) ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 255, 255, 255);

       
            Vector3 screenNeck = SDK::WorldToScreen(GetBonePositionByHitBox(1, entity->GetBase(), pos), viewMatrix);
            Vector3 screenChest = SDK::WorldToScreen(GetBonePositionByHitBox(2, entity->GetBase(), pos), viewMatrix);
            Vector3 screenWaist = SDK::WorldToScreen(GetBonePositionByHitBox(3, entity->GetBase(), pos), viewMatrix);
            Vector3 screenBottom = SDK::WorldToScreen(GetBonePositionByHitBox(4, entity->GetBase(), pos), viewMatrix);
            Vector3 screenLeftShoulder = SDK::WorldToScreen(GetBonePositionByHitBox(6, entity->GetBase(), pos), viewMatrix);
            Vector3 screenLeftElbow = SDK::WorldToScreen(GetBonePositionByHitBox(7, entity->GetBase(), pos), viewMatrix);
            Vector3 screenLeftHand = SDK::WorldToScreen(GetBonePositionByHitBox(8, entity->GetBase(), pos), viewMatrix);
            Vector3 screenRightShoulder = SDK::WorldToScreen(GetBonePositionByHitBox(9, entity->GetBase(), pos), viewMatrix);
            Vector3 screenRightElbow = SDK::WorldToScreen(GetBonePositionByHitBox(10, entity->GetBase(), pos), viewMatrix);
            Vector3 screenRightHand = SDK::WorldToScreen(GetBonePositionByHitBox(11, entity->GetBase(), pos), viewMatrix);
            Vector3 screenLeftThigh = SDK::WorldToScreen(GetBonePositionByHitBox(12, entity->GetBase(), pos), viewMatrix);
            Vector3 screenLeftKnee = SDK::WorldToScreen(GetBonePositionByHitBox(13, entity->GetBase(), pos), viewMatrix);
            Vector3 screenLeftLeg = SDK::WorldToScreen(GetBonePositionByHitBox(14, entity->GetBase(), pos), viewMatrix);
            Vector3 screenRightThigh = SDK::WorldToScreen(GetBonePositionByHitBox(16, entity->GetBase(), pos), viewMatrix);
            Vector3 screenRightKnee = SDK::WorldToScreen(GetBonePositionByHitBox(17, entity->GetBase(), pos), viewMatrix);
            Vector3 screenRightLeg = SDK::WorldToScreen(GetBonePositionByHitBox(18, entity->GetBase(), pos), viewMatrix);

           
            auto isBoneInBox = [&](const Vector3& bonePos) {
                return bonePos.x >= boxMin.x && bonePos.x <= boxMax.x && bonePos.y >= boxMin.y && bonePos.y <= boxMax.y;
                };

           
            if (isBoneInBox(screenHead) && isBoneInBox(screenNeck)) drawList->AddLine(ImVec2(screenHead.x, screenHead.y), ImVec2(screenNeck.x, screenNeck.y), color, thickness);
            if (isBoneInBox(screenNeck) && isBoneInBox(screenChest)) drawList->AddLine(ImVec2(screenNeck.x, screenNeck.y), ImVec2(screenChest.x, screenChest.y), color, thickness);
            if (isBoneInBox(screenChest) && isBoneInBox(screenWaist)) drawList->AddLine(ImVec2(screenChest.x, screenChest.y), ImVec2(screenWaist.x, screenWaist.y), color, thickness);
            if (isBoneInBox(screenWaist) && isBoneInBox(screenBottom)) drawList->AddLine(ImVec2(screenWaist.x, screenWaist.y), ImVec2(screenBottom.x, screenBottom.y), color, thickness);

          
            if (isBoneInBox(screenChest) && isBoneInBox(screenLeftShoulder)) drawList->AddLine(ImVec2(screenChest.x, screenChest.y), ImVec2(screenLeftShoulder.x, screenLeftShoulder.y), color, thickness);
            if (isBoneInBox(screenLeftShoulder) && isBoneInBox(screenLeftElbow)) drawList->AddLine(ImVec2(screenLeftShoulder.x, screenLeftShoulder.y), ImVec2(screenLeftElbow.x, screenLeftElbow.y), color, thickness);
            if (isBoneInBox(screenLeftElbow) && isBoneInBox(screenLeftHand)) drawList->AddLine(ImVec2(screenLeftElbow.x, screenLeftElbow.y), ImVec2(screenLeftHand.x, screenLeftHand.y), color, thickness);

            if (isBoneInBox(screenChest) && isBoneInBox(screenRightShoulder)) drawList->AddLine(ImVec2(screenChest.x, screenChest.y), ImVec2(screenRightShoulder.x, screenRightShoulder.y), color, thickness);
            if (isBoneInBox(screenRightShoulder) && isBoneInBox(screenRightElbow)) drawList->AddLine(ImVec2(screenRightShoulder.x, screenRightShoulder.y), ImVec2(screenRightElbow.x, screenRightElbow.y), color, thickness);
            if (isBoneInBox(screenRightElbow) && isBoneInBox(screenRightHand)) drawList->AddLine(ImVec2(screenRightElbow.x, screenRightElbow.y), ImVec2(screenRightHand.x, screenRightHand.y), color, thickness);

      
            if (isBoneInBox(screenBottom) && isBoneInBox(screenLeftThigh)) drawList->AddLine(ImVec2(screenBottom.x, screenBottom.y), ImVec2(screenLeftThigh.x, screenLeftThigh.y), color, thickness);
            if (isBoneInBox(screenLeftThigh) && isBoneInBox(screenLeftKnee)) drawList->AddLine(ImVec2(screenLeftThigh.x, screenLeftThigh.y), ImVec2(screenLeftKnee.x, screenLeftKnee.y), color, thickness);
            if (isBoneInBox(screenLeftKnee) && isBoneInBox(screenLeftLeg)) drawList->AddLine(ImVec2(screenLeftKnee.x, screenLeftKnee.y), ImVec2(screenLeftLeg.x, screenLeftLeg.y), color, thickness);

            if (isBoneInBox(screenBottom) && isBoneInBox(screenRightThigh)) drawList->AddLine(ImVec2(screenBottom.x, screenBottom.y), ImVec2(screenRightThigh.x, screenRightThigh.y), color, thickness);
            if (isBoneInBox(screenRightThigh) && isBoneInBox(screenRightKnee)) drawList->AddLine(ImVec2(screenRightThigh.x, screenRightThigh.y), ImVec2(screenRightKnee.x, screenRightKnee.y), color, thickness);
            if (isBoneInBox(screenRightKnee) && isBoneInBox(screenRightLeg)) drawList->AddLine(ImVec2(screenRightKnee.x, screenRightKnee.y), ImVec2(screenRightLeg.x, screenRightLeg.y), color, thickness);
        }
        if (config::PlayerBox)
            esp::DrawBox(boxMin, boxMax, distance, config::PlayerCornorBox,entity->IsVisible(i));

        if (config::playerhealthbar)
            esp::DrawHealthbar(boxMin, boxMax, entity->GetHealth(), entity->IsVisible(i));

        if (config::PlayerSnapline)
            DrawSnapLines(screenFeet);

        if (config::VisCheck && !entity->IsVisible(i)) continue;





        

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        if (config::aimbot_enabled && GetAsyncKeyState(VK_RBUTTON)) {
            float dx = screenHead.x - (screenWidth / 2.f);
            float dy = screenHead.y - (screenHeight / 2.f);
            float dist2D = sqrtf(dx * dx + dy * dy);

            if (dist2D < closestDist) {
                closestDist = dist2D;
                targetScreen = screenHead;
                targetBoxMin = boxMin;
                targetBoxMax = boxMax;
                foundTarget = true;
            }
        }
    }

    if (foundTarget && GetAsyncKeyState(VK_RBUTTON)) {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        float currentX = screenWidth / 2.f;
        float currentY = screenHeight / 2.f;

        float dx = targetScreen.x - currentX;
        float dy = targetScreen.y - currentY;

        dx /= config::aimbot_smooth;
        dy /= config::aimbot_smooth * 2;

        dx = std::clamp(dx, -50.f, 50.f);
        dy = std::clamp(dy, -50.f, 50.f);

        if (config::X_Only) {
            drv::MoveMouse(dx, dy);
        }
        else {
            drv::MoveMouse(dx, dy);
        }

    }



    if (config::aimbot_enabled) {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        ImVec2 screenCenter(screenWidth / 2, screenHeight / 2);
        drawList->AddCircle(
            screenCenter,
            config::aimbot_fov,
            IM_COL32(255, 0, 255, 255),
            64,
            1.5f
        );
    }
}



int main() {
    
    Login();

   

    if (!drv::Init()) {
      
        cout << "\n driver not initialized.\n";
    }

    drv::ProcessIdentifier = drv::FindProcessID("r5apex_dx12.exe");
    virtualaddy = drv::GetBaseAddress();

    while (virtualaddy == 0) {
        static int dots = 0;

      
        string output = "Waiting For Apex";
        for (int i = 0; i < dots; i++) {
            output += ".";
        }

        
        cout << "\r" << output << "   " << flush; 

       
        dots = (dots + 1) % 4;

        Sleep(500);
        drv::ProcessIdentifier = drv::FindProcessID("r5apex_dx12.exe");
        virtualaddy = drv::GetBaseAddress();
    }
 
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);


    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

    SetConsoleTitle("");


    system("cls");

    if (virtualaddy != 0 || virtualaddy != NULL)
    {
        cout << "Ready." << endl;
    }

            overlay.shouldRun = true;
            overlay.RenderMenu = false;

            overlay.CreateOverlay();

            overlay.CreateDevice();
            overlay.CreateImGui();

            overlay.SetForeground(GetConsoleWindow());

            while (overlay.shouldRun) {
                overlay.StartRender();

                if (overlay.RenderMenu) {

                    overlay.Render();

                }

                loop();

                overlay.EndRender();
            }

        
    

    
    overlay.CreateDevice();
    overlay.CreateImGui();

    overlay.SetForeground(GetConsoleWindow());

    while (overlay.shouldRun) {
        overlay.StartRender();

        if (overlay.RenderMenu) {

            overlay.Render();

        }

        loop();

        overlay.EndRender();
    }

  

}