// TTS credentials(tts_v3)
// https://console.cloud.thethings.network/

#define USE_OTAA

#ifndef USE_OTAA
// ABP
static const char DEVADDR[] = "260B4F2E";
static const char NWKSKEY[] = "C8C2CE3ADE475B282995A5937B5CA437";
static const char APPSKEY[] = "4E6D2ADA888BF79B8C0CC6E14EF9A5B6";
#else

// OTAA
static const char APPEUI[] = "0000000000000000";
static const char DEVEUI[] = "70B3D57ED0045DDA";
static const char APPKEY[] = "5340769C09F900173852BEB4DBAA8B59";
#endif
