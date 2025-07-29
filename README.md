## NOTE:
Starting with Chrome version 127 (released around July 2024), Google introduced Application-Bound (App-Bound) Encryption. So this utility will work on chrome & edge upto v127.

## What is does?
Extract stored credentials of various browsers (i.e. Chrome, Brave, Edge, Opera).

#### Upate for firefox will be soon added.
## Quick Start

Grab the latest binary from the [release section](https://github.com/tajiknomi/Browsers_MasterKey_Extractor/releases) and turn off the AV on the machine
 

1: Run the `extractMasterKeys.exe` on the target machine to extract the AES Master key of various browser(s)
```
$ extractMasterKeys.exe <path/to/output/dir>
```

`<path/to/output/dir>` will contain the output extracted keys and corresponding files of installed browsers.


2: Run the python script i.e. `browserCredDecryptor.py` on your machine and provide the key and files extracted by the above utility. e.g.

```
$ python browserCredDecryptor.py
Enter the AES key (hex): 37ac4ebd676cf9bfb910a4e34a9778467cc288730091f3da057adb7141d3de2b
Enter the path to the 'Login Data' file: path/to/Login Data
``` 

### Note:
`browserCredDecryptor.py` depends on `pycryptodome` package and its better to install the package in python virtual environment so that it doesn't conflict with your crypto package(s).

Windows
```
$ python -m venv virtual_env && virtual_env\Scripts\activate
$ pip install pycryptodome
```
Linux
```
$ apt install python3.12-venv && python3 -m venv virtual_env && source virtual_env/bin/activate
$ pip install pycryptodome
```

## The reason behind spliting the process
During a security assessment, you can utilize `extractMasterKeys.exe` to extract the master key and essential files without requiring additional dependency on the target. By focusing solely on key extraction,it keeps the software footprint small and is slightly OPSEC-SAFE tactic.

## How to build
```
$ cd "Extract Master Keys" && mkdir build && cd build
$ cmake ..
$ cmake --build . --config Release
```

You can specify the [cmake-generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) to create configuration files for your build system e.g.

```
$ cmake -A x64 ..
$ cmake -A Win32 ..
$ cmake -G "Visual Studio 16 2019" -A x64 ..
```

## Disclaimer

This software is intended for educational and research purposes, particularly in offensive security (offsec). It is your responsibility to use it ethically and at your own risk. The developer is not liable for any misuse or consequences of using this software.

## LICENSE
[GNU General Public License](https://www.gnu.org/licenses/gpl-3.0.en.html)
