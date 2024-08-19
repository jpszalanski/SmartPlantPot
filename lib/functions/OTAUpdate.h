#ifndef OTAUPDATE_H
#define OTAUPDATE_H

#include <ArduinoOTA.h>

class OTAUpdate
{
public:
    OTAUpdate(const char *password = nullptr);
    void begin();
    void handle();

private:
    const char *password;
    void setupOTA();
};

#endif
