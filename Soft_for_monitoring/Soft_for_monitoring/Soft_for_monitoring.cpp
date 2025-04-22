#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

class ServiceMonitor {
private:
    struct Service {
        std::string host;
        int port;
        std::string name;
    };

    std::vector<Service> services;
    std::string logFile = "service_monitor.log";
    WSADATA wsaData;

public:
    ServiceMonitor() {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
            exit(1);
        }
    }

    ~ServiceMonitor() {
        WSACleanup();
    }

    void addService(const std::string& host, int port, const std::string& name) {
        services.push_back({ host, port, name });
    }

    bool checkService(const Service& service) {
        struct addrinfo hints = { 0 }, * result = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        // Получаем информацию об адресе
        if (getaddrinfo(service.host.c_str(), std::to_string(service.port).c_str(), &hints, &result) != 0) {
            return false;
        }

        SOCKET sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (sock == INVALID_SOCKET) {
            freeaddrinfo(result);
            return false;
        }

        // Установка таймаута (3 секунды)
        DWORD timeout = 3000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

        // Попытка подключения
        bool connected = (connect(sock, result->ai_addr, (int)result->ai_addrlen) != SOCKET_ERROR);

        // Очистка ресурсов
        closesocket(sock);
        freeaddrinfo(result);

        return connected;
    }

    void logEvent(const std::string& message) {
        std::ofstream log(logFile, std::ios::app);
        if (log.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            char timeStr[26];
            ctime_s(timeStr, sizeof(timeStr), &time);
            log << "[" << timeStr << "] " << message << std::endl;
            log.close();
        }
    }

    void startMonitoring(int interval = 300) {
        while (true) {
            for (const auto& service : services) {
                bool available = checkService(service);
                std::string status = available ? "доступен" : "недоступен";
                std::string message = "Сервис " + service.name + " (" +
                    service.host + ":" + std::to_string(service.port) +
                    ") " + status;
                logEvent(message);
            }
            std::this_thread::sleep_for(std::chrono::seconds(interval));
        }
    }
};

int main() {
    setlocale(LC_ALL, "");
    ServiceMonitor monitor;

    // Добавляем сервисы для мониторинга
    monitor.addService("google.com", 80, "Google HTTP");
    monitor.addService("microsoft.com", 443, "Microsoft HTTPS");
    monitor.addService("localhost", 3389, "RDP");

    std::cout << "Мониторинг сервисов запущен (логи в service_monitor.log)..." << std::endl;
    monitor.startMonitoring(300); // Проверка каждые 5 минут

    return 0;
}
