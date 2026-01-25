#include "ESP-FTP-Server-Lib.h"
#include "FTPFilesystem.h"
#include "SD.h"
#include "s3_geek_ftp.h"
#include "s3_geek_graphics.h"

#define FTP_USER "esp32"
#define FTP_PASSWORD "esp32"

FTPServer ftp;
fs::FS *ftpfs = &SD;
extern S3GeekGraphics graphics;

void setup_ftp()
{
    ftp.addUser(FTP_USER, FTP_PASSWORD);
    ftp.addFilesystem("SD", ftpfs);
    ftp.begin();

    graphics.postMessage("Started ftp ... ");
}

bool ftp_was_connected = false;

void loop_ftp()
{
    ftp.handle();
    bool connected_now = ftp.countConnections() > 0;

    // Only print message if state changed
    if (connected_now && !ftp_was_connected)
    {
        graphics.postMessage("FTP client connected");
        Serial.println("FTP client connected");
    }
    else if (!connected_now && ftp_was_connected)
    {
        graphics.postMessage("FTP client disconnected");
        Serial.println("FTP client disconnected");
    }

    ftp_was_connected = connected_now; // save state for next loop
}