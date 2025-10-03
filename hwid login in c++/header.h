#include <Windows.h>
#include <string>
#include <iostream>
#include "colors.h"
#include "xorstr.h"

#include <tchar.h>
#include <intrin.h>
#pragma once
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "wininet.lib")

#include <string>
#include <wininet.h>
using namespace std;


std::string getFirstHddSerialNumber2() {
	//get a handle to the first physical drive
	HANDLE h = CreateFileW(L"\\\\.\\PhysicalDrive0", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) return {};
	//an std::unique_ptr is used to perform cleanup automatically when returning (i.e. to avoid code duplication)
	std::unique_ptr<std::remove_pointer<HANDLE>::type, void(*)(HANDLE)> hDevice{ h, [](HANDLE handle) {CloseHandle(handle); } };
	//initialize a STORAGE_PROPERTY_QUERY data structure (to be used as input to DeviceIoControl)
	STORAGE_PROPERTY_QUERY storagePropertyQuery{};
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;
	//initialize a STORAGE_DESCRIPTOR_HEADER data structure (to be used as output from DeviceIoControl)
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader{};
	//the next call to DeviceIoControl retrieves necessary size (in order to allocate a suitable buffer)
	//call DeviceIoControl and return an empty std::string on failure
	DWORD dwBytesReturned = 0;
	if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER), &dwBytesReturned, NULL))
		return {};
	//allocate a suitable buffer
	const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
	std::unique_ptr<BYTE[]> pOutBuffer{ new BYTE[dwOutBufferSize]{} };
	//call DeviceIoControl with the allocated buffer
	if (!DeviceIoControl(hDevice.get(), IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pOutBuffer.get(), dwOutBufferSize, &dwBytesReturned, NULL))
		return {};
	//read and return the serial number out of the output buffer
	STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(pOutBuffer.get());
	const DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
	if (dwSerialNumberOffset == 0) return {};
	const char* serialNumber = reinterpret_cast<const char*>(pOutBuffer.get() + dwSerialNumberOffset);
	return serialNumber;
}

string a_replaceAll(string subject, const string& search,
	const string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

std::string a_DownloadURL(string URL) {
	HINTERNET interwebs = InternetOpenA("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	HINTERNET urlFile;
	string rtn;
	if (interwebs) {
		urlFile = InternetOpenUrlA(interwebs, URL.c_str(), NULL, NULL, NULL, NULL);
		if (urlFile) {
			char buffer[2000];
			DWORD bytesRead;
			do {
				InternetReadFile(urlFile, buffer, 2000, &bytesRead);
				rtn.append(buffer, bytesRead);
				memset(buffer, 0, 2000);
			} while (bytesRead);
			InternetCloseHandle(interwebs);
			InternetCloseHandle(urlFile);
			string p = a_replaceAll(rtn, "|n", "\r\n");
			return p;
		}
	}
	InternetCloseHandle(interwebs);
	string p = a_replaceAll(rtn, "|n", "\r\n");
	return p;
}
#include <sstream>

bool dologin()
{


	std::string serialNumber =  getFirstHddSerialNumber2();
	
	if (serialNumber.empty())
	{
		exit(0);
	}
	std::string str = a_DownloadURL("https://raw.githubusercontent.com/KaiOpg1/Hwid-Login-c-/refs/heads/main/hwid-test.txt");
	string str2(serialNumber);
	

	if (str.find(str2) != string::npos) {

		std::cout << green << xorstr_("[+] Welcome") << std::endl;


	}
	else {
		std::cout << red << xorstr_("[-] Wrong HWID") << std::endl;
		Sleep(15000);
		exit(0);

	}
	return true;
}