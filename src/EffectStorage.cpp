#include "EffectStorage.h"
#include <FS.h>
#include <LittleFS.h>
#include <ctype.h>
 #include <Arduino.h>

namespace {
constexpr uint16_t kMaxTempFiles = 20;
}

void EffectStorage::begin() {
    Serial.println(F("[EffectStorage] begin"));
    if (!ensureLittleFs()) {
        return;
    }

    ensureDirectoryExists("/effects");
    ensureDirectoryExists("/tempEffects");
    initializeTempIndex();
}

String EffectStorage::sanitizeFilename(const String& name) const {
    Serial.println(F("[EffectStorage] sanitizeFilename"));
    String trimmed = name;
    trimmed.trim();
    while (trimmed.startsWith("/")) {
        trimmed.remove(0, 1);
    }
    String sanitized;
    sanitized.reserve(trimmed.length());
    for (size_t i = 0; i < trimmed.length(); ++i) {
        char c = trimmed.charAt(i);
        bool allowed = (c >= '0' && c <= '9') ||
                       (c >= 'A' && c <= 'Z') ||
                       (c >= 'a' && c <= 'z') ||
                       c == '_' || c == '-' || c == '.';
        sanitized += allowed ? c : '_';
    }
    sanitized.trim();
    return sanitized;
}

String EffectStorage::prepareFilename(const String& baseName, SaveMode mode) {
    Serial.println(F("[EffectStorage] prepareFilename"));
    String sanitized = sanitizeFilename(baseName);
    if (sanitized.isEmpty()) {
        return "";
    }

    if (mode == SaveMode::Temp) {
        return getTempFilename(sanitized);
    }

    if (mode == SaveMode::Persistent) {
        return sanitized;
    }

    return "";
}

String EffectStorage::getTempFilename(const String& baseName) {
    Serial.println(F("[EffectStorage] getTempFilename"));
    String sanitized = sanitizeFilename(baseName);
    if (sanitized.isEmpty()) {
        return "";
    }
    initializeTempIndex();
    String generated = sanitized + "_" + String(nextTempIndex);
    nextTempIndex++;
    return generated;
}

String EffectStorage::getFilename(const String& baseName) {
    Serial.println(F("[EffectStorage] getFilename"));
    return getTempFilename(baseName);
}

bool EffectStorage::saveFile(const uint8_t* buffer, size_t size, const String& baseName,
                             const String& finalFilename, SaveMode mode, String* outPath) {
    Serial.println(F("[EffectStorage] saveFile"));
    if (mode == SaveMode::None) {
        return true;
    }

    if (!ensureLittleFs()) {
        return false;
    }

    const char* targetDir = directoryForMode(mode);
    if (!ensureDirectoryExists(targetDir)) {
        return false;
    }

    String sanitizedBase = sanitizeFilename(baseName);
    if (sanitizedBase.isEmpty()) {
        Serial.println(F("[EffectStorage] saveFile: empty base after sanitize"));
        return false;
    }

    if (baseNameExists(targetDir, sanitizedBase)) {
        Serial.println(F("[EffectStorage] saveFile: base already exists"));
        return false;
    }

    if (mode == SaveMode::Temp && !ensureTempCapacity(kMaxTempFiles)) {
        Serial.println(F("[EffectStorage] saveFile: prune failed"));
        return false;
    }

    String resolvedFilename = finalFilename;
    if (resolvedFilename.isEmpty()) {
        resolvedFilename = prepareFilename(baseName, mode);
        if (resolvedFilename.isEmpty()) {
            return false;
        }
    }

    String path = String(targetDir) + "/" + resolvedFilename;
    File outFile = LittleFS.open(path, FILE_WRITE);
    if (!outFile) {
        Serial.print(F("Failed to open file for writing: "));
        Serial.println(path);
        return false;
    }

    size_t written = outFile.write(buffer, size);
    outFile.close();
    if (written != size) {
        Serial.println(F("Failed to write entire buffer, removing partial file"));
        LittleFS.remove(path);
        return false;
    }

    if (outPath) {
        *outPath = path;
    }
    Serial.println(F("[EffectStorage] saveFile: complete"));
    return true;
}

bool EffectStorage::ensureLittleFs() {
    Serial.println(F("[EffectStorage] ensureLittleFs"));
    if (fsReady) {
        return true;
    }
    fsReady = LittleFS.begin(true);
    if (!fsReady) {
        Serial.println(F("[EffectStorage] LittleFS mount failed"));
    }
    return fsReady;
}

void EffectStorage::initializeTempIndex() {
    Serial.println(F("[EffectStorage] initializeTempIndex"));
    if (tempIndexInitialized) {
        return;
    }
    tempIndexInitialized = true;
    nextTempIndex = 0;

    if (!ensureLittleFs()) {
        return;
    }

    if (!LittleFS.exists("/tempEffects")) {
        LittleFS.mkdir("/tempEffects");
        return;
    }

    File dir = LittleFS.open("/tempEffects");
    if (!dir || !dir.isDirectory()) {
        if (dir) {
            dir.close();
        }
        Serial.println(F("/tempEffects is not a directory"));
        return;
    }

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            uint32_t index = 0;
            if (extractIndexFromName(String(file.name()), &index, nullptr)) {
                if (index >= nextTempIndex) {
                    nextTempIndex = index + 1;
                }
            }
        }
        file.close();
        file = dir.openNextFile();
    }
    dir.close();
}

bool EffectStorage::extractIndexFromName(const String& path, uint32_t* indexOut, String* baseOut) const {
    Serial.println(F("[EffectStorage] extractIndexFromName"));
    String name = path;
    int slash = name.lastIndexOf('/');
    if (slash >= 0) {
        name = name.substring(slash + 1);
    }

    int underscore = name.lastIndexOf('_');
    if (underscore <= 0 || underscore == name.length() - 1) {
        return false;
    }

    String indexStr = name.substring(underscore + 1);
    indexStr.trim();
    if (indexStr.isEmpty()) {
        return false;
    }

    for (size_t i = 0; i < indexStr.length(); ++i) {
        if (!isdigit(indexStr.charAt(i))) {
            return false;
        }
    }

    uint32_t parsedIndex = indexStr.toInt();
    if (parsedIndex == 0 && indexStr != "0") {
        return false;
    }

    if (indexOut) {
        *indexOut = parsedIndex;
    }
    if (baseOut) {
        *baseOut = name.substring(0, underscore);
    }
    return true;
}

bool EffectStorage::ensureDirectoryExists(const char* path) {
    Serial.println(F("[EffectStorage] ensureDirectoryExists"));
    if (!ensureLittleFs()) {
        return false;
    }
    if (LittleFS.exists(path)) {
        return true;
    }
    if (LittleFS.mkdir(path)) {
        return true;
    }
    Serial.println(F("[EffectStorage] ensureDirectoryExists: mkdir failed"));
    return false;
}

const char* EffectStorage::directoryForMode(SaveMode mode) const {
    Serial.println(F("[EffectStorage] directoryForMode"));
    switch (mode) {
        case SaveMode::Temp:
            return "/tempEffects";
        case SaveMode::Persistent:
            return "/effects";
        default:
            return "/";
    }
}

bool EffectStorage::findOldestTempFile(String* pathOut, uint32_t* indexOut) {
    Serial.println(F("[EffectStorage] findOldestTempFile"));
    if (!ensureLittleFs()) {
        return false;
    }

    File dir = LittleFS.open("/tempEffects");
    if (!dir || !dir.isDirectory()) {
        if (dir) {
            dir.close();
        }
        return false;
    }

    bool found = false;
    uint32_t lowestIndex = UINT32_MAX;
    String lowestPath;

    File file = dir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            uint32_t index = 0;
            if (extractIndexFromName(String(file.name()), &index, nullptr)) {
                if (!found || index < lowestIndex) {
                    found = true;
                    lowestIndex = index;
                    lowestPath = String(file.name());
                    if (!lowestPath.startsWith("/")) {
                        lowestPath = String("/tempEffects/") + lowestPath;
                    }
                }
            }
        }
        file.close();
        file = dir.openNextFile();
    }
    dir.close();

    if (!found) {
        return false;
    }

    if (pathOut) {
        *pathOut = lowestPath;
    }
    if (indexOut) {
        *indexOut = lowestIndex;
    }
    return true;
}

bool EffectStorage::deleteOldestTempFile(String* deletedPath) {
    Serial.println(F("[EffectStorage] deleteOldestTempFile"));
    initializeTempIndex();
    String path;
    if (!findOldestTempFile(&path, nullptr)) {
        return false;
    }

    if (!LittleFS.exists(path)) {
        return false;
    }

    if (!LittleFS.remove(path)) {
        return false;
    }

    if (deletedPath) {
        *deletedPath = path;
    }
    return true;
}

bool EffectStorage::ensureTempCapacity(uint16_t maxFiles) {
    Serial.println(F("[EffectStorage] ensureTempCapacity"));
    if (!ensureLittleFs()) {
        return false;
    }

    while (true) {
        File dir = LittleFS.open("/tempEffects");
        if (!dir || !dir.isDirectory()) {
            if (dir) {
                dir.close();
            }
            return false;
        }

        uint16_t count = 0;
        File file = dir.openNextFile();
        while (file) {
            if (!file.isDirectory()) {
                ++count;
            }
            file.close();
            file = dir.openNextFile();
        }
        dir.close();

        if (count < maxFiles) {
            return true;
        }

        if (!deleteOldestTempFile(nullptr)) {
            Serial.println(F("[EffectStorage] ensureTempCapacity: delete failed"));
            return false;
        }
    }
}

void EffectStorage::listEffects(Stream& out) {
    Serial.println(F("[EffectStorage] listEffects"));
    if (!ensureLittleFs()) {
        return;
    }

    auto listDir = [&](const char* path) {
        if (!LittleFS.exists(path)) {
            out.print(F("[EffectStorage] Directory missing: "));
            out.println(path);
            return;
        }
        File dir = LittleFS.open(path);
        if (!dir || !dir.isDirectory()) {
            out.print(F("[EffectStorage] Not a directory: "));
            out.println(path);
            if (dir) {
                dir.close();
            }
            return;
        }
        out.print(F("[EffectStorage] Listing "));
        out.println(path);
        File file = dir.openNextFile();
        while (file) {
            if (!file.isDirectory()) {
                out.print(F("  "));
                out.print(file.name());
                out.print(F("  Size: "));
                out.println(file.size());
            }
            file.close();
            file = dir.openNextFile();
        }
        dir.close();
    };

    listDir("/effects");
    listDir("/tempEffects");
}

bool EffectStorage::baseNameExists(const char* dir, const String& baseName) const {
    Serial.println(F("[EffectStorage] baseNameExists"));
    if (!LittleFS.exists(dir)) {
        return false;
    }

    File folder = LittleFS.open(dir);
    if (!folder || !folder.isDirectory()) {
        if (folder) {
            folder.close();
        }
        return false;
    }

    String prefix = baseName + "_";
    File file = folder.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String name = file.name();
            int slash = name.lastIndexOf('/');
            if (slash >= 0) {
                name = name.substring(slash + 1);
            }
            if (name == baseName || name.startsWith(prefix)) {
                file.close();
                folder.close();
                return true;
            }
        }
        file.close();
        file = folder.openNextFile();
    }
    folder.close();
    return false;
}

bool EffectStorage::findExistingFile(const String& name, size_t expectedSize, String* outPath) {
    Serial.println(F("[EffectStorage] findExistingFile"));
    if (!ensureLittleFs()) {
        Serial.println(F("[EffectStorage] findExistingFile: LittleFS not ready"));
        return false;
    }

    String sanitized = sanitizeFilename(name);
    if (sanitized.isEmpty()) {
        Serial.println(F("[EffectStorage] sanitized empty"));
        return false;
    }

    auto checkDir = [&](const char* dir) -> bool {
        String path = String(dir) + "/" + sanitized;
        File f = LittleFS.open(path, "r");
        if (!f) {
            return false;
        }
        bool match = (f.size() == expectedSize);
        f.close();
        if (match && outPath) {
            *outPath = path;
        }
        return match;
    };

    auto scanTempDir = [&]() -> bool {
        File dir = LittleFS.open("/tempEffects");
        if (!dir || !dir.isDirectory()) {
            if (dir) {
                dir.close();
            }
            return false;
        }

        bool found = false;
        File file = dir.openNextFile();
        while (file) {
            if (!file.isDirectory()) {
                String basePart;
                if (extractIndexFromName(String(file.name()), nullptr, &basePart) &&
                    basePart == sanitized && file.size() == expectedSize) {
                    String absolutePath = file.name();
                    if (!absolutePath.startsWith("/")) {
                        absolutePath = String("/tempEffects/") + absolutePath;
                    }
                    if (outPath) {
                        *outPath = absolutePath;
                    }
                    found = true;
                    file.close();
                    break;
                }
            }
            file.close();
            file = dir.openNextFile();
        }
        dir.close();
        return found;
    };

    if (checkDir("/effects")) {
        Serial.println(F("[EffectStorage] found in /effects"));
        return true;
    }

    if (checkDir("/tempEffects")) {
        Serial.println(F("[EffectStorage] found exact in /tempEffects"));
        return true;
    }

    bool foundTempNumbered = scanTempDir();
    if (foundTempNumbered) {
        Serial.println(F("[EffectStorage] found numbered in /tempEffects"));
    }
    return foundTempNumbered;
}
