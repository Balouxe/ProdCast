#include <ProdCast.h>
#include "AudioSources/AudioFile.h"
#include "ProcessingChain.h"
#include "Effects/Filters/IIRFilter.h"
#include "VST/VSTPlugin.h"
#include <thread>
#include <iostream>
#include <windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

class windowPlugFrameListener : public ProdCast::VST::PlugFrameListener {
public:
	windowPlugFrameListener(HWND window) {
		m_window = window;
	}

	void OnResizePlugView(Steinberg::ViewRect const& newSize) {
		RECT rcClient, rcWind;
		POINT ptDiff;
		GetClientRect(m_window, &rcClient);
		GetWindowRect(m_window, &rcWind);
		ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
		ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;

		HDWP pos = BeginDeferWindowPos(1);
		DeferWindowPos(pos, m_window, NULL, newSize.left, newSize.top, newSize.getWidth() + ptDiff.x, newSize.getHeight() + ptDiff.y, SWP_NOMOVE);
		EndDeferWindowPos(pos);
	};
private:
	HWND m_window;
};

int main() {



	WNDCLASS windowClass = { 0 };
	windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hInstance = NULL;
	windowClass.lpfnWndProc = WndProc;
	windowClass.lpszClassName = L"Window in Console";
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	if (!RegisterClass(&windowClass))
		MessageBox(NULL, L"Could not register class", L"Error", MB_OK);
	HWND windowHandle = CreateWindow(L"Window in Console",
		NULL,
		(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX),
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		NULL,
		NULL,
		NULL,
		NULL);

	ShowWindow(windowHandle, SW_RESTORE);


	ProdCast::AudioSettings settings;
	settings.sampleRate = 48000;
	settings.audioBackend = ProdCast::BE_PORTAUDIO;
	ProdCast::ProdCastEngine* engine = new ProdCast::ProdCastEngine(settings);
	engine->setMasterGain(1.0f);

	ProdCast::AudioFile file1(engine);
	file1.LoadFile("F:/Dev/Projets/ProdCast/bin/Debug-x64/FrogCast/tests/flactest.flac");
	file1.setVolume(0.5f);

	ProdCast::VST::VSTEffect* effect = new ProdCast::VST::VSTEffect(engine, "C:/Program Files/Common Files/VST3/Wider.vst3");

	ProdCast::ProcessingChain* proc = new ProdCast::ProcessingChain(engine);
	proc->AddEffect(effect, 0);
	file1.ApplyProcessingChain(proc);
	file1.AddParent(engine->getMasterBus());
	file1.Play();

	ProdCast::VST::PlugFrameListener* listener = new windowPlugFrameListener(windowHandle);
	effect->getPlugin()->OpenEditor(windowHandle, listener);


	MSG messages;
	while (GetMessage(&messages, NULL, 0, 0) > 0)
	{
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(90s);
	DeleteObject(windowHandle); //doing it just in case
	return messages.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_CHAR: //this is just for a program exit besides window's borders/taskbar
		if (wparam == VK_ESCAPE)
		{
			DestroyWindow(hwnd);
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wparam, lparam);
	}
}