#include "imgui/imgui.h"

class esp
{
public:
    static void DrawBox(ImVec2 boxMin, ImVec2 boxMax, float distance, bool cornerbox, bool Visable);

    static void DrawHealthbar(ImVec2 boxMin, ImVec2 boxMax, int health, bool Visable);

 

};

