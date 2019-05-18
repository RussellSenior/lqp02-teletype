#include "ps2_Keyboard.h"
#include "ps2_NeutralTranslator.h"
#include "ps2_SimpleDiagnostics.h"
// #include "ps2_KeyboardLeds.h"

#define Printer Serial1
#define Computer Serial2

#define ESCAPE  0x1B

static const int clockPin = 4;
static const int dataPin = 5;
bool debug = 0;
bool local = 0;

int sendEscapeSequence(int f) {
    Serial.printf("sending escape sequence %d\n",f);
    switch(f) {
        case 1: // reset printer
            Printer.printf("%cc",ESCAPE);
            break;
        case 2: // advance paper to next line
            Printer.printf("%cE",ESCAPE);
            break;
        case 3: // set right margin wrap mode
            Printer.printf("%c[?7h",ESCAPE);
            break;
        case 4: // reset right margin wrap mode
            Printer.printf("%c[?7l",ESCAPE);
            break;
        case 5: // enter new line mode
            Printer.printf("%c[20h",ESCAPE);
            break;
        case 6: // reset new line mode
            Printer.printf("%c[20l",ESCAPE);
            break;
        case 7: // set 8 column tabs
            Printer.printf("%c[8;16;24;32;40;48;56;64;72;80u",ESCAPE);
            break;
        case 8: // unset column tabs
            Printer.printf("%c[3g",ESCAPE);
            break;
        case 9: // set 15 char/inch
            Printer.printf("%c[9w",ESCAPE);
            break;
        case 10: // form feed
            Printer.printf("%c",0x0c);
            break;
        case 11: // line feed
            Printer.printf("%cD",ESCAPE);
            break;
        case 12: // reverse line feed
            Printer.printf("%cM",ESCAPE);
            break;
        default: return -1;
    }
    return 0;
}


int16_t unshiftTable[70] = {
  0x7F, 0x1B, 0x08, 0x09, 0x0A, 0x20, 
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x2E, 0x0A, 0x2B, 0x2D, 0x2A, 0x2F,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x27, 0x2C, 0x2D, 0x2E, 0x2F, -1,
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
  0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
  0x7A, 0x3B, 0x5C, 0x5B, 0x5D, 0x3D
};

int16_t shiftTable[70] = {
  0x7F, 0x1B, 0x08, -1, 0x0A, 0x20, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1,
  0x28, 0x21, 0x40, 0x23, 0x24, 0x25, 0x5E, 0x26, 0x2A, 0x28,
  0x22, 0x3C, 0x5F, 0x3E, 0x3F, -1,
  0x7E, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
  0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
  0x5A, 0x3A, 0x7C, 0x7B, 0x7D, 0x2B
};

int16_t keycode2ascii(uint16_t c) {
    // check for special bits so we can bail
    if (c & ~0x607F) {
        return -1;
    }

    if (!(c & 0x2000)) {
        int d = (c & 0x7f) - 0x1A;
        if (d > 0x46 && d <= 0x52) { // I'm a function key
            sendEscapeSequence(c - 0x60);
        }
        if (d >= 0 && d < sizeof(shiftTable)) {
            if (c & 0x4000) {
                return shiftTable[d];
            } else {
                return unshiftTable[d];
            }
        } 
    } else {
    // handle CTRL
        // for CTRL-letter, nuke shift bit
        if ((c & 0xFF) >= 0x41 && (c & 0xFF) <= 0x5A) {
            c = c & ~0x4000; 
        }
        if (c >= 0x2041 && c <= 0x205A) {
            return c & 0x1F;
        }
        switch (c) {
            case 0x6032: // ^@
                return 0;
            case 0x205D: // ^[
                return 0x1B;
            case 0x205C: // ^BACKSLASH
                return 0x1C;
            case 0x205E: // ^]
                return 0x1D;
            case 0x6036: // ^^
                return 0x1E;
            case 0x603C: // ^_
                return 0x1F;
        }
    }
    return -1;
}


typedef ps2::SimpleDiagnostics<32> Diagnostics;
static Diagnostics diagnostics;
static ps2::Keyboard<dataPin,clockPin,16,Diagnostics> ps2Keyboard(diagnostics);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    ps2Keyboard.begin();
    Printer.begin(9600,SERIAL_7E1);
    Printer.attachRts(2);
    Printer.attachCts(20);
    sendEscapeSequence(1); // reset
    sendEscapeSequence(7); // set tabstops
    sendEscapeSequence(9); // set 15 chars/inch
    Computer.begin(1200,SERIAL_8N1);
    Computer.attachRts(11);
    Computer.attachCts(23);
}

static ps2::NeutralTranslator translator;
bool pxon = 1;

void loop() {
    // diagnostics.setLedIndicator<LED_BUILTIN, ps2::DiagnosticsLedBlink::heartbeat>();
    int fromComputer;
    int fromPrinter;

    if (Computer.available() > 0 && pxon) {
        fromComputer = Computer.read();
        if (!local) {
            Printer.write(fromComputer);
        }
        if (debug) {
           Serial.printf("computer: 0x%02x %c",fromComputer,fromComputer);
        }
    }
    if (Printer.available() > 0) {
        fromPrinter = Printer.read();
        fromPrinter &= 0x7F;
        switch (fromPrinter) {
            case 0x13:
                pxon = 0;
                break;
            case 0x11:
                pxon = 1;
                break;
        }
        Serial.printf("printer: 0x%02x %c\n",fromPrinter,fromPrinter);
    }

    ps2::KeyboardOutput scanCode = ps2Keyboard.readScanCode();
    if (scanCode != ps2::KeyboardOutput::none) {
        ps2::KeyCode translated = translator.translatePs2Keycode(scanCode);
        if (translated != ps2::KeyCode::PS2_NONE) {
            int c = keycode2ascii(translated);
            if (c >= 0) {
                if (debug) {
                Serial.printf("keyboard: 0x%02x %c\n",c,c);
                }

                if (!local) {
                    Computer.write(c);
                } else {
                    Printer.write(c);
                }
            }
        }
    }
}
