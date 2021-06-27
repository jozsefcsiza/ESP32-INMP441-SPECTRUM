#define BLUETOOTH_NAME				"Esp32 Jozsi"	//Bluetooth name -> Give any name you want

//Bluetooth communication keys
String BT_SPECTRUM_AUTO_PATTERN =	"sap";
String BT_SPECTRUM_NEXT_PATTERN =	"snp";
String BT_SPECTRUM_BRIGHTNESS =		"sbr";
String BT_SPECTRUM_GAIN =			"sga";
String BT_SPECTRUM_SQUELCH =		"ssq";
String BT_SPECTRUM_RESET =			"srs";

String BT_NO_DATA =					"";
String BT_BUTTON =					"";

String BT_GET_ESP_DATA =			"esp";
String BT_ESP_DEEP_SLEEP =			"dsl";

#define O                   -1 //No data
int bluetoothData;

#include "BluetoothSerial.h"
/* Check if Bluetooth configurations are enabled in the SDK */
/* If not, then you have to recompile the SDK */
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

