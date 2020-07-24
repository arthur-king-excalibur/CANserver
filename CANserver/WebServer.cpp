#include "WebServer.h"

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#include <SPIFFS.h>

//The SPIFFS editor taks up a lot of flash space.  For development purposes it is nice to have, but for production it should prob be comented out
#define INCLUDE_SPIFFS_EDITOR
#ifdef INCLUDE_SPIFFS_EDITOR
#include <SPIFFSEditor.h>
#endif

#include <AsyncJson.h>
#include <SD.h>


#include "DisplayState.h"
#include "VehicleState.h"
#include "SDCard.h"
#include "CanBus.h"

#define ASSIGN_HELPER(keyname) if (keyParam->value() == #keyname)\
                    {\
                        vehicleStateInstance->keyname = atoi(valParam->value().c_str());\
                    }
#define FETCH_HELPER(keyname) vehiclestatus[#keyname] = vehicleStateInstance->keyname;
AsyncWebServer server(80);

namespace CANServer
{
    namespace WebServer
    {
        String _displayTemplateProcessor(const String& var);

        void setup()
        {
            Serial.println("Setting up Web Server...");

        #ifdef INCLUDE_SPIFFS_EDITOR
            //Attach the SPIFFS editor helper so we can edit files on the fly
            server.addHandler(new SPIFFSEditor(SPIFFS, "admin","password"));
        #endif

            server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(SPIFFS, "/html/index.html");
            });


            //Configuration related url handling
            server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(SPIFFS, "/html/config.html");
            });

            server.on("/config_save", HTTP_POST, [](AsyncWebServerRequest * request) {
                
                if(request->hasParam("disp0", true))
                {
                    AsyncWebParameter* newValue = request->getParam("disp0", true);
                    CANServer::DisplayState::display0->updateDisplayString(newValue->value().c_str());
                    CANServer::DisplayState::display0->save();
                }

                if(request->hasParam("disp1", true))
                {
                    AsyncWebParameter* newValue = request->getParam("disp1", true);
                    CANServer::DisplayState::display1->updateDisplayString(newValue->value().c_str());
                    CANServer::DisplayState::display1->save();
                }

                if(request->hasParam("disp2", true))
                {
                    AsyncWebParameter* newValue = request->getParam("disp2", true);
                    CANServer::DisplayState::display2->updateDisplayString(newValue->value().c_str());
                    CANServer::DisplayState::display2->save();
                }

                request->redirect("/config");
            });

            server.on("/config_update", HTTP_GET, [](AsyncWebServerRequest *request) {
                AsyncJsonResponse * response = new AsyncJsonResponse();
                JsonVariant& doc = response->getRoot();

                JsonObject displaysettings = doc.createNestedObject("displaysettings");
                
                displaysettings["disp0"] = CANServer::DisplayState::display0->displayString();
                displaysettings["disp1"] = CANServer::DisplayState::display1->displayString();
                displaysettings["disp2"] = CANServer::DisplayState::display2->displayString();
                displaysettings["dispOff"] = CANServer::DisplayState::offDisplayString();
                
                response->setLength();
                request->send(response);
            }); 

           


            //Debug related url handling
            server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *request) {
                request->send(SPIFFS, "/html/debug.html");
            });

            server.on("/debug_save", HTTP_POST, [](AsyncWebServerRequest * request) {
                //Handle any incomming post vars and update the vehicle state as required

                if(request->hasParam("key", true) && request->hasParam("value", true))
                {
                    AsyncWebParameter* keyParam = request->getParam("key", true);
                    AsyncWebParameter* valParam = request->getParam("value", true);

                    CANServer::VehicleState *vehicleStateInstance = CANServer::VehicleState::instance();
                    ASSIGN_HELPER(BattVolts)
                    ASSIGN_HELPER(BattAmps)

                    if (keyParam->value() == "BattVolts")
                    {
                        vehicleStateInstance->BattVolts = atoi(valParam->value().c_str());

                        vehicleStateInstance->BattPower = vehicleStateInstance->BattVolts * vehicleStateInstance->BattAmps / 100;
                    }

                    if (keyParam->value() == "BattAmps")
                    {
                        vehicleStateInstance->BattAmps = atoi(valParam->value().c_str());

                        vehicleStateInstance->BattPower = vehicleStateInstance->BattVolts * vehicleStateInstance->BattAmps / 100;
                    }
                    
                    if (keyParam->value() == "RearTorque")
                    {
                        vehicleStateInstance->RearTorque = atoi(valParam->value().c_str());
                    }

                    ASSIGN_HELPER(FrontTorque)
                    ASSIGN_HELPER(MinBattTemp)
                    ASSIGN_HELPER(BattCoolantRate)
                    ASSIGN_HELPER(PTCoolantRate)
                    ASSIGN_HELPER(MaxRegen)
                    ASSIGN_HELPER(MaxDisChg)
                    ASSIGN_HELPER(VehSpeed)
                    ASSIGN_HELPER(SpeedUnit)
                    ASSIGN_HELPER(v12v261)
                    ASSIGN_HELPER(BattCoolantTemp)
                    ASSIGN_HELPER(PTCoolantTemp)
                    ASSIGN_HELPER(BattRemainKWh)
                    ASSIGN_HELPER(BattFullKWh)
                    ASSIGN_HELPER(InvHStemp376)
                    ASSIGN_HELPER(BSR)
                    ASSIGN_HELPER(BSL)
                    ASSIGN_HELPER(DisplayOn)
                    
                    request->send(200);
                }
                else
                {
                    request->send(404);
                }
                
                    //AsyncWebParameter* p = request->getParam("download");
            });

            server.on("/debug_update", HTTP_GET, [](AsyncWebServerRequest *request) {
                AsyncJsonResponse * response = new AsyncJsonResponse();
                JsonVariant& doc = response->getRoot();

                JsonObject vehiclestatus = doc.createNestedObject("vehiclestatus");

                CANServer::VehicleState *vehicleStateInstance = CANServer::VehicleState::instance();
                FETCH_HELPER(BattVolts)
                FETCH_HELPER(BattAmps)
                FETCH_HELPER(RearTorque)
                FETCH_HELPER(FrontTorque)
                FETCH_HELPER(MinBattTemp)
                FETCH_HELPER(BattCoolantRate)
                FETCH_HELPER(PTCoolantRate)
                FETCH_HELPER(MaxRegen)
                FETCH_HELPER(MaxDisChg)
                FETCH_HELPER(VehSpeed)
                FETCH_HELPER(SpeedUnit)
                FETCH_HELPER(v12v261)
                FETCH_HELPER(BattCoolantTemp)
                FETCH_HELPER(PTCoolantTemp)
                FETCH_HELPER(BattRemainKWh)
                FETCH_HELPER(BattFullKWh)
                FETCH_HELPER(InvHStemp376)
                FETCH_HELPER(BSR)
                FETCH_HELPER(BSL)
                FETCH_HELPER(DisplayOn)
                
                response->setLength();
                request->send(response);

            });    



            //Accessability to log files
            server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
                request->send(SPIFFS, "/html/logs.html");
            }); 

            server.on("/logs_update", HTTP_GET, [](AsyncWebServerRequest *request) {
                AsyncJsonResponse * response = new AsyncJsonResponse();
                JsonVariant& doc = response->getRoot();

                {
                    //raw log details
                    JsonObject rawlogdetails = doc.createNestedObject("rawlog");                    
                    size_t fileSize = 0;
                    
                    SDFile rawlog = SD.open("/" RAWCANLOGNAME, FILE_READ);
                    if (rawlog)
                    {
                        fileSize = rawlog.size();
                        rawlog.close();
                    }
                    rawlogdetails["filesize"] = fileSize;
                    rawlogdetails["enabled"] = CANServer::CanBus::logRawCan;
                }
                
                response->setLength();
                request->send(response);

            });     

            server.on("/log_download", HTTP_GET, [](AsyncWebServerRequest *request) {

                if(request->hasParam("id", false))
                {
                    AsyncWebParameter* logid = request->getParam("id", false);

                    if (logid->value() == "rawlog")
                    {
                        request->send(SD, "/" RAWCANLOGNAME, "application/octet-stream", true);
                        return;
                    }
                }
                
                //If we got to here we can't find the log that was asked for
                request->send(404);
            }); 

            server.on("/log_delete", HTTP_GET, [](AsyncWebServerRequest *request) {

                if(request->hasParam("id", false))
                {
                    AsyncWebParameter* logid = request->getParam("id", false);

                    if (logid->value() == "rawlog")
                    {
                        bool restartLogging = false;
                        if (CANServer::CanBus::logRawCan)
                        {
                            //If we are logging we need to stop logging so we can remove the file and then start again after
                            restartLogging = true;
                            CANServer::CanBus::logRawCan = false;
                            CANServer::CanBus::closeRawLog();
                            CANServer::CanBus::saveSettings();
                        }  

                        SD.remove("/" RAWCANLOGNAME);   

                        if (restartLogging)
                        {
                            CANServer::CanBus::logRawCan = true;
                            CANServer::CanBus::openRawLog();
                            CANServer::CanBus::saveSettings();
                        }     
                    }
                }
                
                request->redirect("/logs");
            }); 

            server.on("/logs_save", HTTP_POST, [](AsyncWebServerRequest * request) {
                
                bool requestedRawLogState = false;
                if (request->hasParam("rawlog", true))
                {
                    AsyncWebParameter* newValue = request->getParam("rawlog", true);                    
                    if (newValue->value() == "on")
                    {
                        requestedRawLogState = true;
                    }
                    else
                    {
                        requestedRawLogState = false;
                    }
                    
                }
                if (requestedRawLogState)
                {
                    if (CANServer::CanBus::logRawCan == false)
                    {
                        //We need to make sure the file gets opened
                        CANServer::CanBus::openRawLog();
                    }

                    CANServer::CanBus::logRawCan = true;
                    CANServer::CanBus::saveSettings();
                }
                else
                {
                    if (CANServer::CanBus::logRawCan == true)
                    {
                        //We need to make sure the file gets closed
                        CANServer::CanBus::closeRawLog();
                    }

                    CANServer::CanBus::logRawCan = false;
                    CANServer::CanBus::saveSettings();
                }

                request->redirect("/logs");
            });            


            //Display related URL handling
            // set up servers for displays
            server.on("/disp0", HTTP_GET, [](AsyncWebServerRequest *request){
                if (CANServer::VehicleState::instance()->DisplayOn)
                {
                    request->send("text/plain", CANServer::DisplayState::display0->displayStringLength(), [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                        if (index > 0) return 0;  //Since we know our data will fit in a single call we just return 0 if the index is >0 to show that we are done

                        size_t dataCoppied = fmin(maxLen, CANServer::DisplayState::display0->displayStringLength());
                        memcpy(buffer, CANServer::DisplayState::display0->displayString(), dataCoppied);
                        return dataCoppied;

                        }, _displayTemplateProcessor);
                }
                else
                {
                    request->send(200, "text/plain", CANServer::DisplayState::offDisplayString());
                }
            });
            server.on("/disp1", HTTP_GET, [](AsyncWebServerRequest *request){
                if (CANServer::VehicleState::instance()->DisplayOn)
                {
                    request->send("text/plain", CANServer::DisplayState::display1->displayStringLength(), [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                        if (index > 0) return 0;  //Since we know our data will fit in a single call we just return 0 if the index is >0 to show that we are done

                        size_t dataCoppied = fmin(maxLen, CANServer::DisplayState::display1->displayStringLength());
                        memcpy(buffer, CANServer::DisplayState::display1->displayString(), dataCoppied);
                        return dataCoppied;

                        }, _displayTemplateProcessor);
                }
                else
                {
                    request->send(200, "text/plain", CANServer::DisplayState::offDisplayString());
                }
            });
            server.on("/disp2", HTTP_GET, [](AsyncWebServerRequest *request){
                if (CANServer::VehicleState::instance()->DisplayOn)
                {
                    request->send("text/plain", CANServer::DisplayState::display2->displayStringLength(), [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                        if (index > 0) return 0;  //Since we know our data will fit in a single call we just return 0 if the index is >0 to show that we are done

                        size_t dataCoppied = fmin(maxLen, CANServer::DisplayState::display2->displayStringLength());
                        memcpy(buffer, CANServer::DisplayState::display2->displayString(), dataCoppied);
                        return dataCoppied;

                        }, _displayTemplateProcessor);
                }
                else
                {
                    request->send(200, "text/plain", CANServer::DisplayState::offDisplayString());
                }
            });

            //receive posts of display buttons, TODO do something with the buttons
            server.on("/post0", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
                    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

                /*Serial.print("POST0: ");
                for (size_t i = 0; i < len; i++) {
                    Serial.write(data[i]);
                }
                Serial.println();*/
                request->send(200);
            });
            server.on("/post1", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
                    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
                
                /*Serial.print("POST1: ");
                for (size_t i = 0; i < len; i++) {
                    Serial.write(data[i]);
                }
                Serial.println();*/
                request->send(200);
            });
            server.on("/post2", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL,
                    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
                
                /*Serial.print("POST2: ");
                for (size_t i = 0; i < len; i++) {
                    Serial.write(data[i]);
                }
                Serial.println();*/
                request->send(200);
            });
            
           


            //Static content related URL handling
            server.on("/js/zepto.min.js", HTTP_GET,  [](AsyncWebServerRequest *request){
                AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/js/zepto.min.js.gz", "text/javascript");
                response->addHeader("Content-Encoding", "gzip");
                response->addHeader("Cache-Control", "max-age=600");
                request->send(response);
            });


            // Start server
            server.begin();

            Serial.println("Done");
        }


#define TEMPLATEMATCHHELPER(name) if (var == #name)\
                    {\
                        return String(vehicleStateInstance->name);\
                    }
        
        String _displayTemplateProcessor(const String& var)
        {
            CANServer::VehicleState *vehicleStateInstance = CANServer::VehicleState::instance();

            TEMPLATEMATCHHELPER(BattVolts)
            TEMPLATEMATCHHELPER(BattAmps)
            TEMPLATEMATCHHELPER(BattPower)
            TEMPLATEMATCHHELPER(RearTorque)
            TEMPLATEMATCHHELPER(FrontTorque)
            TEMPLATEMATCHHELPER(MinBattTemp)
            TEMPLATEMATCHHELPER(BattCoolantRate)
            TEMPLATEMATCHHELPER(PTCoolantRate)
            TEMPLATEMATCHHELPER(MaxRegen)
            TEMPLATEMATCHHELPER(MaxDisChg)
            
            if (var == "VehSpeed")
            {
                if (vehicleStateInstance->SpeedUnit == 1) 
                { 
                    //MPH
                    return String(int(0.621371 * vehicleStateInstance->VehSpeed));
                } 
                else 
                {
                    //KMH
                    return String(vehicleStateInstance->VehSpeed);
                }
            }

            if (var == "SpeedUnit")
            {
                if (vehicleStateInstance->SpeedUnit == 1) 
                { 
                    //MPH
                    return "HPM";
                } 
                else 
                {
                    //KMH
                    return "HMK";
                }
            }

            TEMPLATEMATCHHELPER(SpeedUnit)
            TEMPLATEMATCHHELPER(v12v261)
            TEMPLATEMATCHHELPER(BattCoolantTemp)
            TEMPLATEMATCHHELPER(PTCoolantTemp)
            TEMPLATEMATCHHELPER(BattRemainKWh)
            TEMPLATEMATCHHELPER(BattFullKWh)
            TEMPLATEMATCHHELPER(InvHStemp376)
            TEMPLATEMATCHHELPER(BSR)
            TEMPLATEMATCHHELPER(BSL)
            TEMPLATEMATCHHELPER(DisplayOn)

            //Some scaled vars that are used by the default bargraph display
            if (var == "BattPower_Scaled_Bar")
            {
                return String(int(0.008 * vehicleStateInstance->BattPower));
            }

            if (var == "RearTorque_Scaled_Bar")
            {
                return String(int(0.006 * vehicleStateInstance->RearTorque));
            }

            Serial.print("Didn't match variable: ");
            Serial.println(var);
            return String();
        }
    }
}