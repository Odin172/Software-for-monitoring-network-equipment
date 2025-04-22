#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <Windows.h>

class NetworkMonitor {
private:
    std::vector<std::string> devices;  // Список устройств для мониторинга
    std::string logFile = "network_log.txt";  // Файл для логов

public:
    // Добавление устройства в список мониторинга
    void addDevice(const std::string& ip) {
        devices.push_back(ip);
    }

    // Проверка доступности устройства (ping)
    bool pingDevice(const std::string& ip) {
        std::string command = "ping -n 1 " + ip + " > NUL";  
        int status = system(command.c_str());
        return (status == 0);  // 0 = успешный ping
    }

   // Запись лога в файл
    void logEvent(const std::string& event) {
        std::ofstream log(logFile, std::ios::app);  // Открываем файл в режиме дописывания
        if (log.is_open()) {
            time_t now = time(0);
            char timeStr[26];
            ctime_r(timeStr, sizeof(timeStr), &now); //ctime_r для работы в Linux
            log << "[" << timeStr << "] " << event << std::endl;
            log.close();
        }
    }

    // Главный цикл мониторинга
    void startMonitoring(int intervalSeconds = 60) {
        while (true) {
            for (const auto& device : devices) {
                bool isOnline = pingDevice(device);
                if (!isOnline) {
                    std::string alert = "Устройство " + device + " недоступно!";
                    logEvent(alert);
                }
                else {
                    logEvent("Устройство " + device + " работает нормально.");
                }
            }
            Sleep(intervalSeconds);  // Пауза между проверками
        }
    }
};

int main() {
    NetworkMonitor monitor;

    // Добавляем устройства для мониторинга
    monitor.addDevice("8.8.8.8");      // Google DNS
    monitor.addDevice("192.168.1.1");  // Локальный роутер

    // Запускаем мониторинг
    monitor.startMonitoring();

    return 0;
}
