/////////////////////////////////  Includes  //////////////////////////////////

#include "pch.h"
#include "enumser.h"


/////////////////////////////// Macros / Defines //////////////////////////////

#if !defined(NO_CENUMERATESERIAL_USING_SETUPAPI1) || !defined(NO_CENUMERATESERIAL_USING_SETUPAPI2)
  #pragma comment(lib, "setupapi.lib")
#endif //#if !defined(NO_CENUMERATESERIAL_USING_SETUPAPI1) || !defined(NO_CENUMERATESERIAL_USING_SETUPAPI2)

#ifndef NO_CENUMERATESERIAL_USING_ENUMPORTS
  #ifndef _WINSPOOL_
    #include <winspool.h>
  #endif //#ifndef _WINSPOOL_

  #pragma comment(lib, "winspool.lib")
#endif //#ifndef NO_CENUMERATESERIAL_USING_ENUMPORTS

#if !defined(NO_CENUMERATESERIAL_USING_SETUPAPI1) || !defined(NO_CENUMERATESERIAL_USING_SETUPAPI2) || !defined(NO_CENUMERATESERIAL_USING_REGISTRY)
  #pragma comment(lib, "advapi32.lib")
#endif //#if !defined(NO_CENUMERATESERIAL_USING_SETUPAPI1) || !defined(NO_CENUMERATESERIAL_USING_SETUPAPI2) || !defined(NO_CENUMERATESERIAL_USING_REGISTRY)

#ifndef NO_CENUMERATESERIAL_USING_WMI
  #ifndef __IWbemLocator_FWD_DEFINED__
    #include <WbemCli.h>
  #endif //#ifndef __IWbemLocator_FWD_DEFINED__

#ifndef _INC_COMDEF
    #include <comdef.h>
  #endif //#ifndef _INC_COMDEF

  #ifndef _ARRAY_
    #include <array>
  #endif //#ifndef _ARRAY_

  #pragma comment(lib, "WbemUuid.lib")
#endif //#ifndef NO_CENUMERATESERIAL_USING_WMI

#ifndef NO_CENUMERATESERIAL_USING_COMDB
  #ifndef _MSPORTS_H
    #include <msports.h>
  #endif //#ifndef _MSPORTS_H

  #pragma comment(lib, "msports.lib")
#endif //#ifndef NO_CENUMERATESERIAL_USING_COMDB


///////////////////////////// Implementation //////////////////////////////////

#ifndef NO_CENUMERATESERIAL_USING_CREATEFILE
_Return_type_success_(return != false) bool CEnumerateSerial::UsingCreateFile(_Inout_ CPortsArray& ports)
{
  //Reset the output parameter
  ports.clear();

  //Up to 255 COM ports are supported so we iterate through all of them seeing
  //if we can open them or if we fail to open them, get an access denied or general error error.
  //Both of these cases indicate that there is a COM port at that number. 
  for (UINT i=1; i<256; i++)
  {
    //Form the Raw device name
    ATL::CAtlString sPort;
    sPort.Format(_T("\\\\.\\COM%u"), i);

    //Try to open the port
    bool bSuccess = false;
    ATL::CHandle port(CreateFile(sPort, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr));
    if (port == INVALID_HANDLE_VALUE)
    {
      const DWORD dwError = GetLastError();

      //Check to see if the error was because some other app had the port open or a general failure
      if ((dwError == ERROR_ACCESS_DENIED) || (dwError == ERROR_GEN_FAILURE) || (dwError == ERROR_SHARING_VIOLATION) || (dwError == ERROR_SEM_TIMEOUT))
        bSuccess = true;
    }
    else
    {
      //The port was opened successfully
      bSuccess = true;
    }

    //Add the port number to the array which will be returned
    if (bSuccess)
#pragma warning(suppress: 26489)
      ports.push_back(i);
  }

  //Return the success indicator
  return true;
}
#endif //#ifndef NO_CENUMERATESERIAL_USING_CREATEFILE

#if !defined(NO_CENUMERATESERIAL_USING_SETUPAPI1) || !defined(NO_CENUMERATESERIAL_USING_SETUPAPI2)
#pragma warning(suppress: 26429)
_Return_type_success_(return != false) bool CEnumerateSerial::RegQueryValueString(_In_ ATL::CRegKey& key, _In_ LPCTSTR lpValueName, _Inout_ String& sValue)
{
  //Reset the output parameter
  sValue.clear();

  //First query for the size of the registry value
  ULONG nChars = 0;
  LSTATUS nStatus = key.QueryStringValue(lpValueName, nullptr, &nChars);
  if (nStatus != ERROR_SUCCESS)
  {
    SetLastError(nStatus);
    return false;
  }

  //Allocate enough bytes for the return value
#pragma warning(suppress: 26472 26489)
  sValue.resize(static_cast<size_t>(nChars) + 1); //+1 is to allow us to null terminate the data if required
  const DWORD dwAllocatedSize = ((nChars + 1)*sizeof(TCHAR));

  //We will use RegQueryValueEx directly here because ATL::CRegKey::QueryStringValue does not handle non-null terminated data
  DWORD dwType = 0;
  ULONG nBytes = dwAllocatedSize;
#pragma warning(suppress: 26446 26489 26490)
  nStatus = RegQueryValueEx(key, lpValueName, nullptr, &dwType, reinterpret_cast<LPBYTE>(&(sValue[0])), &nBytes);
  if (nStatus != ERROR_SUCCESS)
  {
    SetLastError(nStatus);
    return false;
  }
  if ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ))
  {
    SetLastError(ERROR_INVALID_DATA);
    return false;
  }
  if ((nBytes % sizeof(TCHAR)) != 0)
  {
    SetLastError(ERROR_INVALID_DATA);
    return false;
  }
#pragma warning(suppress: 26446 26489)
  if (sValue[(nBytes / sizeof(TCHAR)) - 1] != _T('\0'))
  {
    //Forcibly null terminate the data ourselves
#pragma warning(suppress: 26446 26489)
    sValue[(nBytes / sizeof(TCHAR))] = _T('\0');
  }

  return true;
}

_Return_type_success_(return != false) bool CEnumerateSerial::QueryRegistryPortName(_In_ ATL::CRegKey& deviceKey, _Out_ int& nPort)
{
  //What will be the return value from the method (assume the worst)
  bool bAdded = false;

  //Read in the name of the port
  String sPortName;
  if (RegQueryValueString(deviceKey, _T("PortName"), sPortName))
  {
    //If it looks like "COMX" then
    //add it to the array which will be returned
    const size_t nLen = sPortName.length();
    if (nLen > 3)
    {
#pragma warning(suppress: 26481)
      if ((_tcsnicmp(sPortName.c_str(), _T("COM"), 3) == 0) && IsNumeric((sPortName.c_str() + 3), false))
      {
        //Work out the port number
#pragma warning(suppress: 26481)
        nPort = _ttoi(sPortName.c_str() + 3);
        bAdded = true;
      }
    }
  }

  return bAdded;
}

_Return_type_success_(return != false) bool CEnumerateSerial::QueryUsingSetupAPI(const GUID& guid, _In_ DWORD dwFlags, _Inout_ CPortAndNamesArray& ports)
{
  //Reset the output parameter
  ports.clear();

  //Create a "device information set" for the specified GUID
  HDEVINFO hDevInfoSet = SetupDiGetClassDevs(&guid, nullptr, nullptr, dwFlags);
  if (hDevInfoSet == INVALID_HANDLE_VALUE)
    return false;

  //Finally do the enumeration
  bool bMoreItems = true;
  int nIndex = 0;
  SP_DEVINFO_DATA devInfo{};
  while (bMoreItems)
  {
    //Enumerate the current device
    devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
    bMoreItems = SetupDiEnumDeviceInfo(hDevInfoSet, nIndex, &devInfo);
    if (bMoreItems)
    {
      //Did we find a serial port for this device
      bool bAdded = false;

      std::pair<UINT, String> pair;

      //Get the registry key which stores the ports settings
      ATL::CRegKey deviceKey;
      deviceKey.Attach(SetupDiOpenDevRegKey(hDevInfoSet, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE));
      if (deviceKey != INVALID_HANDLE_VALUE)
      {
        int nPort = 0;
#pragma warning(suppress: 26486)
        if (QueryRegistryPortName(deviceKey, nPort))
        {
          pair.first = nPort;
          bAdded = true;
        }
      }

      //If the port was a serial port, then also try to get its friendly name
      if (bAdded)
      {
#pragma warning(suppress: 26489)
        if (QueryDeviceDescription(hDevInfoSet, devInfo, pair.second))
#pragma warning(suppress: 26489)
          ports.push_back(pair);
      }
    }

    ++nIndex;
  }

  //Free up the "device information set" now that we are finished with it
  SetupDiDestroyDeviceInfoList(hDevInfoSet);

  //Return the success indicator
  return true;
}

_Return_type_success_(return != false) bool CEnumerateSerial::QueryDeviceDescription(_In_ HDEVINFO hDevInfoSet, _In_ SP_DEVINFO_DATA& devInfo, _Inout_ String& sFriendlyName)
{
  DWORD dwType = 0;
  DWORD dwSize = 0;
  //Query initially to get the buffer size required
  if (!SetupDiGetDeviceRegistryProperty(hDevInfoSet, &devInfo, SPDRP_DEVICEDESC, &dwType, nullptr, 0, &dwSize))
  {
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
      return false;
  }
  sFriendlyName.resize(dwSize / sizeof(TCHAR));
#pragma warning(suppress: 26446 26490)
  if (!SetupDiGetDeviceRegistryProperty(hDevInfoSet, &devInfo, SPDRP_DEVICEDESC, &dwType, reinterpret_cast<PBYTE>(&(sFriendlyName[0])), dwSize, &dwSize))
    return false;
  if (dwType != REG_SZ)
  {
    SetLastError(ERROR_INVALID_DATA);
    return false;
  }
  return true;
}
#endif //#if !defined(NO_CENUMERATESERIAL_USING_SETUPAPI1) || !defined(NO_CENUMERATESERIAL_USING_SETUPAPI2)

#pragma warning(suppress: 26429)
_Return_type_success_(return != false) bool CEnumerateSerial::IsNumeric(_In_z_ LPCSTR pszString, _In_ bool bIgnoreColon) noexcept
{
  const size_t nLen = strlen(pszString);
  if (nLen == 0)
    return false;

  //What will be the return value from this function (assume the best)
  bool bNumeric = true;

  for (size_t i=0; i<nLen && bNumeric; i++)
  {
#pragma warning(suppress: 26481)
    if (bIgnoreColon && (pszString[i] == ':'))
      bNumeric = true;
    else
#pragma warning(suppress: 26472 26481)
      bNumeric = (isdigit(static_cast<int>(pszString[i])) != 0);
  }

  return bNumeric;
}

#pragma warning(suppress: 26429)
_Return_type_success_(return != false) bool CEnumerateSerial::IsNumeric(_In_z_ LPCWSTR pszString, _In_ bool bIgnoreColon) noexcept
{
  const size_t nLen = wcslen(pszString);
  if (nLen == 0)
    return false;

  //What will be the return value from this function (assume the best)
  bool bNumeric = true;

  for (size_t i=0; i<nLen && bNumeric; i++)
  {
#pragma warning(suppress: 26481)
    if (bIgnoreColon && (pszString[i] == L':'))
      bNumeric = true;
    else
#pragma warning(suppress: 26481)
       bNumeric = (iswdigit(pszString[i]) != 0);
  }

  return bNumeric;
}

#ifndef NO_CENUMERATESERIAL_USING_QUERYDOSDEVICE
_Return_type_success_(return != false) bool CEnumerateSerial::UsingQueryDosDevice(_Inout_ CPortsArray& ports)
{
  //Reset the output parameter
  ports.clear();

  //Use QueryDosDevice to look for all devices of the form COMx. Since QueryDosDevice does
  //not consistently report the required size of buffer, lets start with a reasonable buffer size
  //of 4096 characters and go from there
  int nChars = 4096;
  bool bWantStop = false;
  while (nChars && !bWantStop)
  {
    std::vector<TCHAR> devices;
    devices.resize(nChars);

#pragma warning(suppress: 26446)
    const DWORD dwChars = QueryDosDevice(nullptr, &(devices[0]), nChars);
    if (dwChars == 0)
    {
      const DWORD dwError = GetLastError();
      if (dwError == ERROR_INSUFFICIENT_BUFFER)
      {
        //Expand the buffer and  loop around again
        nChars *= 2;
      }
      else
        return false;
    }
    else
    {
      bWantStop = true;

      size_t i = 0;
      #pragma warning(suppress: 6385 26446)
      while (devices[i] != _T('\0'))
      {
        //Get the current device name
#pragma warning(suppress: 26429 26446)
        LPCTSTR pszCurrentDevice = &(devices[i]);

        //If it looks like "COMX" then
        //add it to the array which will be returned
        const size_t nLen = _tcslen(pszCurrentDevice);
        if (nLen > 3)
        {
#pragma warning(suppress: 26481)
          if ((_tcsnicmp(pszCurrentDevice, _T("COM"), 3) == 0) && IsNumeric(&(pszCurrentDevice[3]), false))
          {
            //Work out the port number
#pragma warning(suppress: 26481)
            const int nPort = _ttoi(&pszCurrentDevice[3]);
#pragma warning(suppress: 26489)
            ports.push_back(nPort);
          }
        }

        //Go to next device name
        i += (nLen + 1);
      }
    }
  }

  return true;
}
#endif //#ifndef NO_CENUMERATESERIAL_USING_QUERYDOSDEVICE

#ifndef NO_CENUMERATESERIAL_USING_GETDEFAULTCOMMCONFIG
_Return_type_success_(return != false) bool CEnumerateSerial::UsingGetDefaultCommConfig(_Inout_ CPortsArray& ports)
{
  //Reset the output parameter
  ports.clear();

  //Up to 255 COM ports are supported so we iterate through all of them seeing
  //if we can get the default configuration
  for (UINT i=1; i<256; i++)
  {
    //Form the Raw device name
    ATL::CAtlString sPort;
    sPort.Format(_T("COM%u"), i);

    COMMCONFIG cc{};
    DWORD dwSize = sizeof(COMMCONFIG);
    if (GetDefaultCommConfig(sPort, &cc, &dwSize))
#pragma warning(suppress: 26489)
      ports.push_back(i);
  }

  //Return the success indicator
  return true;
}
#endif //#ifndef NO_CENUMERATESERIAL_USING_GETDEFAULTCOMMCONFIG

#ifndef NO_CENUMERATESERIAL_USING_SETUPAPI1
_Return_type_success_(return != false) bool CEnumerateSerial::UsingSetupAPI1(_Inout_ CPortAndNamesArray& ports)
{
  //Delegate the main work of this method to the helper method
  return QueryUsingSetupAPI(GUID_DEVINTERFACE_COMPORT, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE, ports);
}
#endif //#ifndef NO_CENUMERATESERIAL_USING_SETUPAPI1

#ifndef NO_CENUMERATESERIAL_USING_SETUPAPI2
_Return_type_success_(return != false) bool CEnumerateSerial::UsingSetupAPI2(_Inout_ CPortAndNamesArray& ports)
{
  //Delegate the main work of this method to the helper method
  return QueryUsingSetupAPI(GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR, DIGCF_PRESENT, ports);
}
#endif //#ifndef NO_CENUMERATESERIAL_USING_SETUPAPI2

#ifndef NO_CENUMERATESERIAL_USING_ENUMPORTS
_Return_type_success_(return != false) bool CEnumerateSerial::UsingEnumPorts(_Inout_ CPortAndNamesArray& ports)
{
  //Reset the output parameter
  ports.clear();

  //Call the first time to determine the size of the buffer to allocate
  DWORD cbNeeded = 0;
  DWORD dwPorts = 0;
  if (!EnumPorts(nullptr, 2, nullptr, 0, &cbNeeded, &dwPorts))
  {
    const DWORD dwError = GetLastError();
    if (dwError != ERROR_INSUFFICIENT_BUFFER)
      return false;
  }

  //What will be the return value
  bool bSuccess = false;

  //Allocate the buffer and recall
  std::vector<BYTE> portsBuffer;
  portsBuffer.resize(cbNeeded);
#pragma warning(suppress: 26446)
  bSuccess = EnumPorts(nullptr, 2, &(portsBuffer[0]), cbNeeded, &cbNeeded, &dwPorts);
  if (bSuccess)
  {
#pragma warning(suppress: 26429 26490)
    auto pPortInfo = reinterpret_cast<const PORT_INFO_2*>(portsBuffer.data());
    for (DWORD i=0; i<dwPorts; i++)
    {
      //If it looks like "COMX" then
      //add it to the array which will be returned
#pragma warning(suppress: 26486 26489)
      const size_t nLen = _tcslen(pPortInfo->pPortName);
      if (nLen > 3)
      {
#pragma warning(suppress: 26481 26486 26489)
        if ((_tcsnicmp(pPortInfo->pPortName, _T("COM"), 3) == 0) && IsNumeric(&(pPortInfo->pPortName[3]), true))
        {
          //Work out the port number
#pragma warning(suppress: 26481 26489)
          const int nPort = _ttoi(&(pPortInfo->pPortName[3]));
          std::pair<UINT, String> pair;
          pair.first = nPort;
#pragma warning(suppress: 26486 26489)
          pair.second = pPortInfo->pDescription;
#pragma warning(suppress: 26489)
          ports.push_back(pair);
        }
      }

#pragma warning(suppress: 26481)
      pPortInfo++;
    }
  }

  return bSuccess;
}
#endif //#ifndef NO_CENUMERATESERIAL_USING_ENUMPORTS

#ifndef NO_CENUMERATESERIAL_USING_WMI
HRESULT CEnumerateSerial::UsingWMI(_Inout_ CPortAndNamesArray& ports)
{
  //Reset the output parameter
  ports.clear();

  //Create the WBEM locator
  ATL::CComPtr<IWbemLocator> pLocator;
#pragma warning(suppress: 26490)
  HRESULT hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void**>(&pLocator));
  if (FAILED(hr))
    return hr;

  ATL::CComPtr<IWbemServices> pServices;
  hr = pLocator->ConnectServer(_bstr_t(R"(\\.\root\cimv2)"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &pServices);
  if (FAILED(hr))
    return hr;

  //Execute the query
  ATL::CComPtr<IEnumWbemClassObject> pClassObject;
  hr = pServices->CreateInstanceEnum(_bstr_t("Win32_SerialPort"), WBEM_FLAG_RETURN_WBEM_COMPLETE, nullptr, &pClassObject);
  if (FAILED(hr))
    return hr;

  //Now enumerate all the ports
  hr = WBEM_S_NO_ERROR;

  //The final Next will return WBEM_S_FALSE
  while (hr == WBEM_S_NO_ERROR)
  {
    ULONG uReturned = 0;
    std::array<ATL::CComPtr<IWbemClassObject>, 10> apObj;
#pragma warning(suppress: 26490)
    hr = pClassObject->Next(WBEM_INFINITE, 10, reinterpret_cast<IWbemClassObject**>(apObj.data()), &uReturned);
    if (SUCCEEDED(hr))
    {
      for (ULONG n=0; n<uReturned; n++)
      {
        ATL::CComVariant varProperty1;
#pragma warning(suppress: 26446 26482)
        const HRESULT hrGet = apObj[n]->Get(L"DeviceID", 0, &varProperty1, nullptr, nullptr);
#pragma warning(suppress: 26486)
        if (SUCCEEDED(hrGet) && (varProperty1.vt == VT_BSTR) && (wcslen(varProperty1.bstrVal) > 3))
        {
          //If it looks like "COMX" then add it to the array which will be returned
#pragma warning(suppress: 26481 26486 26489)
          if ((_wcsnicmp(varProperty1.bstrVal, L"COM", 3) == 0) && IsNumeric(&(varProperty1.bstrVal[3]), true))
          {
            //Work out the port number
#pragma warning(suppress: 26481 26489)
            const int nPort = _wtoi(&(varProperty1.bstrVal[3]));

            std::pair<UINT, String> pair;
            pair.first = nPort;

            //Also get the friendly name of the port
            ATL::CComVariant varProperty2;
#pragma warning(suppress: 26446 26482)
            if (SUCCEEDED(apObj[n]->Get(L"Name", 0, &varProperty2, nullptr, nullptr)) && (varProperty2.vt == VT_BSTR))
            {
            #ifdef _UNICODE
#pragma warning(suppress: 26486)
              std::wstring szName(varProperty2.bstrVal);
            #else
#pragma warning(suppress: 26486)
              std::string szName(ATL::CW2A(varProperty2.bstrVal));
            #endif //#ifdef _UNICODE
#pragma warning(suppress: 26489)
              pair.second = szName;
            }

#pragma warning(suppress: 26486 26489)
            ports.push_back(pair);
          }
        }
      }
    }
  }

  return S_OK;
}
#endif //#ifndef NO_CENUMERATESERIAL_USING_WMI

#ifndef NO_CENUMERATESERIAL_USING_COMDB
_Return_type_success_(return != false) bool CEnumerateSerial::UsingComDB(_Inout_ CPortsArray& ports)
{
  //Reset the output parameter
  ports.clear();

  //First need to open up the DB
  HCOMDB hComDB = nullptr;
  LONG nSuccess = ComDBOpen(&hComDB);
  if (nSuccess != ERROR_SUCCESS)
  {
    SetLastError(nSuccess);
    return false;
  }

  //Work out the size of the buffer required
  DWORD dwMaxPortsReported = 0;
  nSuccess = ComDBGetCurrentPortUsage(hComDB, nullptr, 0, CDB_REPORT_BYTES, &dwMaxPortsReported);
  if (nSuccess != ERROR_SUCCESS)
  {
    ComDBClose(hComDB);
    SetLastError(nSuccess);
    return false;
  }

  //Allocate some space and recall the function
  std::vector<BYTE> portBytes;
  portBytes.resize(dwMaxPortsReported);
#pragma warning(suppress: 26446)
  const LONG nStatus = ComDBGetCurrentPortUsage(hComDB, &(portBytes[0]), dwMaxPortsReported, CDB_REPORT_BYTES, &dwMaxPortsReported);
  if (nStatus != ERROR_SUCCESS)
  {
    ComDBClose(hComDB);
    SetLastError(nStatus);
    return false;
  }

  //Work thro the byte bit array for ports which are in use
  for (DWORD i=0; i<dwMaxPortsReported; i++)
  {
#pragma warning(suppress: 26446)
    if (portBytes[i])
#pragma warning(suppress: 26489)
      ports.push_back(i + 1);
  }

  //Close the DB
  ComDBClose(hComDB);

  return true;
}
#endif //#ifndef NO_CENUMERATESERIAL_USING_COMDB

#ifndef NO_CENUMERATESERIAL_USING_REGISTRY
_Return_type_success_(return != false) bool CEnumerateSerial::UsingRegistry(_Inout_ CNamesArray& ports)
{
  //Reset the output parameter
  ports.clear();

  ATL::CRegKey serialCommKey;
  LSTATUS nStatus = serialCommKey.Open(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DEVICEMAP\\SERIALCOMM"), KEY_QUERY_VALUE);
  if (nStatus != ERROR_SUCCESS)
  {
    SetLastError(nStatus);
    return false;
  }

  //Get the max value name and max value lengths
  DWORD dwMaxValueNameLen = 0;
  nStatus = RegQueryInfoKey(serialCommKey, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &dwMaxValueNameLen, nullptr, nullptr, nullptr);
  if (nStatus != ERROR_SUCCESS)
  {
    SetLastError(nStatus);
    return false;
  }

  const DWORD dwMaxValueNameSizeInChars = dwMaxValueNameLen + 1; //Include space for the null terminator

  //Allocate some space for the value name
  std::vector<TCHAR> valueName;
  valueName.resize(dwMaxValueNameSizeInChars);

  //Enumerate all the values underneath HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
  bool bContinueEnumeration = true;
  DWORD dwIndex = 0;
  while (bContinueEnumeration)
  {
    DWORD dwValueNameSize = dwMaxValueNameSizeInChars;
#pragma warning(suppress: 26446)
    valueName[0] = _T('\0');
#pragma warning(suppress: 26446)
    bContinueEnumeration = (RegEnumValue(serialCommKey, dwIndex, &(valueName[0]), &dwValueNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS);
    if (bContinueEnumeration)
    {
      String sPortName;
#pragma warning(suppress: 26446 26486)
      if (RegQueryValueString(serialCommKey, &(valueName[0]), sPortName))
#pragma warning(suppress: 26489)
        ports.push_back(sPortName);

      //Prepare for the next loop
      ++dwIndex;
    }
  }

  return true;
}
#endif //#ifndef NO_CENUMERATESERIAL_USING_REGISTRY

#ifndef NO_CENUMERATESERIAL_USING_GETCOMMPORTS
_Return_type_success_(return != false) bool CEnumerateSerial::UsingGetCommPorts(_Inout_ CPortsArray& ports)
{
  //Reset the output parameter
  ports.clear();

  using LPGETCOMMPORTS = ULONG (__stdcall *)(PULONG, ULONG, PULONG);
  HMODULE hDLL = LoadLibrary(_T("api-ms-win-core-comm-l1-1-0.dll"));
  if (hDLL == nullptr)
    return false;
#pragma warning(suppress: 26490)
  auto pGetCommPorts = reinterpret_cast<LPGETCOMMPORTS>(GetProcAddress(hDLL, "GetCommPorts"));
  if (pGetCommPorts == nullptr)
    return false;

  std::vector<ULONG> intPorts;
  intPorts.resize(255);
  ULONG nPortNumbersFound = 0;
#pragma warning(suppress: 26446 26472)
  const ULONG nReturn = pGetCommPorts(&(intPorts[0]), static_cast<ULONG>(intPorts.size()), &nPortNumbersFound);
  FreeLibrary(hDLL);
  if (nReturn != ERROR_SUCCESS)
  {
    SetLastError(nReturn);
    return false;
  }

  for (ULONG i=0; i<nPortNumbersFound; i++)
#pragma warning(suppress: 26446 26489)
    ports.push_back(intPorts[i]);

  //Return the success indicator
  return true;
}
#endif //#ifndef NO_CENUMERATESERIAL_USING_GETCOMMPORTS
