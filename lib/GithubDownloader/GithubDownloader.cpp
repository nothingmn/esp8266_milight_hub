#include <GithubDownloader.h>
#include <fs.h>
  
bool GithubDownloader::downloadFile(const String& path, Stream& dest) {
  if (!client.connect(GITHUB_RAW_DOMAIN, 443)) {
    Serial.println("Failed to connect to github over HTTPS.");
    return false;
  }
  
  if (!client.verify(GITHUB_SSL_FINGERPRINT, GITHUB_RAW_DOMAIN)) {
    Serial.println("Failed to verify github certificate");
    return false;
  }
  
  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + GITHUB_RAW_DOMAIN + "\r\n" +
               "Connection: close\r\n\r\n");
               
  yield();
  
  if (client.connected()) {
    client.find("\r\n\r\n", 4);
  }
               
  while (client.connected()) {
    size_t l = client.read(buffer, GITHUB_DOWNLOADER_BUFFER_SIZE);
    dest.write(buffer, l);
    yield();
  }
  
  return true;
}

bool GithubDownloader::downloadFile(const String& username, const String& repo, const String& repoPath, Stream& dest) {
  String path = String("/") + username + "/" + repo + "/master/" + repoPath;
  return downloadFile(path, dest);
}

bool GithubDownloader::downloadFile(const String& username, const String& repo, const String& repoPath, const String& fsPath) {
  File f = SPIFFS.open(fsPath.c_str(), "w");
  
  if (!f) {
    Serial.print("ERROR - could not open file for downloading: ");
    Serial.println(fsPath);
    return false;
  }
  
  if (!downloadFile(username, repo, repoPath, f)) {
    f.close();
    return false;
  }
  
  f.close();
  
  return true;
}