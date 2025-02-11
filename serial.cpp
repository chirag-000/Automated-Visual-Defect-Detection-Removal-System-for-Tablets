#include "serial.h"
#include <iostream>

SerialPort::SerialPort(const char* portName, int baudRate) {
    connected = false;

    hSerial = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            std::cerr << "ERROR: Handle was not attached. Reason: " << portName << " not available.\n";
        } else {
            std::cerr << "ERROR: Unable to open serial port " << portName << "\n";
        }
        return;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "ERROR: Failed to get current serial parameters\n";
        CloseHandle(hSerial);
        return;
    }

    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        std::cerr << "ERROR: Could not set serial port parameters\n";
        CloseHandle(hSerial);
        return;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        std::cerr << "ERROR: Could not set timeouts\n";
        CloseHandle(hSerial);
        return;
    }

    connected = true;
}

SerialPort::~SerialPort() {
    if (connected) {
        CloseHandle(hSerial);
    }
}

bool SerialPort::isConnected() {
    return connected;
}

bool SerialPort::writeSerialPort(const char* buffer, unsigned int buf_size) {
    if (!connected) {
        return false;
    }

    DWORD bytes_written;
    if (!WriteFile(hSerial, buffer, buf_size, &bytes_written, NULL)) {
        std::cerr << "ERROR: Failed to write to serial port\n";
        return false;
    }
    return true;
}

std::vector<std::string> SerialPort::enumeratePorts() {
    std::vector<std::string> ports;
    for (int i = 1; i <= 255; i++) {
        std::string portName = "COM" + std::to_string(i);
        HANDLE hSerial = CreateFileA(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hSerial != INVALID_HANDLE_VALUE) {
            ports.push_back(portName);
            CloseHandle(hSerial);
        } else {
            DWORD error = GetLastError();
            if (error == ERROR_FILE_NOT_FOUND) {
                // Port does not exist, continue
            } else {
                std::cerr << "ERROR: Failed to open " << portName << " with error " << error << "\n";
            }
        }
    }
    return ports;
}

