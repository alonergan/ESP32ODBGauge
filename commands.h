#ifndef COMMANDS_H
#define COMMANDS_H

#include "bluetooth.h"

class Commands {
private:
    int A;
    int B;
    static const String commandConfig[6][8];
    void parsePIDResponse(String response, String pid, int numBytes);
    double query(int commandIndex);

public:
    Commands();
    double getReading(int selectedGauge);
    void initializeOBD();
    String sendCommand(String pid);
};

#endif // COMMANDS_H