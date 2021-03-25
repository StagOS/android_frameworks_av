/*
 * Copyright 2021, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "CodecProperties"
#include <utils/Log.h>

#include <string>

#include <media/formatshaper/CodecProperties.h>

namespace android {
namespace mediaformatshaper {

CodecProperties::CodecProperties(std::string name, std::string mediaType) {
    mName = name;
    mMediaType = mediaType;
}

std::string CodecProperties::getName(){
    return mName;
}

std::string CodecProperties::getMediaType(){
    return mMediaType;
}

int CodecProperties::supportedMinimumQuality() {
    return mMinimumQuality;
}
void CodecProperties::setSupportedMinimumQuality(int vmaf) {
    mMinimumQuality = vmaf;
}

int CodecProperties::targetQpMax() {
    return mTargetQpMax;
}
void CodecProperties::setTargetQpMax(int qpMax) {
    mTargetQpMax = qpMax;
}

// what API is this codec set up for (e.g. API of the associated partition)
// vendor-side (OEM) codecs may be older, due to 'vendor freeze' and treble
int CodecProperties::supportedApi() {
    return mApi;
}

std::string CodecProperties::getMapping(std::string key, std::string kind) {
    ALOGV("getMapping(key %s, kind %s )", key.c_str(), kind.c_str());
    //play with mMappings
    auto mapped = mMappings.find(kind + "-" + key);
    if (mapped != mMappings.end()) {
        std::string result = mapped->second;
        ALOGV("getMapping(%s, %s) -> %s", key.c_str(), kind.c_str(), result.c_str());
        return result;
    }
    ALOGV("nope, return unchanged key");
    return key;
}


// really a bit of debugging code here.
void CodecProperties::showMappings() {
    ALOGD("Mappings:");
    int count = 0;
    for (const auto& [key, value] : mMappings) {
         count++;
         ALOGD("'%s' -> '%s'", key.c_str(), value.c_str());
    }
    ALOGD("total %d mappings", count);
}

void CodecProperties::setMapping(std::string kind, std::string key, std::string value) {
    ALOGV("setMapping(%s,%s,%s)", kind.c_str(), key.c_str(), value.c_str());
    std::string metaKey = kind + "-" + key;
    mMappings.insert({metaKey, value});
}

const char **CodecProperties::getMappings(std::string kind, bool reverse) {
    ALOGV("getMappings(kind %s, reverse %d", kind.c_str(), reverse);
    // how many do we need?
    int count = mMappings.size();
    if (count == 0) {
        ALOGV("empty mappings");
        return nullptr;
    }
    size_t size = sizeof(char *) * (2 * count + 2);
    const char **result = (const char **)malloc(size);
    if (result == nullptr) {
        ALOGW("no memory to return mappings");
        return nullptr;
    }
    memset(result, '\0', size);

    const char **pp = result;
    for (const auto& [key, value] : mMappings) {
        // split out the kind/key
        size_t pos = key.find('-');
        if (pos == std::string::npos) {
            ALOGD("ignoring malformed key: %s", key.c_str());
            continue;
        }
        std::string actualKind = key.substr(0,pos);
        if (kind.length() != 0 && kind != actualKind) {
            ALOGD("kinds don't match: want '%s' got '%s'", kind.c_str(), actualKind.c_str());
            continue;
        }
        if (reverse) {
            // codec specific -> std aka 'unmapping'
            pp[0] = strdup( value.c_str());
            pp[1] = strdup( key.substr(pos+1).c_str());
        } else {
            // std -> codec specific
            pp[0] = strdup( key.substr(pos+1).c_str());
            pp[1] = strdup( value.c_str());
        }
        ALOGV(" %s -> %s", pp[0], pp[1]);
        pp += 2;
    }

    pp[0] = nullptr;
    pp[1] = nullptr;

    return result;
}


} // namespace mediaformatshaper
} // namespace android
