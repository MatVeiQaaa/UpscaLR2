#define NO_UNIMPLEMENTED

#include <DxLibWrap.hpp>

#include <ConfigManager.hpp>
#include <cstdint>
#include <vector>
#include <string_view>

#include <DxLib_LR2/include/DxLib.h>
#include <safetyhook.hpp>

static struct int2 {
	int x = 0;
	int y = 0;
};

static ConfigManager config("UpscaLR2.ini");

static bool justGotFocus = false;
int OnProcessMessage() {
	int res = ProcessMessage();
	static bool lastFocus = false;
	bool currentFocus = GetWindowActiveFlag() == TRUE;
	if (currentFocus == true) {
		if (currentFocus != lastFocus) justGotFocus = true;
		else justGotFocus = false;
	}
	lastFocus = currentFocus;
	return res;
}

int OnGetMouseWheelRotVol(int CounterReset) {
	auto isCursorOutbounds = []() {
		int2 pos;
		int2 size;
		auto GetMousePointReal = [](int* XBuf, int* YBuf) {
			int2 pos;
			struct {
				double x;
				double y;
			} scale;
			GetMousePoint(&pos.x, &pos.y);
			GetWindowSizeExtendRate(&scale.x, &scale.y);
			*XBuf = static_cast<int>(static_cast<double>(pos.x) * scale.x);
			*YBuf = static_cast<int>(static_cast<double>(pos.y) * scale.y);
		};
		GetMousePointReal(&pos.x, &pos.y);
		GetWindowSize(&size.x, &size.y);
		if (pos.x < 0 || pos.x > size.x ||
			pos.y < 0 || pos.y > size.y)
			return true;
		return false;
	};
	if (justGotFocus || isCursorOutbounds()) {
		GetMouseWheelRotVol(CounterReset);
		return 0;
	}
	return GetMouseWheelRotVol(CounterReset);
}

static bool intCoords = false;
int OnDrawModiGraphF(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int GrHandle, int TransFlag) {
	if (intCoords)
		return DrawModiGraph(x1 + 0.5f, y1 + 0.5f, x2 + 0.5f, y2 + 0.5f, x3 + 0.5f, y3 + 0.5f, x4 + 0.5f, y4 + 0.5f, GrHandle, TransFlag);
	else
		return DrawModiGraphF(x1, y1, x2, y2, x3, y3, x4, y4, GrHandle, TransFlag);
}

int OnDrawExtendGraphF(float x1f, float y1f, float x2f, float y2f, int GrHandle, int TransFlag) {
	if (intCoords)
		return DrawExtendGraph(x1f + 0.5f, y1f + 0.5f, x2f + 0.5f, y2f + 0.5f, GrHandle, TransFlag);
	else
		return DrawExtendGraphF(x1f, y1f, x2f, y2f, GrHandle, TransFlag);
}

int OnStopSoundMem(int SoundHandle) {
	return StopSoundMem(SoundHandle);
}

int OnCreateFontToHandle(const TCHAR* FontName, int Size, int Thick, int FontType, int CharSet, int EdgeSize, int Italic, unsigned int DataIndex, int Handle) {
	return CreateFontToHandle(FontName, Size, Thick, FontType, CharSet, EdgeSize, Italic, Handle);
}

int OnDrawBox(int x1, int y1, int x2, int y2, unsigned int Color, int FillFlag) {
	return DrawBox(x1, y1, x2, y2, Color, FillFlag);
}

int OnGetFontStateToHandle(TCHAR* FontName, int* Size, int* Thick, int FontHandle) {
	return GetFontStateToHandle(FontName, Size, Thick, FontHandle);
}

int OnDrawKeyInputString(int x, int y, int InputHandle) {
	return DrawKeyInputString(x, y, InputHandle);
}

int OnMakeKeyInput(size_t MaxStrLength, int CancelValidFlag, int SingleCharOnlyFlag, int NumCharOnlyFlag) {
	return MakeKeyInput(MaxStrLength, CancelValidFlag, SingleCharOnlyFlag, NumCharOnlyFlag);
}

static int2 resolutionCfg{};
static int2 resolutionOrig{};
static bool resolutionOverwrite = false;
int OnSetGraphMode(int ScreenSizeX, int ScreenSizeY, int ColorBitDepth, int RefreshRate) {
#ifdef NO_UNIMPLEMENTED
	return SetGraphMode(ScreenSizeX, ScreenSizeY, ColorBitDepth, 9999);
#else
	static int2 resolution = [](int x, int y) {
		resolutionOrig = { x, y };
		int2 res{};
		if (resolutionCfg.x == 0) {
			config.WriteValue("ResolutionX", x, true);
			res.x = x;
		}
		else res.x = resolutionCfg.x;
		if (resolutionCfg.y == 0) {
			config.WriteValue("ResolutionY", y, true);
			res.y = y;
		}
		else res.y = resolutionCfg.y;
		return res;
	}(ScreenSizeX, ScreenSizeY);
	return resolutionOverwrite ? SetGraphMode(resolution.x, resolution.y, ColorBitDepth, 9999) :
								 SetGraphMode(ScreenSizeX, ScreenSizeY, ColorBitDepth, 9999);
#endif
}

int OnSetWindowSizeExtendRate(double ExRateX, double ExRateY) {
	if (resolutionOverwrite) return SetWindowSizeExtendRate(
		ExRateX * resolutionOrig.x / resolutionCfg.x,
		ExRateY * resolutionOrig.y / resolutionCfg.y
	);
	return SetWindowSizeExtendRate(ExRateX, ExRateY);
}

void DxLibWrap::Init(uintptr_t selfModule) {
#ifdef NO_UNIMPLEMENTED
	config.WriteComment("DirectX",
		"Possible values: dx9, dx9ex"
	);
	auto dxVer = [](ConfigManager& config) {
		std::string val(config.ReadValue<std::string>("DirectX", "dx9"));
		if (val == "dx9") return DX_DIRECT3D_9;
		if (val == "dx9ex") return DX_DIRECT3D_9EX;
		return DX_DIRECT3D_9;
	}(config);
#else
	config.WriteComment("DirectX", 
		"Possible values: dx9, dx9ex, dx11\n"
		"dx11 is currently unimplemented, will default to dx9ex"
	);
	auto dxVer = [](ConfigManager& config) {
		std::string val(config.ReadValue<std::string>("DirectX", "dx9"));
		if (val == "dx9") return DX_DIRECT3D_9;
		if (val == "dx9ex") return DX_DIRECT3D_9EX;
		if (val == "dx11") return DX_DIRECT3D_11;
		return DX_DIRECT3D_9;
	}(config);
#endif

	config.WriteComment("Scaling", 
		"Possible values: bilinear, nearest, crt\n"
		"Scaling method for fullscreen mode. Does nothing in \"native\" fullscreen mode"
	);
	auto scaling = [](ConfigManager& config) {
		std::string val(config.ReadValue<std::string>("Scaling", "bilinear"));
		if (val == "bilinear") return DX_FSSCALINGMODE_BILINEAR;
		if (val == "nearest") return DX_FSSCALINGMODE_NEAREST;
		if (val == "crt") return DX_FSSCALINGMODE_CRT;
		return DX_FSSCALINGMODE_BILINEAR;
	}(config);

#ifdef NO_UNIMPLEMENTED
	auto fitScaling = FALSE;
#else
	config.WriteComment("KeepAspectRatio",
		"Possible values: true, false\n"
		"Currently does nothing\n"
		"Meant to tell if the fullscreen image would be stretched to screen horizontally or not"
	);
	auto fitScaling = config.ReadValue("KeepAspectRatio", true) ? FALSE : TRUE;
#endif

	config.WriteComment("FullscreenMode",
		"Possible values: desktop, borderless, native, maximum"
	);
	auto fsMode = [](ConfigManager& config) {
		std::string val(config.ReadValue<std::string>("FullscreenMode", "desktop"));
		if (val == "desktop") return DX_FSRESOLUTIONMODE_DESKTOP;
		if (val == "borderless") return DX_FSRESOLUTIONMODE_BORDERLESS_WINDOW;
		if (val == "native") return DX_FSRESOLUTIONMODE_NATIVE;
		if (val == "maximum") return DX_FSRESOLUTIONMODE_MAXIMUM;
		return DX_FSRESOLUTIONMODE_DESKTOP;
	}(config);

	config.WriteComment("IntegerCoordinates",
		"Possible values: false, true\n"
		"Makes the render use integer coordinates to position sprites on the screen\n"
		"This can make sprite edges look crisper, closer to source image\n"
		"Also fixes a bug where SRC coordinate may shift -1 pixel both X and Y"
	);
	intCoords = config.ReadValue("IntegerCoordinates", false);

#ifdef NO_UNIMPLEMENTED
	resolutionOverwrite = false;
#else
	config.WriteComment("ResolutionEnable",
		"Possible values: true, false\n"
		"Enables the system to override render resolution from config values\n"
		"Uses \"ResolutionX\" and \"ResolutionY\" values"
	);
	resolutionOverwrite = config.ReadValue("ResolutionEnable", false);

	config.WriteComment("ResolutionX",
		"Possible values: any integer\n"
		"Does nothing until \"ResolutionEnable\" is set to \'true\'"
	);
	resolutionCfg.x = config.ReadValue("ResolutionX", 0);
	resolutionCfg.y = config.ReadValue("ResolutionY", 0);
#endif

	config.SaveConfig(); // Updates the comments in existing config file.

	SetUseDirectInputFlag(1);
	SetDirectInputMouseMode(0);
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);
	SetUseDirect3DVersion(dxVer);
	SetFullScreenScalingMode(scaling, fitScaling);
	SetFullScreenResolutionMode(fsMode);

	static std::vector<safetyhook::InlineHook> hooks;
	auto hook = []<class T>(int target, T(*destination)) {
		hooks.push_back(safetyhook::create_inline(target, destination));
	};

	// Main
	hook(0x4C7FE0, ErrorLogAdd);
	hook(0x4C8660, ErrorLogFmtAdd);
	hook(0x50AC30, SetUse3DFlag);
	hook(0x50F930, OnSetGraphMode);
	hook(0x4CDB60, SetWindowSizeChangeEnableFlag);
	hook(0x4CE100, SetNotSoundFlag);
	hook(0x4CDBA0, OnSetWindowSizeExtendRate);
	hook(0x517910, SetWaitVSyncFlag);
	hook(0x4CD7D0, ChangeWindowMode);
	hook(0x50E810, GetDirectDrawDeviceNum);
	hook(0x50E4D0, SetUseDirectDrawDeviceIndex);
	hook(0x4CD9F0, SetMainWindowText);
	hook(0x4CDA60, SetOutApplicationLogValidFlag);
	hook(0x50E4C0, SetMultiThreadFlag);
	hook(0x4CE690, SetUseFPUPreserveFlag);
	hook(0x511EF0, DxLib_Init);
	hook(0x511F20, DxLib_End);
	hook(0x51CEC0, ChangeFont);
	hook(0x4CE640, SetLogFontSize);
	hook(0x4CDD90, SetSysCommandOffFlag);
	hook(0x50A330, SetDrawScreen);
	hook(0x4CDA80, SetAlwaysRunFlag);
	hook(0x4D09E0, SetMouseDispFlag);
	hook(0x4C9830, clsDx);
	hook(0x4C94B0, printfDx);
	hook(0x4EC3C0, MakeGraph);
	hook(0x5129B0, LoadGraph);
	hook(0x4EDC70, DeleteGraph);
	hook(0x4F1A30, DrawGraph);
	hook(0x50F210, ScreenFlip);
	hook(0x511FC0, OnProcessMessage);
	hook(0x4CCD80, GetWindowModeFlag);
	hook(0x4CD120, GetWindowSize);
	hook(0x517780, SetTransColor);
	hook(0x4CCD70, GetMainWindowHandle);
	hook(0x4F1780, ClsDrawScreen);
	hook(0x4D0BE0, WaitTimer);
	hook(0x4D0DD0, GetDateTime);
	hook(0x510060, SaveDrawScreenToPNG);
	hook(0x50C9A0, GetDrawScreenGraph);
	hook(0x509F10, SetDrawMode);
	hook(0x509F90, SetDrawBlendMode);
	hook(0x4F34D0, DrawExtendGraph);
	hook(0x511700, SeekMovieToGraph);
	hook(0x511550, PlayMovieToGraph);

	// ProcessInput
	hook(0x4CCD60, GetWindowActiveFlag);
	hook(0x4D0A90, GetMousePoint);
	hook(0x4D0BB0, OnGetMouseWheelRotVol);
	hook(0x5392C0, GetMouseInput);
	hook(0x539460, GetHitKeyStateAll);
	hook(0x5395D0, GetJoypadInputState);

	// InitSkin
	hook(0x51BE40, InitFontToHandle);

	// Sound
	hook(0x532300, ChangeVolumeSoundMem);
	hook(0x5325C0, GetFrequencySoundMem);
	hook(0x5324F0, SetFrequencySoundMem);
	hook(0x531F60, OnStopSoundMem);
	hook(0x5319B0, DeleteSoundMem);
	hook(0x532FB0, SetCreateSoundDataType);
	hook(0x5314B0, LoadSoundMem);
	hook(0x532A30, GetSoundTotalTime);
	hook(0x532AF0, GetSoundCurrentTime);
	hook(0x531D60, PlaySoundMem);
	
	// SkinLoad/Load
	hook(0x50D7E0, GetGraphSize);
	hook(0x50DEF0, DerivationGraph);
	hook(0x4C98B0, FileRead_open);
	hook(0x4C9A50, FileRead_gets);
	hook(0x4C9950, FileRead_close);
	hook(0x511820, SetMovieVolumeToGraph);
	hook(0x4C86E0, ErrorLogTabSub);
	hook(0x51C000, OnCreateFontToHandle);

	// SkinDraw
	hook(0x50A250, SetDrawBright);
	hook(0x4DE940, VectorRotationZ);
	hook(0x4FAD10, OnDrawModiGraphF);
	hook(0x50C770, GetColor);
	hook(0x503250, OnDrawBox);
	hook(0x4F4DC0, OnDrawExtendGraphF);
	hook(0x52C200, OnGetFontStateToHandle);
	hook(0x52C060, GetDrawStringWidthToHandle);
	hook(0x52C500, DrawStringToHandle);
	hook(0x52CD40, DrawExtendStringToHandle);
	hook(0x4D79A0, GetKeyInputString);
	hook(0x4D5AC0, SetKeyInputStringFont);
	hook(0x4D6F20, OnDrawKeyInputString);
	hook(0x4D7CB0, GetIMEInputData);
	hook(0x4D7C40, GetKeyInputCursorPosition);
	hook(0x4D5800, GetIMEInputModeStr);
	hook(0x50A7E0, SetDrawArea);
	hook(0x4D5D90, DeleteKeyInput);
	hook(0x4D5BD0, InitKeyInput);
	hook(0x4D5C60, OnMakeKeyInput);
	hook(0x4D5EA0, SetActiveKeyInput);
	hook(0x4D7600, SetKeyInputString);
	hook(0x4D6010, CheckKeyInput);
}