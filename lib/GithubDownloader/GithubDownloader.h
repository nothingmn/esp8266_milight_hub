#include <Arduino.h>
#include <WiFiClientSecure.h>

#ifndef _GITHUB_DOWNLOADER
#define _GITHUB_DOWNLOADER 

#define GITHUB_DOWNLOADER_BUFFER_SIZE 200

#define GITHUB_SSL_FINGERPRINT "21 99 13 84 63 72 17 13 B9 ED 0E 8F 00 A5 9B 73 0D D0 56 58"
#define GITHUB_RAW_DOMAIN "raw.githubusercontent.com"

class GithubDownloader {
public:
  bool downloadFile(const String& url, Stream& dest);
  bool downloadFile(const String& username, const String& repo, const String& path, Stream& dest);
  bool downloadFile(const String& username, const String& repo, const String& path, const String& fsPath);
  
private:
  WiFiClientSecure client;
  uint8_t buffer[GITHUB_DOWNLOADER_BUFFER_SIZE];
};

#endif