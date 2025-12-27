#pragma once
#include "Arduino.h"
#include <WiFi.h>
#include <time.h>

void setup_local_time();
bool wifi_connect();
void wifi_diconnect();