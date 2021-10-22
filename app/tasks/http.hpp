#pragma once

#include <modm/board.hpp>
#include <modm/processing/rtos.hpp>

#include <FreeRTOS_IP.h>
#include <FreeRTOS_Sockets.h>

class HttpConnection {
    static constexpr TickType_t shutdownTimeout { pdMS_TO_TICKS(5000) };
    static constexpr TickType_t receiveTimeout { pdMS_TO_TICKS(5000) };
    static constexpr TickType_t sendTimeout { pdMS_TO_TICKS(5000) };

public:
    static constexpr char name[] { "HTTPConnection" };
    static constexpr uint8_t httpText[] = { "HTTP/1.1 200 OK \r\n"
                                            "Content-Type: text/html\r\n"
                                            "Connection: keep-alive\r\n"
                                            "\r\n"
                                            "<html><body><h1>Hello from your STM32! :)</h1></body></html>" };

    enum class ResponseStatus : uint16_t {
        Ok = 200,
        BadRequest = 400,
        NotFound = 404,
    };

    static void run(void* parameter) {
        Socket_t connectedSocket = reinterpret_cast<Socket_t>(parameter);
        uint8_t* buffer = reinterpret_cast<uint8_t*>(pvPortMalloc(ipconfigTCP_MSS));

        if (buffer) {
            FreeRTOS_setsockopt(connectedSocket, 0, FREERTOS_SO_RCVTIMEO, &receiveTimeout, sizeof(receiveTimeout));
            FreeRTOS_setsockopt(connectedSocket, 0, FREERTOS_SO_SNDTIMEO, &sendTimeout, sizeof(sendTimeout));

            while (true) {
                std::memset(buffer, 0, ipconfigTCP_MSS);
                int32_t bytes = FreeRTOS_recv(connectedSocket, buffer, ipconfigTCP_MSS, 0);
                if (bytes <= 0)
                    break;
                if (FreeRTOS_send(connectedSocket, httpText, sizeof(httpText) - 1, 0) < 0)
                    break;
            }
        }

        FreeRTOS_shutdown(connectedSocket, FREERTOS_SHUT_RDWR);
        TickType_t shutdownTime { xTaskGetTickCount() };
        do {
            if (FreeRTOS_recv(connectedSocket, buffer, ipconfigTCP_MSS, 0) < 0)
                break;
        } while ((xTaskGetTickCount() - shutdownTime) < shutdownTimeout);

        vPortFree(buffer);
        FreeRTOS_closesocket(connectedSocket);
        vTaskDelete(0);
    }
};

class HttpServerListener {
    static constexpr TickType_t receiveTimeout { portMAX_DELAY };
    static constexpr BaseType_t backlog { 20 };

public:
    static constexpr char name[] { "HTTPListener" };

    static void run(void*) {
        MODM_LOG_DEBUG << "  -- Http server task run()" << modm::endl;

        Socket_t listeningSocket;
        Socket_t connectedSocket;

        listeningSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
        FreeRTOS_setsockopt(listeningSocket, 0, FREERTOS_SO_RCVTIMEO, &receiveTimeout, sizeof(receiveTimeout));

#if ipconfigUSE_TCP_WIN == 1
        WinProperties_t winProps {
            .lTxBufSize = ipconfigTCP_TX_BUFFER_LENGTH,
            .lTxWinSize = 2,
            .lRxBufSize = ipconfigTCP_RX_BUFFER_LENGTH,
            .lRxWinSize = 2,
        };
        FreeRTOS_setsockopt(
            listeningSocket, 0, FREERTOS_SO_WIN_PROPERTIES, reinterpret_cast<void*>(&winProps), sizeof(winProps));
#endif

        struct freertos_sockaddr bindAddress {
            .sin_len = 0, .sin_family = 0, .sin_port = FreeRTOS_htons(80), .sin_addr = 0
        };
        FreeRTOS_bind(listeningSocket, &bindAddress, sizeof(bindAddress));
        FreeRTOS_listen(listeningSocket, backlog);

        struct freertos_sockaddr clientAddress;

        while (true) {
            connectedSocket = FreeRTOS_accept(listeningSocket, &clientAddress, 0);
            char buffer[16];
            FreeRTOS_inet_ntoa(clientAddress.sin_addr, buffer);
            xTaskCreate(
                HttpConnection::run,
                HttpConnection::name,
                configMINIMAL_STACK_SIZE * 5,
                reinterpret_cast<void*>(connectedSocket),
                configMAX_PRIORITIES -2,
                0);
        }
    }
};
