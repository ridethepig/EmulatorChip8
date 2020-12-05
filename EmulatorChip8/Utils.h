#pragma once

#include <Windows.h>
#include <tchar.h>
#include <commdlg.h>

class Configure {
public:
	Configure() : FPS(200), default_rom(TEXT("")), keymap_stat(TEXT("off")), keymap_on(FALSE) {}
	
	int load_config() {
		DWORD ret;
		TCHAR config_path[] = TEXT(".\\chip8.ini");
		TCHAR buffer[1023] = { 0 };
		GetCurrentDirectory(sizeof(buffer), buffer);
		_tprintf("%s\n", buffer);
		int fps = GetPrivateProfileInt(TEXT("main"), TEXT("fps"), -1, config_path);
		if (fps > 0) FPS = fps;
		ret = GetPrivateProfileString(TEXT("main"), TEXT("default_rom"), TEXT(""), 
			buffer, sizeof(buffer) / sizeof(TCHAR), config_path); // To avoid overflow
		if (_tcslen(buffer) > 3) {
			_tcscpy_s(default_rom, sizeof(buffer) / sizeof(TCHAR), buffer);
		}
		else if (GetLastError()){
			_tprintf(TEXT("Warning: configuration file[chip8.ini] not found\n"));			
			return 1;
		}
		memset(buffer, 0, sizeof(buffer));
		ret = GetPrivateProfileString(TEXT("main"), TEXT("keymap_on"), TEXT(""),
			buffer, sizeof(buffer) / sizeof(TCHAR), config_path); // To avoid overflow
		if (_tcslen(buffer) > 1) {
			_tcscpy_s(keymap_stat, sizeof(buffer) / sizeof(TCHAR), buffer);
			if (_tcscmp(keymap_stat, TEXT("on")) == 0) {
				keymap_on = TRUE;
			}
			else {
				keymap_on = FALSE;
			}
		}
		memset(buffer, 0, sizeof(buffer));
		if (keymap_on) {
			ret = GetPrivateProfileString(TEXT("keymap"), TEXT("keymap"), TEXT(""), 
				buffer, sizeof(buffer) / sizeof(TCHAR), config_path);
			if (_tcslen(buffer) == 16) {
				for (register int i = 0; i < 16; ++i) {
					if (32 <= buffer[i] && buffer[i] <= 126)
						keymap[buffer[i]] = i;
					else {
						_tprintf(TEXT("Warning: keymap remapping failed due to invalid character.\n"));
						keymap_on = FALSE;
						break;
					}
				}
			}
			else {
				_tprintf(TEXT("Warning: keymap remapping failed due to incorrect format.\n"));
				keymap_on = FALSE;
			}
		}

		return 0;
	}

	TCHAR* get_default_rom() {		
		if (_tcslen(default_rom) <= 2) {
			return nullptr;
		}
		return default_rom;
	}

	TCHAR* get_keymap_stat() {
		if (_tcslen(keymap_stat) < 2) {
			_tcscpy_s(keymap_stat, _tcslen(TEXT("None")), TEXT("None"));
		}
		return keymap_stat;
	}

	DWORD get_fps() {		
		return FPS;
	}

	BOOL get_keymap_on() {
		return keymap_on;
	}

	BYTE* get_keymap() {
		return keymap;
	}

private:
	TCHAR default_rom[1024];
	TCHAR keymap_stat[1024];
	DWORD FPS;
	BOOL keymap_on;
	BYTE keymap[256] = { 0 };
};

TCHAR* Open_file_dialog(TCHAR* init_dir = nullptr) {
	OPENFILENAME ofn; 
	ZeroMemory(&ofn, sizeof(ofn));
	TCHAR* filename = new TCHAR[MAX_PATH];	
	TCHAR szFilter[] = TEXT("All Files\0*.*\0\0");
	memset(filename, 0, sizeof(filename));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = szFilter;

	ofn.nFilterIndex = 1;
	
	ofn.lpstrFile = filename; ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = init_dir;
	ofn.lpstrTitle = TEXT("Open chip-8 ROM");

	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = TEXT("rom");
	if (GetOpenFileName(&ofn)){
		return filename;
	}
	_tprintf(TEXT("%d"), CommDlgExtendedError());
	return nullptr;
}