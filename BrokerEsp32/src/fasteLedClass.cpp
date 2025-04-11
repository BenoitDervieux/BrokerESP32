#include <fastLedClass.h>


CRGB leds[NUM_LEDS];

FastLedClass::FastLedClass() {
    
}

void FastLedClass::init() {
    FastLED.addLeds<WS2812B, DATA_PIN_1, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    fill_solid(leds, NUM_LEDS, CRGB::Blue);
    FastLED.show();
}

void FastLedClass::changeColor(String colorToConvert) {
    CRGB color = hexToCRGB(this->decimalStringToHex(colorToConvert));
	fill_solid(leds, NUM_LEDS, color);
	FastLED.show();
}

void FastLedClass::setToPurple() {
	fill_solid(leds, NUM_LEDS, CRGB::Purple);
	FastLED.show();
}




CRGB FastLedClass::hexToCRGB(const String& hex) {
    // Ensure the hex string is valid and properly formatted
    String cleanedHex = hex;
    if (cleanedHex.startsWith("#")) {
        cleanedHex.remove(0, 1); // Remove '#'
    }
    uint32_t color = strtoul(cleanedHex.c_str(), nullptr, 16);
    return CRGB(
        (color >> 16) & 0xFF,  // Red
        (color >> 8) & 0xFF,   // Green
        color & 0xFF           // Blue
    );
}

String FastLedClass::decimalStringToHex(String decimalString) {
    // Convert the decimal string to an integer
    int decimalColor = decimalString.toInt();

    // Use sprintf to format the integer as a hexadecimal string
    char hexColor[8];  // Enough space for a 6-digit hex number and the null terminator
    sprintf(hexColor, "#%06X", decimalColor);
    
    // Return the hexadecimal string
    return String(hexColor);
}
