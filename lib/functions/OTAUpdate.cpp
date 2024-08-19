#include "OTAUpdate.h"

OTAUpdate::OTAUpdate(const char *password)
{
    this->password = password;
}

void OTAUpdate::begin()
{
    setupOTA();
}

void OTAUpdate::setupOTA()
{
    if (password != nullptr)
    {
        ArduinoOTA.setPassword(password);
    }

    ArduinoOTA.onStart([]()
                       {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        Serial.println("Iniciando atualização " + type); });

    ArduinoOTA.onEnd([]()
                     { Serial.println("\nFinalizado"); });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progresso: %u%%\r", (progress / (total / 100))); });

    ArduinoOTA.onError([](ota_error_t error)
                       {
        Serial.printf("Erro [%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Erro de Autenticação");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Erro no início");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Erro de conexão");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Erro ao receber");
        } else if (error == OTA_END_ERROR) {
            Serial.println("Erro no final");
        } });

    ArduinoOTA.begin();
    Serial.println("Pronto para OTA");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
}

void OTAUpdate::handle()
{
    ArduinoOTA.handle();
}
