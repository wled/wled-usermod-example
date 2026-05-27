#include "wled.h"

/*
 * Usermods allow you to add own functionality to WLED without touching core source files.
 * See the WLED docs: https://kno.wled.ge/advanced/custom-features/
 *
 * This is an example usermod. It demonstrates:
 *   - persistent settings via addToConfig() / readFromConfig()
 *   - JSON state read/write via addToJsonState() / readFromJsonState()
 *   - MQTT subscribe and message handling (guarded by WLED_DISABLE_MQTT)
 *   - button event handling
 *   - the Usermod Settings page via appendConfigData()
 *
 * To create your own usermod:
 *   1. Click "Use this template" on https://github.com/wled/wled-usermod-example to create your own repo.
 *   2. Rename the class and file to something descriptive.
 *   3. Reference your new repo in platformio_override.ini via custom_usermods.
 *
 * REGISTER_USERMOD() at the bottom self-registers the instance — no other
 * file edits are needed.
 */

//class name. Use something descriptive and leave the ": public Usermod" part :)
class MyExampleUsermod : public Usermod {

  private:

    // Private class members. You can declare variables and functions only accessible to your usermod here
    bool enabled = false;
    bool initDone = false;
    unsigned long lastTime = 0;

    // config variables — boot defaults can be set here or inside readFromConfig()
    bool testBool = false;
    unsigned long testULong = 42424242;
    float testFloat = 42.42;
    String testString = "Forty-Two";
    uint16_t greatValue = 0;  // example persistent value exposed in JSON state

    // These config variables have defaults set inside readFromConfig()
    int testInt;
    long testLong;
    int8_t testPins[2];

    // string that are used multiple time (this will save some flash memory)
    static const char _name[];
    static const char _enabled[];


    // any private methods should go here (non-inline method should be defined out of class)
    void publishMqtt(const char* state, bool retain = false); // example for publishing MQTT message


  public:

    // non WLED related methods, may be used for data exchange between usermods (non-inline methods should be defined out of class)

    /**
     * Enable/Disable the usermod
     */
    inline void enable(bool enable) { enabled = enable; }

    /**
     * Get usermod enabled/disabled state
     */
    inline bool isEnabled() { return enabled; }

    // To access this usermod from another usermod, cast the result of UsermodManager::lookup():
    //   MyExampleUsermod* um = (MyExampleUsermod*) UsermodManager::lookup(USERMOD_ID_MYUSERMOD);
    // Make sure to assign a unique ID in getId()!


    /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * readFromConfig() is called prior to setup()
     * You can use it to initialize variables, sensors or similar.
     */
    void setup() override {
      // do your set-up here
      //Serial.println("Hello from my usermod!");
      initDone = true;
    }


    /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
    void connected() override {
      //Serial.println("Connected to WiFi!");
    }


    /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
    void loop() override {
      // if usermod is disabled or called during strip updating just exit
      // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
      if (!enabled || strip.isUpdating()) return;

      // do your magic here
      if (millis() - lastTime > 1000) {
        //Serial.println("I'm alive!");
        lastTime = millis();
      }
    }


    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root) override
    {
      // if "u" object does not exist yet wee need to create it
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      //this code adds "u":{"ExampleUsermod":[20," lux"]} to the info object
      //int reading = 20;
      //JsonArray lightArr = user.createNestedArray(FPSTR(_name))); //name
      //lightArr.add(reading); //value
      //lightArr.add(F(" lux")); //unit

      // if you are implementing a sensor usermod, you may publish sensor data
      //JsonObject sensor = root[F("sensor")];
      //if (sensor.isNull()) sensor = root.createNestedObject(F("sensor"));
      //temp = sensor.createNestedArray(F("light"));
      //temp.add(reading);
      //temp.add(F("lux"));
    }


    /*
     * addToJsonState() adds entries to the /json/state response. Clients can read and write these.
     * Use this to expose runtime state that should be controllable via the API.
     * addToJsonState() is NOT called for presets — use addToConfig() for persistent values.
     */
    void addToJsonState(JsonObject& root) override
    {
      if (!initDone || !enabled) return;  // prevent crash on boot applyPreset()

      JsonObject usermod = root[FPSTR(_name)];
      if (usermod.isNull()) usermod = root.createNestedObject(FPSTR(_name));

      usermod["greatValue"] = greatValue;
    }


    /*
     * readFromJsonState() receives values a client POSTs to /json/state.
     * The JSON key nesting matches what addToJsonState() writes — clients send back the same structure.
     */
    void readFromJsonState(JsonObject& root) override
    {
      if (!initDone) return;  // prevent crash on boot applyPreset()

      JsonObject usermod = root[FPSTR(_name)];
      if (!usermod.isNull()) {
        // getJsonValue copies the value if present and returns true; leaves the variable unchanged if missing
        getJsonValue(usermod["greatValue"], greatValue);
      }
    }


    /*
     * addToConfig() saves settings to cfg.json under the "um" object. WLED calls this whenever settings are saved.
     * The Usermod Settings page in the UI is generated automatically from the keys you write here.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * To force a config write from loop(), call serializeConfig() — but use it sparingly (flash wear,
     * possible LED stutter). Never call it from a network callback.
     */
    void addToConfig(JsonObject& root) override
    {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_enabled)] = enabled;
      top["great"] = greatValue;
      top["testBool"] = testBool;
      top["testInt"] = testInt;
      top["testLong"] = testLong;
      top["testULong"] = testULong;
      top["testFloat"] = testFloat;
      top["testString"] = testString;
      JsonArray pinArray = top.createNestedArray("pin");
      pinArray.add(testPins[0]);
      pinArray.add(testPins[1]); 
    }


    /*
     * readFromConfig() is called before setup() and again after settings are saved.
     * Return false if any expected keys were missing — WLED will then call addToConfig() to write the defaults.
     * getJsonValue(src, dest) copies the value if present and returns true; leaves dest unchanged if missing.
     * getJsonValue(src, dest, default) also assigns a default when the key is absent.
     */
    bool readFromConfig(JsonObject& root) override
    {
      JsonObject top = root[FPSTR(_name)];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["great"], greatValue);
      configComplete &= getJsonValue(top["testBool"], testBool);
      configComplete &= getJsonValue(top["testULong"], testULong);
      configComplete &= getJsonValue(top["testFloat"], testFloat);
      configComplete &= getJsonValue(top["testString"], testString);

      // A 3-argument getJsonValue() assigns the 3rd argument as a default value if the Json value is missing
      configComplete &= getJsonValue(top["testInt"], testInt, 42);  
      configComplete &= getJsonValue(top["testLong"], testLong, -42424242);

      // "pin" fields have special handling in settings page (or some_pin as well)
      configComplete &= getJsonValue(top["pin"][0], testPins[0], -1);
      configComplete &= getJsonValue(top["pin"][1], testPins[1], -1);

      return configComplete;
    }


    /*
     * appendConfigData() is called when the Usermod Settings page renders.
     * Write JavaScript snippets to settingsScript to add helper text or dropdowns for your config fields.
     * addInfo('<ModName>:<key>', 1, '<html>') adds a tooltip/label next to the field.
     * addDropdown / addOption replace a plain text input with a <select>.
     */
    void appendConfigData(Print& settingsScript) override
    {
      settingsScript.print(F("addInfo('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F(":great',1,'<i>(this is a great config value)</i>');"));
      settingsScript.print(F("addInfo('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F(":testString',1,'enter any string you want');"));
      settingsScript.print(F("dd=addDropdown('")); settingsScript.print(FPSTR(_name)); settingsScript.print(F("','testInt');"));
      settingsScript.print(F("addOption(dd,'Nothing',0);"));
      settingsScript.print(F("addOption(dd,'Everything',42);"));
    }


    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    void handleOverlayDraw() override
    {
      //strip.setPixelColor(0, RGBW32(0,0,0,0)) // set the first pixel to black
    }


    /**
     * handleButton() can be used to override default button behaviour. Returning true
     * will prevent button working in a default way.
     * Replicating button.cpp
     */
    bool handleButton(uint8_t b) override {
      yield();
      // ignore certain button types as they may have other consequences
      if (!enabled
       || buttons[b].type == BTN_TYPE_NONE
       || buttons[b].type == BTN_TYPE_RESERVED
       || buttons[b].type == BTN_TYPE_PIR_SENSOR
       || buttons[b].type == BTN_TYPE_ANALOG
       || buttons[b].type == BTN_TYPE_ANALOG_INVERTED) {
        return false;
      }

      bool handled = false;
      // do your button handling here
      return handled;
    }
  

#ifndef WLED_DISABLE_MQTT
    /**
     * onMqttMessage() is called when a subscribed MQTT topic receives a message.
     * topic only contains stripped topic (part after /wled/MAC).
     * Return true to mark the message handled (prevents other usermods from seeing it).
     * These methods must be inside a #ifndef WLED_DISABLE_MQTT guard — MQTT support is a compile-time option.
     * See usermods/multi_relay for a well-structured subscribe-in-connect / handle-in-message example.
     */
    bool onMqttMessage(char* topic, char* payload) override {
      //if (strlen(topic) == 8 && strncmp_P(topic, PSTR("/command"), 8) == 0) {
      //  String action = payload;
      //  if (action == "on")     { enabled = true;  return true; }
      //  if (action == "off")    { enabled = false; return true; }
      //  if (action == "toggle") { enabled = !enabled; return true; }
      //}
      return false;
    }

    /**
     * onMqttConnect() is called when MQTT connection is established.
     * Subscribe to topics here; mqttDeviceTopic holds the device-specific prefix.
     */
    void onMqttConnect(bool sessionPresent) override {
      //char subuf[64];
      //if (mqttDeviceTopic[0] != 0) {
      //  strcpy(subuf, mqttDeviceTopic);
      //  strcat_P(subuf, PSTR("/command"));
      //  mqtt->subscribe(subuf, 0);
      //}
    }
#endif


    /**
     * onStateChanged() is used to detect WLED state change
     * @mode parameter is CALL_MODE_... parameter used for notifications
     */
    void onStateChange(uint8_t mode) override {
      // do something if WLED state changed (color, brightness, effect, preset, etc)
    }


    /*
     * getId() allows you to optionally give your usermod a unique ID.
     * The base class returns USERMOD_ID_UNSPECIFIED, which is correct for most custom usermods.
     * Override only if you need reliable cross-usermod lookup via UsermodManager::lookup()
     * and have multiple usermods with the same ID registered simultaneously.
     */
    // uint16_t getId() override { return USERMOD_ID_UNSPECIFIED; }

   //More methods can be added in the future, this example will then be extended.
   //Your usermod will remain compatible as it does not need to implement all methods from the Usermod base class!
};


// add more strings here to reduce flash memory usage
const char MyExampleUsermod::_name[]    PROGMEM = "ExampleUsermod";
const char MyExampleUsermod::_enabled[] PROGMEM = "enabled";


// implementation of non-inline member methods

void MyExampleUsermod::publishMqtt(const char* state, bool retain)
{
#ifndef WLED_DISABLE_MQTT
  //Check if MQTT Connected, otherwise it will crash the 8266
  if (WLED_MQTT_CONNECTED) {
    char subuf[64];
    strcpy(subuf, mqttDeviceTopic);
    strcat_P(subuf, PSTR("/example"));
    mqtt->publish(subuf, 0, retain, state);
  }
#endif
}

static MyExampleUsermod example_usermod;
REGISTER_USERMOD(example_usermod);
