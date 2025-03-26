#include "threading.h"

// building commands argument for thread function
void Threads::buildParams(const std::vector<std::string>& wbs, const std::string& path) {
	std::string confLine;
	for (const auto& name : wbs) {
		confLine = path + " " + name;
		this->threadCommands.emplace_back(confLine);
	}
}

void Threads::threadMsWorkerWithTimeout(const std::string& command, DWORD timeoutMs) {
    {
        std::unique_lock<std::mutex> lock(startMutex);
        startCV.wait(lock, [this]() { return readyToStart; });
    }

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    std::wstring wcommand(command.begin(), command.end());

    // Create the process
    if (!CreateProcessW(
        NULL,
        &wcommand[0],
        NULL,
        NULL,
        FALSE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        std::cerr << "CreateProcess failed for: " << command << std::endl;
        return;
    }

    // Wait for the process to finish or timeout
    DWORD result = WaitForSingleObject(pi.hProcess, timeoutMs);
    if (result == WAIT_TIMEOUT) {
        std::cerr << "Process timed out, terminating: " << command << std::endl;
        TerminateProcess(pi.hProcess, 1);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void Threads::runThreadPoolWithTimeout(DWORD timeoutMs) {
    for (auto& command : this->threadCommands) {
        this->threadPool.emplace_back([this, command, timeoutMs]() {
            this->threadMsWorkerWithTimeout(command, timeoutMs);
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // wait while preparing all threads

    {
        std::lock_guard<std::mutex> lock(startMutex);
        readyToStart = true;
    }
    startCV.notify_all();

    for (auto& t : this->threadPool) {
        if (t.joinable()) {
            t.join();
        }
    }

    {
        std::lock_guard<std::mutex> lock(startMutex);
        readyToStart = false;
    }
    this->threadPool.clear();
}

