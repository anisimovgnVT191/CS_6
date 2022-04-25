// ArpTable.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <WinSock2.h>
#include <iostream>
#include <algorithm>
#include <iphlpapi.h>
#include <iomanip>
#include <variant>
#include <sstream>

#pragma warning(disable: 4996)

class ArpTable {
private:
    PMIB_IPNETTABLE ipNetTable = nullptr;
    PMIB_IPADDRTABLE ipAddrTable = nullptr;
public:
    ArpTable() {
        initIpNetTable();
        initIpAddrTable();
    }

    ~ArpTable() {
        free(ipNetTable);
        free(ipAddrTable);
    }

    std::string adapterIndeciesStr() {
        std::stringstream stream;

        stream << "[ ";
        for (int i = 0; i < ipAddrTable->dwNumEntries; i++) {
            stream << ipAddrTable->table[i].dwIndex << " ";
        }
        stream << "]";
        return std::string(stream.str());
    }

    static DWORD deleteIpNetEntry(const std::string &ipAddressStr, DWORD adapterIndex) {
        unsigned long ipAddress = inet_addr(ipAddressStr.c_str());
        PMIB_IPNETROW ipNetRow = new MIB_IPNETROW;

        ipNetRow->dwIndex = adapterIndex;
        ipNetRow->dwAddr = ipAddress;

        auto resultCode = DeleteIpNetEntry(ipNetRow);

        delete ipNetRow;
        return resultCode;
    }

    static DWORD createIpNetEntry(const std::string& ipAddressStr, DWORD adapterIndex, const std::string& macAddress) {
        MIB_IPNETROW ipNetRow;
        char* end;
        ipNetRow.dwAddr = ipAddressToULong(ipAddressStr);
        ipNetRow.dwPhysAddrLen = 6;
        ipNetRow.bPhysAddr[0] = std::strtoul(macAddress.substr(0,2).c_str(), &end, 16);
        ipNetRow.bPhysAddr[1] = std::strtoul(macAddress.substr(3,2).c_str(), &end, 16);
        ipNetRow.bPhysAddr[2] = std::strtoul(macAddress.substr(6,2).c_str(), &end, 16);
        ipNetRow.bPhysAddr[3] = std::strtoul(macAddress.substr(9,2).c_str(), &end, 16);
        ipNetRow.bPhysAddr[4] = std::strtoul(macAddress.substr(12,2).c_str(), &end, 16);
        ipNetRow.bPhysAddr[5] = std::strtoul(macAddress.substr(15,2).c_str(), &end, 16);

        ipNetRow.dwIndex = adapterIndex;
        ipNetRow.dwType = MIB_IPNET_TYPE_STATIC;

        return CreateIpNetEntry(&ipNetRow);

    }

    static std::string macFromIpAddress(const std::string &ipAddressStr) {
        ULONG macAddrSize = 6;
        ULONG macAddr[2];
        unsigned long ipAddress = ipAddressToULong(ipAddressStr);

        auto code = SendARP(ipAddress, 0, macAddr, &macAddrSize);

        if (code == NO_ERROR) {
            std::stringstream stream;
            BYTE *bMacAddr = (BYTE *) &macAddr;
            for (int i = 0; i < macAddrSize; i++) {
                stream << std::setw(2) << std::setfill('0') << std::hex << (int) bMacAddr[i];
                if (i != macAddrSize - 1) {
                    stream << ":";
                }
            }
            return std::string(stream.str());
        }

        return "";
    }

    std::string toString() {
        int j = 0;
        std::string result;

        for (int i = 0; i < ipAddrTable->dwNumEntries; i++) {
            result.append("Интерфейс: " +
                          ipAddrStringFromDword(ipAddrTable->table[i].dwAddr) +
                          " --- " +
                          ipTypeToHexString(ipAddrTable->table[i].wType) +
                          "\n");

            result.append(std::string("адрес в Интернете") + "\t" +
                          std::string("Физический адрес") + "\t" + "Тип" + "\n");

            for (; ipAddrTable->table[i].dwIndex == ipNetTable->table[j].dwIndex; j++) {
                result.append(ipAddrStringFromDword(ipNetTable->table[j].dwAddr) + "\t" + "\t");
                result.append(
                        physAddrLenFromUcharArray(ipNetTable->table[j].bPhysAddr, ipNetTable->table[j].dwPhysAddrLen));
                result.append("\t");
                result.append(stringFromType(ipNetTable->table[j].dwType));
                result.append("\n");

            }
            result.append("\n");
        }
        return result;
    }

private:
    void initIpNetTable() {
        unsigned long ipNetTableSize = 0;

        GetIpNetTable(ipNetTable, &ipNetTableSize, true);
        ipNetTable = (PMIB_IPNETTABLE) malloc(ipNetTableSize);
        GetIpNetTable(ipNetTable, &ipNetTableSize, true);
    }

    void initIpAddrTable() {
        unsigned long ipAddrTableSize = 0;

        GetIpAddrTable(ipAddrTable, &ipAddrTableSize, true);
        ipAddrTable = (PMIB_IPADDRTABLE) malloc(ipAddrTableSize);
        GetIpAddrTable(ipAddrTable, &ipAddrTableSize, true);
    }

    static std::string ipAddrStringFromDword(DWORD ipAddress) {
        struct in_addr addr{};
        addr.S_un.S_addr = (long) ipAddress;
        char *ipAddrStr = inet_ntoa(addr);
        return std::string(ipAddrStr);
    }

    static std::string ipTypeToHexString(unsigned short type) {
        std::stringstream stream;
        stream << "0x" << std::hex << type;
        return std::string(stream.str());
    }

    static std::string physAddrLenFromUcharArray(UCHAR *addr, int addrLen) {
        std::stringstream stream;

        if (addrLen == 0) {
            return ("00:00:00:00:00:00");
        }

        for (int i = 0; i < addrLen; i++) {
            stream << std::setw(2) << std::setfill('0') << std::hex << (int) addr[i];
            if (i != addrLen - 1) {
                stream << ":";
            }
        }
        return std::string(stream.str());
    }

    static std::string stringFromType(unsigned short type) {
        switch (type) {
            case 1: {
                return ("Другой");
            }
            case 2: {
                return ("Недопустимый");
            }
            case 3: {
                return ("Динамический");
            }
            case 4: {
                return ("Статический");
            }
        }
    }

    static unsigned long ipAddressToULong(const std::string &ipAddress) {
        SOCKADDR_IN sa = {AF_INET};
        sa.sin_addr.s_addr = inet_addr(ipAddress.c_str());

        return sa.sin_addr.S_un.S_addr;
    }
};

class App {
private:
    ArpTable arpTable = ArpTable();
public:
    App() = default;

    void run() {
        int programFlowIndicator;

        while (true) {
            std::cout << "Для печати ARP введите 1" << std::endl;
            std::cout << "Для добавления записи в ARP таблицу введите 2" << std::endl;
            std::cout << "Для удаления записи из ARP таблицы введите 3" << std::endl;
            std::cout << "Для получения MAC адреса по IP адресу введите 4" << std::endl;
            std::cout << "Для выходы из программы введите любой символ" << std::endl;

            std::cin >> programFlowIndicator;

            switch (programFlowIndicator) {
                case 1: {
                    std::cout << arpTable.toString() << std::endl;
                    break;
                }
                case 2: {
                    std::string ipAddress;
                    DWORD adapterIndex;
                    std::string macAddrStr;

                    std::cout << "Введите IP адресс: ";
                    std::cin >> ipAddress;
                    std::cout << "Введите индекс интерфейса для удаления записи (" << arpTable.adapterIndeciesStr()
                              << "): ";
                    std::cin >> adapterIndex;
                    std::cout << "Введите МАС адресс: ";
                    std::cin >> macAddrStr;
                    auto resultCode = arpTable.createIpNetEntry(ipAddress, adapterIndex, macAddrStr);
                    std::cout << deleteErrorMessageFormatter(resultCode) << std::endl;

                    break;
                }
                case 3: {
                    std::string ipAddress;
                    DWORD adapterIndex;
                    std::cout << "Введите IP адресс: ";
                    std::cin >> ipAddress;
                    std::cout << "Введите индекс интерфейса для удаления записи (" << arpTable.adapterIndeciesStr()
                              << "): ";
                    std::cin >> adapterIndex;
                    int code = arpTable.deleteIpNetEntry(ipAddress, adapterIndex);
                    std::cout << deleteErrorMessageFormatter(code) << std::endl;
                    break;
                }
                case 4: {
                    std::string ipAddress;
                    std::cout << "Введите IP адресс: ";
                    std::cin >> ipAddress;
                    std::cout << arpTable.macFromIpAddress(ipAddress) << std::endl;
                    break;
                }
                default: {
                    return;
                }
            }
        };
    }

private:
    static std::string deleteErrorMessageFormatter(int errorCode) {

        switch (errorCode) {
            case ERROR_ACCESS_DENIED: {
                return "Отказано в доступе";
            }
            case ERROR_INVALID_PARAMETER: {
                return "Один из параметров указан неверно";
            }
            case ERROR_NOT_SUPPORTED: {
                return "IPv4 не настроен на данном компьютере";
            }
            case NO_ERROR: {
                return "Удаление прошло успешно";
            }
            default: {
                return "Неизвестная ошибка";
            }
        }
    }

};

int main() {
    system("chcp 65001");

    WSADATA firstsock;

    if (WSAStartup(MAKEWORD(2, 2), &firstsock) != 0) {
        printf("\nFailed to initialise winsock.");
        printf("\nError Code : %d", WSAGetLastError());
        return 1;    //Return 1 on error
    }
    auto app = App();
    app.run();

    WSACleanup();
    return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
