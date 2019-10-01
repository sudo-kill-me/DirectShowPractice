/*
#include <dshow.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "Quartz.lib")

void HideConsole() {
	ShowWindow(GetConsoleWindow(), SW_HIDE);
}

int main(void)	{

	HideConsole();

	IGraphBuilder* pGraph = NULL;
	IMediaControl* pControl = NULL;
	IMediaEvent* pEvent = NULL;

	// Initialize the COM library.
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		printf("ERROR - Could not initialize COM library");
		return 1;
	}

	// Create the filter graph manager and query for interfaces.
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
		IID_IGraphBuilder, (void**)& pGraph);

	if (FAILED(hr))
	{
		printf("ERROR - Could not create the Filter Graph Manager.");
		return 1;
	}

	hr = pGraph->QueryInterface(IID_IMediaControl, (void**)& pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)& pEvent);

	// Build the graph
	hr = pGraph->RenderFile(L"D:\\2_C++\\DirectShowWorkspace\\KeyboardCat.avi", NULL);
	if (SUCCEEDED(hr))
	{
		// Run the graph.
		hr = pControl->Run();
		if (SUCCEEDED(hr))
		{
			// Wait for completion.
			long evCode;
			pEvent->WaitForCompletion(25000, &evCode);
		}
	}

	pControl->Release();
	pEvent->Release();
	pGraph->Release();
	CoUninitialize();

	return 0;
}
*/