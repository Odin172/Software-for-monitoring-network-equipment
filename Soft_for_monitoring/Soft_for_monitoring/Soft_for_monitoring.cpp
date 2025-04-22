#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <Windows.h>

// Для отправки в Telegram (нужна библиотека libcurl)
#include <curl/curl.h>

class NetworkMonitor {
private:
    std::vector<std::string> devices;  // Список устройств для мониторинга
    std::string logFile = "network_log.txt";  // Файл для логов
    std::string telegramBotToken = "7783108874:AAG3hrF0aW_NDpAflPa2AhUsiP9aBO3eS4s";  // Токен бота Telegram
    std::string telegramChatID = "1702112741";  // ID чата для уведомлений

public:
    // Добавление устройства в список мониторинга
    void addDevice(const std::string& ip) {
        devices.push_back(ip);
    }

    // Проверка доступности устройства (ping)
    bool pingDevice(const std::string& ip) {
        std::string command = "ping -c 1 " + ip + " > /dev/null 2>&1";  // Linux/Mac
        // Для Windows: ping -n 1 192.168.1.1 > NUL
        int status = system(command.c_str());
        return (status == 0);  // 0 = успешный ping
    }

    // Отправка сообщения в Telegram
    void sendTelegramAlert(const std::string& message) {
        CURL* curl = curl_easy_init();
        if (curl) {
            std::string url = "https://api.telegram.org/bot" + telegramBotToken +
                "/sendMessage?chat_id=" + telegramChatID +
                "&text=" + message;
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }
    }

    // Запись лога в файл
    void logEvent(const std::string& event) {
        std::ofstream log(logFile, std::ios::app);  // Открываем файл в режиме дописывания
        if (log.is_open()) {
            time_t now = time(0);
            log << "[" << ctime(&now) << "] " << event << std::endl;
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
                    sendTelegramAlert(alert);
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
