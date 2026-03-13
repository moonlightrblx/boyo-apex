#include "esp.hpp"
#include <corecrt_math.h>

template <typename T>
constexpr const T& clamp(const T& value, const T& min, const T& max) {
    return (value < min) ? min : (value > max ? max : value);
}

void esp::DrawBox(ImVec2 boxMin, ImVec2 boxMax, float distance, bool cornerbox, bool Visable) {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    float originalWidth = boxMax.x - boxMin.x;
    float originalHeight = boxMax.y - boxMin.y;

    float widthReduction = originalWidth * 0.2f;
    boxMin.x += widthReduction / 2;
    boxMax.x -= widthReduction / 2;

    float heightIncrease = originalHeight * 0.1f;
    boxMin.y -= heightIncrease;
    boxMax.y += heightIncrease;

    float headSpace = originalHeight * 0.17f;
    boxMin.y -= headSpace;

    ImVec4 visibleColor = Visable ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.9f);
    ImVec4 outlineColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    float outlineThickness = 2.0f;
    float glowIntensity = clamp(2.0f - (distance / 1000.f), 1.0f, 3.0f);
    ImU32 colorPrimary = ImGui::ColorConvertFloat4ToU32(visibleColor);
    ImU32 colorOutline = ImGui::ColorConvertFloat4ToU32(outlineColor);

    if (cornerbox) {
        float boxWidth = boxMax.x - boxMin.x;
        float boxHeight = boxMax.y - boxMin.y;
        float cornerLengthW = boxWidth * 0.25f;
        float cornerLengthH = boxHeight * 0.25f;

        for (int i = 1; i <= (int)glowIntensity; ++i) {
            float alpha = 0.25f - (i * 0.05f);
            ImU32 glowColor = IM_COL32(180, 180, 180, static_cast<int>(alpha * 255));

            drawList->AddLine(ImVec2(boxMin.x - i, boxMin.y - i), ImVec2(boxMin.x + cornerLengthW, boxMin.y - i), glowColor, 1.0f);
            drawList->AddLine(ImVec2(boxMin.x - i, boxMin.y - i), ImVec2(boxMin.x - i, boxMin.y + cornerLengthH), glowColor, 1.0f);

            drawList->AddLine(ImVec2(boxMax.x + i, boxMin.y - i), ImVec2(boxMax.x - cornerLengthW, boxMin.y - i), glowColor, 1.0f);
            drawList->AddLine(ImVec2(boxMax.x + i, boxMin.y - i), ImVec2(boxMax.x + i, boxMin.y + cornerLengthH), glowColor, 1.0f);

            drawList->AddLine(ImVec2(boxMin.x - i, boxMax.y + i), ImVec2(boxMin.x + cornerLengthW, boxMax.y + i), glowColor, 1.0f);
            drawList->AddLine(ImVec2(boxMin.x - i, boxMax.y + i), ImVec2(boxMin.x - i, boxMax.y - cornerLengthH), glowColor, 1.0f);

            drawList->AddLine(ImVec2(boxMax.x + i, boxMax.y + i), ImVec2(boxMax.x - cornerLengthW, boxMax.y + i), glowColor, 1.0f);
            drawList->AddLine(ImVec2(boxMax.x + i, boxMax.y + i), ImVec2(boxMax.x + i, boxMax.y - cornerLengthH), glowColor, 1.0f);
        }

        drawList->AddLine(boxMin, ImVec2(boxMin.x + cornerLengthW, boxMin.y), colorPrimary, outlineThickness);
        drawList->AddLine(boxMin, ImVec2(boxMin.x, boxMin.y + cornerLengthH), colorPrimary, outlineThickness);

        drawList->AddLine(ImVec2(boxMax.x, boxMin.y), ImVec2(boxMax.x - cornerLengthW, boxMin.y), colorPrimary, outlineThickness);
        drawList->AddLine(ImVec2(boxMax.x, boxMin.y), ImVec2(boxMax.x, boxMin.y + cornerLengthH), colorPrimary, outlineThickness);

        drawList->AddLine(ImVec2(boxMin.x, boxMax.y), ImVec2(boxMin.x + cornerLengthW, boxMax.y), colorPrimary, outlineThickness);
        drawList->AddLine(ImVec2(boxMin.x, boxMax.y), ImVec2(boxMin.x, boxMax.y - cornerLengthH), colorPrimary, outlineThickness);

        drawList->AddLine(ImVec2(boxMax.x, boxMax.y), ImVec2(boxMax.x - cornerLengthW, boxMax.y), colorPrimary, outlineThickness);
        drawList->AddLine(ImVec2(boxMax.x, boxMax.y), ImVec2(boxMax.x, boxMax.y - cornerLengthH), colorPrimary, outlineThickness);
    }
    else {

        for (int i = 1; i <= (int)glowIntensity; ++i) {
            float alpha = 0.3f - (i * 0.06f);
            ImU32 glowColor = IM_COL32(180, 180, 180, static_cast<int>(alpha * 255));
            drawList->AddRect(ImVec2(boxMin.x - i, boxMin.y - i), ImVec2(boxMax.x + i, boxMax.y + i), glowColor, 0, 0, 1.0f);
        }

        drawList->AddRect(boxMin, boxMax, colorOutline, 0, 0, outlineThickness + 0.5f);
        drawList->AddRect(boxMin, boxMax, colorPrimary, 0, 0, outlineThickness);
    }
}
void esp::DrawHealthbar(ImVec2 boxMin, ImVec2 boxMax, int health, bool Visable) {
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    float originalWidth = boxMax.x - boxMin.x;
    float originalHeight = boxMax.y - boxMin.y;

    float widthReduction = originalWidth * 0.2f;
    boxMin.x += widthReduction / 2;
    boxMax.x -= widthReduction / 2;

    float heightIncrease = originalHeight * 0.1f;
    boxMin.y -= heightIncrease;
    boxMax.y += heightIncrease;

    float headSpace = originalHeight * 0.17f;
    boxMin.y -= headSpace;

    float healthPercentage = static_cast<float>(health) / 100.0f;
    float barThickness = 3.0f;
    float offsetX = 4.0f;

    ImVec2 barBackgroundMin = ImVec2(boxMin.x - barThickness - offsetX, boxMin.y);
    ImVec2 barBackgroundMax = ImVec2(boxMin.x - offsetX, boxMax.y);
    drawList->AddRectFilled(barBackgroundMin, barBackgroundMax, IM_COL32(80, 0, 0, 255));

    float barHeight = (boxMax.y - boxMin.y) * healthPercentage;
    ImVec2 barMin = ImVec2(boxMin.x - barThickness - offsetX, boxMax.y - barHeight);
    ImVec2 barMax = ImVec2(boxMin.x - offsetX, boxMax.y);

    ImU32 healthColor = Visable
        ? IM_COL32(0, 255, 0, 255)
        : IM_COL32(255 * (1.0f - healthPercentage), 255 * healthPercentage, 0, 255);

    drawList->AddRectFilled(barMin, barMax, healthColor);
    drawList->AddRect(barBackgroundMin, barBackgroundMax, IM_COL32(255, 255, 255, 255));
}