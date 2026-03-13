#ifndef IRACK_AUTH_H
#define IRACK_AUTH_H

#include <windows.h>
#include <wininet.h>
#include <iostream>
#include <string>
#include <array>
#include <cstdio>

#pragma comment(lib, "wininet.lib")

class IRACK_Auth {
public:

    static std::string getHWID() {
        std::string command = "wmic csproduct get uuid";
        std::array<char, 128> buffer;
        std::string result = "";

        FILE* pipe = _popen(command.c_str(), "r");
        if (!pipe) {
            std::cerr << "Failed to run command!" << std::endl;
            return "";
        }

        bool firstLine = true;
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            if (firstLine) {
                firstLine = false;
                continue;
            }
            result += buffer.data();
        }

        _pclose(pipe);

        size_t start = result.find_first_not_of(" \n\r\t");
        size_t end = result.find_last_not_of(" \n\r\t");

        if (start != std::string::npos && end != std::string::npos) {
            result = result.substr(start, end - start + 1);
        }
        else {
            result = "";
        }

        return result;
    }

    static std::string getURLFromPastebin() {
        HINTERNET hInternet = InternetOpen("HTTP Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (hInternet == NULL) {
            std::cerr << "IRACK Auth Is Dwon :( " << std::endl;
            return "";
        }

        const std::string pastebin_url = "https://raw.githubusercontent.com/boyo3473/sdgdgsgdsgd/refs/heads/main/README.md";

        HINTERNET hConnect = InternetOpenUrlA(hInternet, pastebin_url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hConnect == NULL) {
            std::cerr << "IRACK Auth Is Dwon :(" << std::endl;
            InternetCloseHandle(hInternet);
            return "";
        }

        char buffer[4096];
        DWORD bytesRead;
        std::string response;

        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            response.append(buffer, bytesRead);
        }

        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        return response;
    }

    static bool Login(const std::string& key, const std::string& hwid) {

        std::string url = getURLFromPastebin();
        if (url.empty()) {
            std::cerr << "Failed to retrieve the URL from Pastebin!" << std::endl;
            return false;
        }

        url += "/login/" + key + "/" + hwid;

        HINTERNET hInternet = InternetOpen("HTTP Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (hInternet == NULL) {
            std::cerr << "IRACK Auth Is Dwon" << std::endl;
            return false;
        }

        HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hConnect == NULL) {
            std::cerr << "IRACK Auth Is Dwon" << std::endl;
            InternetCloseHandle(hInternet);
            return false;
        }

        char buffer[4096];
        DWORD bytesRead;
        std::string response;

        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            response.append(buffer, bytesRead);
        }

        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        return response.find("Login successful") != std::string::npos;
    }

    static bool Login_No_Notification(const std::string& key, const std::string& hwid)
    {

        std::string url = getURLFromPastebin();
        if (url.empty()) {
            std::cerr << "Failed to retrieve the URL from Pastebin!" << std::endl;
            return false;
        }

        url += "/login_no_notification/" + key + "/" + hwid;

        HINTERNET hInternet = InternetOpen("HTTP Client", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (hInternet == NULL) {
            std::cerr << "IRACK Auth Is Dwon" << std::endl;
            return false;
        }

        HINTERNET hConnect = InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hConnect == NULL) {
            std::cerr << "IRACK Auth Is Dwon" << std::endl;
            InternetCloseHandle(hInternet);
            return false;
        }

        char buffer[4096];
        DWORD bytesRead;
        std::string response;

        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            response.append(buffer, bytesRead);
        }

        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        return response.find("Login successful") != std::string::npos;
    }
};

#endif


