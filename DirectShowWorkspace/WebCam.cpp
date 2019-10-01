#include <dshow.h>
#include <windows.h> 

#pragma comment(lib,"Strmiids.lib") //used for direct show


IMoniker* pMoniker = NULL;

HRESULT InitCaptureGraphBuilder(
	IGraphBuilder** ppGraph,  // Receives the pointer.
	ICaptureGraphBuilder2** ppBuild  // Receives the pointer.
)
{
	if (!ppGraph || !ppBuild)
	{
		return E_POINTER;
	}
	IGraphBuilder* pGraph = NULL;
	ICaptureGraphBuilder2* pBuild = NULL;

	// Create the Capture Graph Builder.
	HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,
		CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)& pBuild);
	if (SUCCEEDED(hr))
	{
		// Create the Filter Graph Manager.
		hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
			IID_IGraphBuilder, (void**)& pGraph);
		if (SUCCEEDED(hr))
		{
			// Initialize the Capture Graph Builder.
			pBuild->SetFiltergraph(pGraph);

			// Return both interface pointers to the caller.
			*ppBuild = pBuild;
			*ppGraph = pGraph; // The caller must release both interfaces.
			return S_OK;
		}
		else
		{
			pBuild->Release();
		}
	}
	return hr; // Failed
}
HRESULT EnumerateDevices(REFGUID category, IEnumMoniker** ppEnum)
{
	// Create the System Device Enumerator.
	ICreateDevEnum* pDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the category.
		hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
		}
		pDevEnum->Release();
	}
	return hr;
}
void DisplayDeviceInformation(IEnumMoniker* pEnum)
{

	while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
	{
		IPropertyBag* pPropBag;
		HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;
		}

		VARIANT var;
		VariantInit(&var);

		// Get description or friendly name.
		hr = pPropBag->Read(L"Description", &var, 0);
		if (FAILED(hr))
		{
			hr = pPropBag->Read(L"FriendlyName", &var, 0);
		}
		if (SUCCEEDED(hr))
		{
			printf("%S\n", var.bstrVal);
			if (0 == wcscmp(var.bstrVal, L"Integrated Webcam"))
			{
				VariantClear(&var);
				pPropBag->Release();
				return;
			}
			VariantClear(&var);
		}

		hr = pPropBag->Write(L"FriendlyName", &var);

		pPropBag->Release();
	}
}

void main()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	IGraphBuilder* pGraph = NULL;
	ICaptureGraphBuilder2* pBuild = NULL;
	IMediaControl* pControl = NULL;
	IMediaEvent* pEvent = NULL;

	if (FAILED(hr))
	{
		printf("ERROR - Could not initialize COM library\n");
		return;
	}

	printf("SUCCESS - Initialized COM library.\n");

	hr = InitCaptureGraphBuilder(&pGraph, &pBuild);

	if (FAILED(hr))
	{
		printf("ERROR - Could not create the Filter Graph Manager.\n");
		return;
	}

	printf("SUCCESS - Created the Filter Graph Manager.\n");

	hr = pGraph->QueryInterface(IID_IMediaControl, (void**)& pControl);
	hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)& pEvent);

	if (FAILED(hr))
	{
		printf("ERROR - Could not create the Media Control or Media Event Interfaces.\n");
		return;
	}

	printf("SUCCESS - Created the Media Control and Media Event Interfaces.\n");

	IEnumMoniker* pEnum;

	hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);

	if (FAILED(hr))
	{
		printf("ERROR - Failed to enumerate devices.\n");
		return;
	}

	printf("SUCCESS - Enumerated devices.\n");


	printf("--------------DEVICE INFORMATION--------------\n");
	DisplayDeviceInformation(pEnum);
	printf("------------END DEVICE INFORMATION------------\n");

	IBaseFilter* pCap = NULL;
	hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)& pCap);

	if (FAILED(hr))
	{
		printf("ERROR - Failed to bind to moniker.\n");
		return;
	}

	printf("SUCCESS - Bound to moniker.\n");
	hr = pGraph->AddFilter(pCap, L"Capture Filter");
	hr = pBuild->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pCap, NULL, NULL);

	if (FAILED(hr))
	{
		printf("ERROR - RenderStream() failed.\n");
		return;
	}

	printf("SUCCESS - RenderStream() succeeded.\n");

	hr = pControl->Run();

	if (FAILED(hr))
	{
		printf("ERROR - Run() failed.\n");
		return;
	}

	printf("SUCCESS - Running.\n");

	long evCode;
	hr = pEvent->WaitForCompletion(INFINITE, &evCode);

	if (FAILED(hr))
	{
		printf("ERROR - WaitForCompletion() failed.\n");
		return;
	}

	hr = pControl->Stop();

	if (FAILED(hr))
	{
		printf("ERROR - Stop() failed.\n");
		return;
	}

	printf("SUCCESS - Stopping.\n");

	pControl->Release();
	pEvent->Release();
	pGraph->Release();
	pMoniker->Release();
	pEnum->Release();

	CoUninitialize();
}