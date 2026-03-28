#ifndef EFFECT_STORAGE_H
#define EFFECT_STORAGE_H

#include <Arduino.h>

class EffectStorage {
public:
    enum class SaveMode : uint8_t {
        None,
        Temp,
        Persistent
    };

    void begin();
    String sanitizeFilename(const String& name) const;
    String prepareFilename(const String& baseName, SaveMode mode);
    String getTempFilename(const String& baseName);
    String getFilename(const String& baseName);
    bool saveFile(const uint8_t* buffer, size_t size, const String& baseName,
                  const String& finalFilename, SaveMode mode, String* outPath = nullptr);
    bool deleteOldestTempFile(String* deletedPath = nullptr);
    void listEffects(Stream& out = Serial);
    bool findExistingFile(const String& name, size_t expectedSize, String* outPath = nullptr);

private:
    bool fsReady = false;
    uint32_t nextTempIndex = 0;
    bool tempIndexInitialized = false;

    bool ensureLittleFs();
    void initializeTempIndex();
    bool extractIndexFromName(const String& path, uint32_t* indexOut, String* baseOut = nullptr) const;
    bool ensureDirectoryExists(const char* path);
    const char* directoryForMode(SaveMode mode) const;
    bool findOldestTempFile(String* pathOut, uint32_t* indexOut);
    bool ensureTempCapacity(uint16_t maxFiles);
    bool baseNameExists(const char* dir, const String& baseName) const;
};

#endif

