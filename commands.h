#ifndef COMMANDS_H
#define COMMANDS_H

#include "bluetooth.h"

class Commands {
private:
    int A;
    int B;
    static const String commandConfig[5][8];
    String sendCommand(String pid);
    void parsePIDResponse(String response, String pid, int numBytes);
    double query(int commandIndex);

public:
    Commands();
    double getRPM();
    double getBarometricPressure();
    double getEngineLoad();
    double getTorque();
    double getBoost();
    void initializeOBD();
};

#endif // COMMANDS_H