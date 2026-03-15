#pragma once

#include <unordered_map>
#include <queue>
#include <mutex>
#include <memory>
#include <chrono>

template<typename KeyType>
class RequestTracker {

private:

    struct TrackingData {
        std::queue<int> timestamps;
        std::mutex mtx;
    };

    std::unordered_map<KeyType, std::shared_ptr<TrackingData>> dataMap;
    std::mutex mapMutex;
    int windowSeconds;

    int getCurrentTime()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
    }

public:

    RequestTracker(int window = 60) : windowSeconds(window) {}

    void recordRequest(KeyType key)
    {
        int time = getCurrentTime();

        std::unique_lock<std::mutex> mapLock(mapMutex);
        auto& entry = dataMap[key];

        if (!entry)
        {
            entry = std::make_shared<TrackingData>();
        }

        mapLock.unlock();

        std::lock_guard<std::mutex> dataLock(entry->mtx);
        entry->timestamps.push(time);
    }

    int getCount(KeyType key)
    {
        std::unique_lock<std::mutex> mapLock(mapMutex);

        auto it = dataMap.find(key);

        if (it == dataMap.end())
        {
            return 0;
        }

        auto entry = it->second;
        mapLock.unlock();

        int time = getCurrentTime();

        std::lock_guard<std::mutex> dataLock(entry->mtx);

        while (!entry->timestamps.empty() &&
               (time - entry->timestamps.front() > windowSeconds))
        {
            entry->timestamps.pop();
        }

        return entry->timestamps.size();
    }
};