#pragma once

class iDataStorage {
    public:
        virtual ~iDataStorage() = default;
        virtual void initialize() = 0;
        virtual bool isAvailable() = 0;
        virtual bool connect() = 0;
        virtual bool disconnect() = 0;
        virtual void insert(std::string_view query) = 0;
        virtual void start() = 0;
        virtual void stop() = 0;
};