#include <iostream>
#include <winsock2.h>
#include <string>
#include <fstream>
#include <windows.h>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

// ==========================================
// --- SECURE WORKER FUNCTION ---
// ==========================================
void handleClient(SOCKET clientSocket) {
    // 1. ANTI-SLOWLORIS SECURITY: 5-second timeout
    DWORD timeout = 5000;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    char buffer[10000] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0) {
        string rawData(buffer);

        // 2. HANDSHAKE SECURITY: Check for Secret Key
        string secretKey = "SATI_PROJ_2026";
        if (rawData.find(secretKey) != 0) {
            cout << "[SECURITY ALERT] Unauthorized access attempt blocked." << endl;
            Beep(200, 500); // Warning tone
            closesocket(clientSocket);
            return;
        }

        // 3. PARSING: Strip key and split filename|content
        // Format received: SATI_PROJ_2026|filename|content
        string payload = rawData.substr(secretKey.length() + 1);

        size_t pipePos = payload.find('|');
        if (pipePos != string::npos) {
            string originalName = payload.substr(0, pipePos);
            string fileContent = payload.substr(pipePos + 1);

            // 4. PATH TRAVERSAL SECURITY: Strip directory paths
            size_t lastSlash = originalName.find_last_of("/\\");
            if (lastSlash != string::npos) {
                originalName = originalName.substr(lastSlash + 1);
            }

            // 5. VERSIONING: Create unique timestamped filename
            auto now = chrono::system_clock::now();
            auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
            string uniqueName = to_string(ms) + "_" + originalName;

            cout << "[SECURE THREAD " << this_thread::get_id() << "] Saving: " << uniqueName << endl;

            // 6. STORAGE: Save to disk
            ofstream outFile(uniqueName, ios::out | ios::trunc);
            if (outFile.is_open()) {
                outFile << fileContent;
                outFile.close();
                Beep(1000, 100); // Success chirp
            }
        }
    }
    closesocket(clientSocket);
}

// ==========================================
// --- MAIN MANAGER ---
// ==========================================
int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Winsock Error!" << endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9090);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Bind Failed! Error: " << WSAGetLastError() << endl;
        return 1;
    }

    listen(serverSocket, 3);
    cout << "--- ASHUTOSH'S SECURE MULTI-THREADED CLOUD ---" << endl;
    cout << "STATUS: Live on Port 9090. Key: SATI_PROJ_2026" << endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);

        if (clientSocket != INVALID_SOCKET) {
            cout << ">> Connection accepted. Spawning secure worker..." << endl;
            thread t(handleClient, clientSocket);
            t.detach();
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}