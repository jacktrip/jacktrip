#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

// Structure to hold JackTrip product information
struct JackTripProduct {
    std::string productCode;
    std::string displayName;
    std::string publisher;
};

// Function to get current timestamp for logging
std::string GetTimestamp() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << st.wYear << "-"
        << std::setfill('0') << std::setw(2) << st.wMonth << "-"
        << std::setfill('0') << std::setw(2) << st.wDay << " "
        << std::setfill('0') << std::setw(2) << st.wHour << ":"
        << std::setfill('0') << std::setw(2) << st.wMinute << ":"
        << std::setfill('0') << std::setw(2) << st.wSecond;
    return oss.str();
}

// Function to log messages with timestamp
void LogMessage(const std::string& message) {
    std::cout << "[" << GetTimestamp() << "] " << message << std::endl;
}

// Function to check if a registry key exists
bool RegKeyExists(HKEY hKey, const std::string& subKey) {
    HKEY hSubKey;
    LONG result = RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey);
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hSubKey);
        return true;
    }
    return false;
}

// Function to get a DWORD value from registry
bool GetRegDWORD(HKEY hKey, const std::string& subKey, const std::string& valueName, DWORD& value) {
    HKEY hSubKey;
    LONG result = RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey);
    if (result != ERROR_SUCCESS) {
        return false;
    }

    DWORD dataSize = sizeof(DWORD);
    DWORD dataType = REG_DWORD;
    result = RegQueryValueExA(hSubKey, valueName.c_str(), NULL, &dataType, (LPBYTE)&value, &dataSize);
    RegCloseKey(hSubKey);

    return (result == ERROR_SUCCESS && dataType == REG_DWORD);
}

// Function to get a string value from registry
bool GetRegString(HKEY hKey, const std::string& subKey, const std::string& valueName, std::string& value) {
    HKEY hSubKey;
    LONG result = RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey);
    if (result != ERROR_SUCCESS) {
        return false;
    }

    DWORD dataSize = 0;
    DWORD dataType = REG_SZ;
    result = RegQueryValueExA(hSubKey, valueName.c_str(), NULL, &dataType, NULL, &dataSize);
    if (result != ERROR_SUCCESS) {
        RegCloseKey(hSubKey);
        return false;
    }

    std::vector<char> buffer(dataSize);
    result = RegQueryValueExA(hSubKey, valueName.c_str(), NULL, &dataType, (LPBYTE)buffer.data(), &dataSize);
    RegCloseKey(hSubKey);

    if (result == ERROR_SUCCESS && dataType == REG_SZ) {
        value = std::string(buffer.data());
        return true;
    }

    return false;
}

// Function to enumerate registry subkeys
std::vector<std::string> EnumRegSubKeys(HKEY hKey, const std::string& subKey) {
    std::vector<std::string> subKeys;
    HKEY hSubKey;

    LONG result = RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_ENUMERATE_SUB_KEYS, &hSubKey);
    if (result != ERROR_SUCCESS) {
        return subKeys;
    }

    char keyName[256];
    DWORD keyNameSize = sizeof(keyName);
    DWORD index = 0;

    while (RegEnumKeyExA(hSubKey, index, keyName, &keyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
        subKeys.push_back(std::string(keyName));
        keyNameSize = sizeof(keyName);
        index++;
    }

    RegCloseKey(hSubKey);
    return subKeys;
}

// Function to test if JackTrip version is old (2.7.0 or earlier)
bool TestJackTripVersion(const std::string& productCode) {
    std::string regPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + productCode;

    // Check if this is a Windows Installer (MSI) installation
    DWORD windowsInstaller = 0;
    if (!GetRegDWORD(HKEY_LOCAL_MACHINE, regPath, "WindowsInstaller", windowsInstaller) || windowsInstaller != 1) {
        LogMessage("Product " + productCode + " is not a Windows Installer (MSI) installation");
        return false;
    }

    // Get Version DWORD value
    DWORD versionDword = 0;
    if (!GetRegDWORD(HKEY_LOCAL_MACHINE, regPath, "Version", versionDword)) {
        LogMessage("No Version DWORD found for product " + productCode);
        return false;
    }

    LogMessage("Found Version DWORD: " + std::to_string(versionDword) + " for product " + productCode);

    // Extract version components from DWORD
    // First byte = major, second byte = minor, third byte = build, fourth byte = patch
    DWORD major = (versionDword & 0xFF000000) >> 24;
    DWORD minor = (versionDword & 0x00FF0000) >> 16;
    DWORD build = (versionDword & 0x0000FF00) >> 8;
    DWORD patch = (versionDword & 0x000000FF);

    LogMessage("Parsed version as major=" + std::to_string(major) +
               ", minor=" + std::to_string(minor) +
               ", patch=" + std::to_string(patch));

    // Check if version is 2.7.0 or earlier
    if (major < 2 || (major == 2 && minor < 7) || (major == 2 && minor == 7 && patch == 0)) {
        return true;
    }

    return false;
}

// Function to uninstall JackTrip product using msiexec
bool UninstallJackTripProduct(const std::string& productCode) {
    LogMessage("Attempting to uninstall JackTrip product: " + productCode);

    // Build msiexec command
    std::string command = "msiexec.exe /x \"" + productCode + "\" /quiet /norestart";
    LogMessage("Executing: " + command);

    // Execute the command
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    // Convert command to char array for CreateProcessA
    std::vector<char> cmdBuffer(command.begin(), command.end());
    cmdBuffer.push_back('\0');

    BOOL result = CreateProcessA(
        NULL,                   // No module name (use command line)
        cmdBuffer.data(),       // Command line
        NULL,                   // Process handle not inheritable
        NULL,                   // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        0,                      // No creation flags
        NULL,                   // Use parent's environment block
        NULL,                   // Use parent's starting directory
        &si,                    // Pointer to STARTUPINFO structure
        &pi                     // Pointer to PROCESS_INFORMATION structure
    );

    if (!result) {
        LogMessage("Failed to start uninstall process for product " + productCode);
        return false;
    }

    // Wait for the process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Get exit code
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    // Clean up
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exitCode == 0) {
        LogMessage("Successfully uninstalled JackTrip product: " + productCode);
        return true;
    } else {
        LogMessage("Uninstall failed for product " + productCode + " with exit code: " + std::to_string(exitCode));
        return false;
    }
}

int main() {
    LogMessage("Starting JackTrip old version detection and uninstall process");

    // Query registry for JackTrip installations
    std::vector<JackTripProduct> jacktripProducts;
    std::vector<std::string> subKeys = EnumRegSubKeys(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");

    for (const auto& subKey : subKeys) {
        std::string regPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\" + subKey;

        std::string displayName, publisher;
        if (GetRegString(HKEY_LOCAL_MACHINE, regPath, "DisplayName", displayName) &&
            GetRegString(HKEY_LOCAL_MACHINE, regPath, "Publisher", publisher)) {

            if (displayName == "JackTrip" && publisher == "JackTrip") {
                JackTripProduct product;
                product.productCode = subKey;
                product.displayName = displayName;
                product.publisher = publisher;
                jacktripProducts.push_back(product);
                LogMessage("Found JackTrip installation: " + subKey);
            }
        }
    }

    if (jacktripProducts.empty()) {
        LogMessage("No JackTrip installations found in registry");
        return 0;
    }

    LogMessage("Found " + std::to_string(jacktripProducts.size()) + " JackTrip installation(s)");

    // Check each product for old version and uninstall if needed
    int uninstalledCount = 0;

    for (const auto& product : jacktripProducts) {
        if (TestJackTripVersion(product.productCode)) {
            LogMessage("Old JackTrip version detected: " + product.productCode);
            if (UninstallJackTripProduct(product.productCode)) {
                uninstalledCount++;
            }
        } else {
            LogMessage("JackTrip installation " + product.productCode + " is not an old version (v2.7.0 or earlier) or is not an MSI installation");
        }
    }

    LogMessage("Uninstall process completed. Uninstalled " + std::to_string(uninstalledCount) + " old JackTrip installation(s)");
    return 0;
}