// ArpTable.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#include <sys/stat.h>
#include <sys/types.h>
#include <iostream>
#include <ipmib.h>
#include <vector>
#include <algorithm>
#include <iphlpapi.h>
#include <iomanip>
#include <sstream>
#pragma warning(disable: 4996)

class ArpTable {
private:
        PMIB_IPNETTABLE ipNetTable = NULL;
        PMIB_IPADDRTABLE ipAddrTable = NULL;
public:
    ArpTable() {
        initIpNetTable();
        initIpAddrTable();
    }

    ~ArpTable() {
        free(ipNetTable);
        free(ipAddrTable);
    }

    std::string getAdaptarIndexiesStr() {
        std::stringstream stream;

        stream << "[ ";
        for (int i = 0; i < ipAddrTable->dwNumEntries; i++) {
            stream << ipAddrTable->table[i].dwIndex << " ";
        }
        stream << "]";
       
        return std::string(stream.str());
    }

    DWORD deleteIpNetEntry(std::string ipAddress, DWORD adapterIndex) {
        
    }

    DWORD createIpNetEntry(std::string ipAddress, DWORD adapterIndex, std::string macAddress) {
        
    }

    std::string macFromIpAddress(std::string ipAddress) {
    
    }

    std::string toString() {
        int j = 0;
        std::string result = "";

        for (int i = 0; i < ipAddrTable->dwNumEntries; i++) {
            result.append("Интерфейс: " + 
                ipAddrStringFromDword(ipAddrTable->table[i].dwAddr) + 
                " --- " + 
                ipTypeToHexString(ipAddrTable->table[i].wType) + 
                "\n");
           
            result.append(std::string("адрес в Интернете") + "\t" +
                std::string("Физический адрес") + "\t" + "Тип" + "\n");

            for ( ; ipAddrTable->table[i].dwIndex == ipNetTable->table[j].dwIndex; j++) {
                result.append(ipAddrStringFromDword(ipNetTable->table[j].dwAddr) + "\t" + "\t");
                result.append(physAddrLenFromUcharArray(ipNetTable->table[j].bPhysAddr, ipNetTable->table[j].dwPhysAddrLen));
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
        ipNetTable = (PMIB_IPNETTABLE)malloc(ipNetTableSize);
        GetIpNetTable(ipNetTable, &ipNetTableSize, true);
    }

    void initIpAddrTable() {
        unsigned long ipAddrTableSize = 0;

        GetIpAddrTable(ipAddrTable, &ipAddrTableSize, true);
        ipAddrTable = (PMIB_IPADDRTABLE)malloc(ipAddrTableSize);
        GetIpAddrTable(ipAddrTable, &ipAddrTableSize, true);
    }

    std::string ipAddrStringFromDword(DWORD ipAddress) {
        struct in_addr addr;
        addr.S_un.S_addr = (long)ipAddress;
        char* ipAddrStr = inet_ntoa(addr);
        return std::string(ipAddrStr);
    }

    std::string ipTypeToHexString(unsigned short type) {
        std::stringstream stream;
        stream << "0x" << std::hex << type;
        return std::string(stream.str());
    }

    std::string physAddrLenFromUcharArray(UCHAR* addr, int addrLen) {
        std::stringstream stream;
        
        if (addrLen == 0) {
            return std::string("00:00:00:00:00:00");
        }

        for (int i = 0; i < addrLen; i++) {
            stream << std::setw(2) << std::setfill('0') << std::hex << (int)addr[i];
            if (i != addrLen - 1) {
                stream << ":";
            }
        }
        return std::string(stream.str());
    }

    std::string stringFromType(unsigned short type) {
        switch (type)
        {
        case 1: {
            return std::string("Другой");
        }
        case 2: {
            return std::string("Недопустимый");
        }
        case 3: {
            return std::string("Динамический");
        }
        case 4: {
            return std::string("Статический");
        }
        }
    }
};
int main()
{
    setlocale(LC_ALL, "Russian");
    ArpTable arpTable = ArpTable();
    std::cout << arpTable.toString();
    std::cout << arpTable.getAdaptarIndexiesStr();
    std::cout << "Hello World!\n";
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
