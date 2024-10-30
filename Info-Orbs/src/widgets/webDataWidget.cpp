#include "widgets/webDataWidget.h"

WebDataWidget::WebDataWidget(ScreenManager &manager, String url) : Widget(manager)
{
    httpRequestAddress = url;

    m_lastUpdate = 0;

    dataHasChanged = false;

    frequentPlayers[0] = "NetheriteCrown1";
    frequentPlayers[1] = "MrswaggoldenBut";
    frequentPlayers[2] = "AaaaaTaco";
    frequentPlayers[3] = "pjhammond";

    // Initialize onlinePlayers as an array within the JsonDocument
    onlinePlayers = onlinePlayersDoc.to<JsonArray>();

    // for (int i = 0; i < 5; i++)
    // {
    //     m_obj[i] = WebDataModel();
    // }
}

WebDataWidget::~WebDataWidget()
{
}

void WebDataWidget::setup()
{
}

void WebDataWidget::changeMode()
{
}

void WebDataWidget::draw(bool force)
{
    // for (int i = 0; i < 5; i++) {
    //     WebDataModel *data = &m_obj[i];
    //     if (force) {
    //         data->setInitializedStatus(false);
    //     }
    //     if (data->isChanged() || force) {
    //         m_manager.selectScreen(i);
    //         data->draw(m_manager.getDisplay());

    //         data->setChangedStatus(false);
    //     }
    // }

    //    Serial.println("About to draw widget...");

    if (force || dataHasChanged)
    {
        // Serial.println("Drawing widget...");
        int16_t bgColor = TFT_BLACK;
        int16_t textColor = TFT_WHITE;

        // Display 0
        m_manager.selectScreen(0);
        TFT_eSPI &display = m_manager.getDisplay();

        display.fillScreen(bgColor);
        display.setTextColor(textColor);
        display.setTextSize(3);
        display.setTextFont(8);
        display.setTextDatum(MC_DATUM);
        display.drawString(version, centre, 80, 2);
        display.setTextSize(2);
        display.setTextFont(1);
        display.drawString(server_ip, centre, 120);
        display.drawString("Port: " + server_port, centre, 140);

        // Player displays 1-4
        for (int i = 0; i < 4; i++)
        {
            int displayID = i + 1;
            m_manager.selectScreen(displayID);
            TFT_eSPI &display = m_manager.getDisplay();

            display.fillScreen(bgColor);
            display.setTextColor(textColor);
            display.setTextSize(3);
            display.setTextDatum(MC_DATUM);

            String playerName = frequentPlayers[i];
            // Serial.println("Player name: " + playerName);
            display.drawString(trimStringToLength(playerName, 10), centre, 80, 2);

            bool playerFound = containsElement(onlinePlayers, playerName);

            String playerOnlineStatus;
            int playerOnlineStatusColor;

            if (playerFound)
            {
                playerOnlineStatusColor = TFT_GREEN;
                playerOnlineStatus = "Online";
            }
            else
            {
                playerOnlineStatusColor = TFT_RED;
                playerOnlineStatus = "Offline";
            }

            // Color the background depending on player status?
            // display.fillScreen(bgColor);

            // Show their status
            display.setTextColor(playerOnlineStatusColor);
            display.setTextSize(2);
            display.setTextFont(1);
            display.drawString(playerOnlineStatus, centre, 120);
        }

        dataHasChanged = false;
    }
}

String WebDataWidget::trimStringToLength(const String &input, size_t length)
{
    // Check if the input string exceeds the specified length
    if (input.length() > length)
    {
        // Truncate the string and add "..."
        return input.substring(0, length - 3) + "..."; // Adjust length to account for "..."
    }
    // Return the original string if it's within the specified length
    return input;
}

bool WebDataWidget::containsElement(JsonArray &array, String target)
{
    // Serial.println("Array has size: " + String(array.size()));
    for (JsonVariant value : array)
    {
        // Serial.println("containsElement ... Comparing " + value.as<String>() + " to " + target);
        if (value.as<String>() == target)
        {
            // Serial.println("containsElement ... They match!");
            return true;
        }
    }
    // Serial.println("containsElement ... Not found!");
    return false;
}

void WebDataWidget::update(bool force)
{
    // Serial.println("About to update widget...");
    if (force || m_lastUpdate == 0 || (millis() - m_lastUpdate) >= m_updateDelay)
    {
        Serial.println("Updating widget...");
        HTTPClient http;

        http.begin(httpRequestAddress);
        int httpCode = http.GET();

        Serial.println("httpCode: " + String(httpCode));
        dataHasChanged = false;

        if (httpCode > 0)
        { // Check for the returning code
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, http.getString());
            Serial.println("Response: " + String(error.f_str()));
            if (!error)
            {
                // Serial.println("Full JSON received: ");
                // serializeJsonPretty(doc, Serial); // Print entire JSON to check structure

                // Fetch version
                if (doc["version"] && (doc["version"].as<String>() != version))
                {
                    Serial.println("Found new version: " + doc["version"].as<String>());
                    version = doc["version"].as<String>();
                    dataHasChanged = true;
                }
                // Fetch server IP address
                if (doc["ip"] && (doc["ip"].as<String>() != server_ip))
                {
                    Serial.println("Found new IP: " + doc["ip"].as<String>());
                    server_ip = doc["ip"].as<String>();
                    dataHasChanged = true;
                }
                // Fetch server port
                if (doc["port"] && (doc["port"].as<String>() != server_port))
                {
                    Serial.println("Found new port: " + doc["port"].as<String>());
                    server_port = doc["port"].as<String>();
                    dataHasChanged = true;
                }

                // Check for player list
                if (doc["players"]["list"].is<JsonArray>())
                {
                    JsonArray currentPlayers = doc["players"]["list"].as<JsonArray>();
                    Serial.println("Found player list.");
                    serializeJson(currentPlayers, Serial); // Print the player list for debugging

                    if (!arraysAreEqual(currentPlayers, onlinePlayers))
                    {
                        Serial.println("Player list has changed.");
                        copyJsonArray(currentPlayers, onlinePlayers);
                        dataHasChanged = true;
                        // Serial.println("onlinePlayers array now contains " + String(onlinePlayers.size()) + " elements.");
                    }
                }
                else
                {
                    Serial.println("Player list not found or is not an array.");
                    onlinePlayers.clear(); // Clear the array if list is missing
                    dataHasChanged = true;
                }

                // debug add some players to onlinePlayers
                // onlinePlayers.add(frequentPlayers[0]);
                // onlinePlayers.add(frequentPlayers[1]);
                // dataHasChanged = true;
                // Serial.println("onlinePlayers array has " + String(onlinePlayers.size()) + " elements.");

                // for (int i = 0; i < array.size(); i++) {
                //     m_obj[i].parseData(array[i].as<JsonObject>(), m_defaultColor, m_defaultBackground);
                // }
                m_lastUpdate = millis();
            }
            else
            {
                // Handle JSON deserialization error
                Serial.println("deserializeJson() failed");
            }
        }
        else
        {
            // Handle HTTP request error
            Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
}

// Helper function to sort a JsonArray in-place (modifies the original array)
void WebDataWidget::sortJsonArray(JsonArray &array)
{
    // Simple Bubble Sort for demonstration
    for (size_t i = 0; i < array.size() - 1; i++)
    {
        for (size_t j = 0; j < array.size() - i - 1; j++)
        {
            if (array[j] > array[j + 1])
            {
                JsonVariant temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

void WebDataWidget::copyJsonArray(JsonArray &sourceArray, JsonArray &destinationArray)
{
    destinationArray.clear(); // Clear existing elements in destination
    // Serial.println("Emptied desatination array.");

    for (JsonVariant v : sourceArray)
    {
        destinationArray.add(v); // Copy each element from source to destination
        // Serial.println("Added element: " + v.as<String>());
        // Serial.println("Desitnation array now contains " + String(destinationArray.size()) + " elements.");
    }
    // Serial.println("Desitnation array ends with " + String(destinationArray.size()) + " elements.");
}

// Function to check if two JsonArrays are equal after sorting
bool WebDataWidget::arraysAreEqual(JsonArray &array1, JsonArray &array2)
{
    // Check if both arrays are the same size
    if (array1.size() != array2.size())
    {

        Serial.println("Array sizes are different: " + String(array1.size()) + " vs " + String(array2.size()));
        return false;
    }

    // Create temporary copies of the arrays to sort them without modifying originals
    JsonDocument doc1;
    JsonArray sortedArray1 = doc1.to<JsonArray>();
    JsonDocument doc2;
    JsonArray sortedArray2 = doc2.to<JsonArray>();

    // Copy elements from original arrays into temporary arrays
    for (JsonVariant v : array1)
        sortedArray1.add(v);
    for (JsonVariant v : array2)
        sortedArray2.add(v);

    // Sort the temporary arrays
    sortJsonArray(sortedArray1);
    // serializeJsonPretty(sortedArray1, Serial); // Print entire JSON to check structure
    sortJsonArray(sortedArray2);
    // serializeJsonPretty(sortedArray2, Serial); // Print entire JSON to check structure

    // Compare each element in the sorted arrays
    for (size_t i = 0; i < sortedArray1.size(); i++)
    {

        // Serial.println("arraysAreEqual ... Comparing: '" + sortedArray1[i].as<String>() + "' vs '" + sortedArray2[i].as<String>() + "'");
        if (sortedArray1[i].as<String>() != sortedArray2[i].as<String>())
        {
            // Serial.println("Strings are not equal");
            return false;
        }
    }

    // Serial.println("Arrays are equal");
    return true;
}