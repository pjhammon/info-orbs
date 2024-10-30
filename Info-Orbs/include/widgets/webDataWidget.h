#ifndef WEB_DATA_WIDGET_H
#define WEB_DATA_WIDGET_H

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <widget.h>

#include "model/webDataModel.h"
#include "utils.h"

class WebDataWidget : public Widget
{
public:
    WebDataWidget(ScreenManager &manager, String url);
    ~WebDataWidget() override;
    void setup() override;
    void update(bool force = false) override;
    void draw(bool force = false) override;
    void changeMode() override;

private:
    int m_lastUpdate = 0;
    int m_updateDelay = 10000; // in milliseconds
    String httpRequestAddress;
//    WebDataModel m_obj[5];
    int32_t m_defaultColor = TFT_WHITE;
    int32_t m_defaultBackground = TFT_BLACK;
    String version;
    String server_ip;
    String server_port;

    String frequentPlayers[4];
    String trimStringToLength(const String &input, size_t length);

    // Declare a backing DynamicJsonDocument
    JsonDocument onlinePlayersDoc;
    // Declare onlinePlayers as a JsonArray reference
    JsonArray onlinePlayers;

    bool containsElement(JsonArray &array, String target);
    void sortJsonArray(JsonArray &array);
    bool arraysAreEqual(JsonArray &array1, JsonArray &array2);
    void copyJsonArray(JsonArray &sourceArray, JsonArray &destinationArray);
    const int centre = 120; // centre location of the screen(240x240)
    bool dataHasChanged;
};
#endif // WEB_DATA_WIDGET_H