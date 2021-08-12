#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include "../libethcore/Farm.h"
#include <cuda_runtime.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include "nvmlapi.h"


std::auto_ptr<nvmlapi> nvmlapi::Instance_;

#define DefFuncType(FuncName) typedef decltype(&FuncName) FN##FuncName;


template<typename T, typename... Args>
nvmlReturn_t safeCall(HMODULE hModule, const T& name, Args&& ... args)
{
	T pfn = name;// reinterpret_cast<T>(dlsym(hModule, name.c_str()));
	if (pfn != NULL)
	{
		return  pfn(std::forward<Args>(args)...);
	}
	else
	{
		return NVML_ERROR_UNINITIALIZED;
	}

}

#define SAFE_CALL(hModule, name, ...) (safeCall<FN ## name>(hModule, name, __VA_ARGS__ ))


DefFuncType(nvmlInit);
DefFuncType(nvmlDeviceGetHandleByIndex);
DefFuncType(nvmlDeviceGetName);
DefFuncType(nvmlDeviceGetUUID);
DefFuncType(nvmlDeviceGetCudaComputeCapability);
DefFuncType(nvmlDeviceGetClockInfo);
DefFuncType(nvmlDeviceGetMaxClockInfo);
DefFuncType(nvmlDeviceGetTemperature);
DefFuncType(nvmlDeviceGetFanSpeed);
DefFuncType(nvmlDeviceGetPowerUsage);
DefFuncType(nvmlDeviceGetCudaComputeCapability);
DefFuncType(nvmlSystemGetCudaDriverVersion);
DefFuncType(nvmlSystemGetDriverVersion);
DefFuncType(nvmlDeviceGetMemoryInfo);
DefFuncType(nvmlDeviceGetGridLicensableFeatures);
DefFuncType(nvmlDeviceGetUtilizationRates);
DefFuncType(nvmlDeviceGetPciInfo);



nvmlapi::nvmlapi()
{
	hNVMLModule_ = NULL;
	hNvidiaModule_ = NULL;
	m_jSwBuilder.settings_["indentation"] = "";
}

nvmlapi::~nvmlapi()
{


}

nvmlapi& nvmlapi::Instance()
{
	if (Instance_.get() == NULL)
	{
		Instance_ = std::auto_ptr< nvmlapi>(new nvmlapi);
	}

	return *Instance_.get();
}


bool nvmlapi::Initialize()
{
	nvmlReturn_t result;
	unsigned int device_count, i;

	// First initialize NVML library
	result = nvmlInit();//SAFE_CALL(hNVMLModule_, nvmlInit);
			 //SAFE_CALL(hNVMLModule_, nvmlInit);
	if (NVML_SUCCESS != result)
	{
		printf("Failed to initialize NVML: %s\n", nvmlErrorString(result));

		printf("Press ENTER to continue...\n");
		getchar();
		return 1;
	}


#if 0
	const char* pstr = "/usr/lib64/libnvidia-ml.so";// "/usr/lib64/libnvidia-nvvm.so";
	hNVMLModule_ = dlopen(pstr, RTLD_NOW);
	if (hNVMLModule_ != NULL)
	{
		nvidia_query = (QueryPtr)dlsym(hNVMLModule_,"nvapi_QueryInterface");
	}
#endif
	//nvidia_query = (QueryPtr)nvapi_QueryInterface;

	
	return true;// nvidia_query != NULL;
}


bool nvmlapi::UpdateDeviceDetail(std::string& gpu_Detail)
{
	bool bRet = false;
	nvmlReturn_t result = NVML_ERROR_UNKNOWN;
	unsigned int device_count = 0;
	result =  nvmlDeviceGetCount( &device_count);
	if (NVML_SUCCESS != result)
	{
		return bRet;
	}
	Json::Value jReq;

	jReq["detail"] = Json::Value(Json::arrayValue);

	for (unsigned int i = 0; i < device_count; i++)
	{
		nvmlDevice_t device = { 0 };
		cudaDeviceProp props = { 0 };
		NVAPI_GPU_POWER_STATUS power_status = { 0 };
		NVAPI_GPU_POWER_TOPO   power_topo = { 0 };
		NV_DISPLAY_DVC_INFO_EX display_dev_info = { 0 };
		NvUsages usages = { 0 };
		std::string ram_type;
		std::string ram_mark;
		power_status.version = NVAPI_GPU_POWER_STATUS_VER;
		power_topo.version = NVAPI_GPU_POWER_TOPO_VERSION;
		display_dev_info.version = NVAPI_DISPLAY_DVC_INFO_EX_VER;
		usages.Version = NVAPI_GPU_USAGES_VERSION;
		auto spBuffer = std::make_shared<GPU_Info>();
		//*
		if (cudaGetDeviceProperties(&props, i) != cudaSuccess)
		{
			return bRet;
		}//*/
		memset(spBuffer.get(),0,sizeof(GPU_Info));
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetHandleByIndex, i, &device);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{
			//printf("Failed to get handle for device %u: %s\n", i, SAFE_CALL(hNVMLModule_, nvmlErrorString, result));
			return bRet;
		}

		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetName, device, spBuffer->szName, NVML_DEVICE_NAME_BUFFER_SIZE);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}

#if 0
		result = nvmlDeviceGetPciInfo_2(device, &spBuffer->nvPciInfo);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
#endif


		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetClockInfo, device, NVML_CLOCK_GRAPHICS, &spBuffer->graphics_clock);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetMaxClockInfo, device, NVML_CLOCK_GRAPHICS, &spBuffer->max_graphics_clock);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}

		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetClockInfo, device, NVML_CLOCK_SM, &spBuffer->sm_clock);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetMaxClockInfo, device, NVML_CLOCK_SM, &spBuffer->max_sm_clock);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetClockInfo, device, NVML_CLOCK_MEM, &spBuffer->mem_clock);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetMaxClockInfo, device, NVML_CLOCK_MEM, &spBuffer->max_mem_clock);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}

		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetClockInfo, device, NVML_CLOCK_VIDEO, &spBuffer->video_clock);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetMaxClockInfo, device, NVML_CLOCK_VIDEO, &spBuffer->max_video_clock);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}





		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetTemperature, device, NVML_TEMPERATURE_GPU, &spBuffer->nTemperature);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetFanSpeed, device, &spBuffer->nFanSpeed);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetPowerUsage, device, &spBuffer->nPowerUsage);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}

		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetCudaComputeCapability, device, &spBuffer->nCCCMajor, &spBuffer->nCCCMinor);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}

		result = SAFE_CALL(hNVMLModule_, nvmlSystemGetCudaDriverVersion, &spBuffer->nCudaDrvVer);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}


		result = SAFE_CALL(hNVMLModule_, nvmlSystemGetDriverVersion, spBuffer->szDrvVer, NVML_DEVICE_NAME_BUFFER_SIZE);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}

		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetMemoryInfo, device, &spBuffer->nvMemInfo);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetGridLicensableFeatures, device, &spBuffer->Licens);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetUUID, device, spBuffer->szUUID, sizeof(spBuffer->szUUID));
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetUtilizationRates, device, &spBuffer->nvUtilization);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetPciInfo, device, &spBuffer->nvPciInfo);
		if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
		{

			return bRet;
		}
		Json::Value gpu;
		uint64_t hashrate;
		char fulluniqueId[128] = { 0 };
		sprintf(fulluniqueId, NVML_DEVICE_PCI_BUS_ID_LEGACY_FMT, 0, props.pciBusID, props.pciDeviceID);
		if (get_hashrate(std::string(fulluniqueId), hashrate))
		{
			gpu["rate"] = hashrate;

		}
		if (get_ram_maker(spBuffer->nvRamMaker, ram_mark))
		{
			gpu["ram_mark"] = ram_mark;
		}

		if (get_ram_type(spBuffer->nvRamType, ram_type))
		{
			gpu["ram_type"] = ram_type;


		}
		



		gpu["Name"] = spBuffer->szName;
		gpu["DrvVer"] = spBuffer->szDrvVer;
		gpu["uuid"] = spBuffer->szUUID;
		char VendorId[32] = { 0 };
		char subVendorId[32] = { 0 };
		sprintf(VendorId, "%04X", spBuffer->nvPciInfo.pciDeviceId & 0xFFFF);
		gpu["VendorId"] = VendorId;
		sprintf(subVendorId, "%04X", spBuffer->nvPciInfo.pciSubSystemId & 0xFFFF);
		gpu["subVendorId"] = subVendorId;

		gpu["Temperature"] = spBuffer->nTemperature;
		gpu["FanSpeed"] = spBuffer->nFanSpeed;
		gpu["PowerUsage"] = spBuffer->nPowerUsage;
		gpu["gpu_clock"] = props.clockRate;
		gpu["gpu_load"] = usages.Usages[2];
		gpu["memory_clock"] = props.memoryClockRate;
		Json::Value clock;
		Json::Value sm;
		sm["curclock"] = spBuffer->sm_clock;
		sm["maxclock"] = spBuffer->max_sm_clock;
		clock["sm"] = sm;

		Json::Value mem;
		mem["curclock"] = spBuffer->mem_clock;
		mem["maxclock"] = spBuffer->max_mem_clock;
		clock["mem"] = mem;

		Json::Value video;
		video["curclock"] = spBuffer->video_clock;
		video["maxclock"] = spBuffer->max_video_clock;
		clock["video"] = video;

		Json::Value graphics;
		graphics["curclock"] = spBuffer->graphics_clock;
		graphics["maxclock"] = spBuffer->max_graphics_clock;
		clock["graphics"] = graphics;

		gpu["clock"] = clock;

		Json::Value ut;
		ut["gpu"] = spBuffer->nvUtilization.gpu;
		ut["memory"] = spBuffer->nvUtilization.memory;
		gpu["Utilization"] = ut;
		//int percent = (int)power_status.entries[0].power / 1000.0;
		//double percent = (double)power_topo.entries[0].power / 1000.0;
		//double percent2 = (double)power_topo.entries[1].power / 1000.0;
		gpu["powerlimit"] = (int)power_topo.entries[0].power;
		//gpu["meminfo"] = Json::Value(Json::arrayValue);
		Json::Value memory;
#if 1
		memory["total"] = (long)spBuffer->nvMemInfo.total;
		memory["used"] = (long)spBuffer->nvMemInfo.used;
		memory["Free"] = (long)spBuffer->nvMemInfo.free;
		gpu["meminfo"] = memory;
#endif
		
		//gpu["PciInfo"] = Json::Value(Json::arrayValue);
		Json::Value pci;
		pci["busIdLegacy"] = spBuffer->nvPciInfo.busIdLegacy;
		pci["domain"] = spBuffer->nvPciInfo.domain;
		pci["bus"] = spBuffer->nvPciInfo.bus;
		pci["device"] = spBuffer->nvPciInfo.device;
		pci["pciDeviceId"] = spBuffer->nvPciInfo.pciDeviceId;
		pci["bus"] = spBuffer->nvPciInfo.bus;
		pci["device"] = spBuffer->nvPciInfo.device;
		pci["pciDeviceId"] = spBuffer->nvPciInfo.pciDeviceId;
		pci["pciSubSystemId"] = spBuffer->nvPciInfo.pciSubSystemId;
		pci["busId"] = spBuffer->nvPciInfo.busId;

		gpu["PciInfo"] = pci;

		jReq["detail"].append(gpu);

	}
	gpu_Detail = Json::writeString(m_jSwBuilder, jReq);
	return true;

}

bool nvmlapi::get_hashrate(std::string uniqueId, uint64_t& rate)
{
	dev::eth::TelemetryType& telemetry = dev::eth::Farm::f().Telemetry();

	std::vector< dev::eth::TelemetryAccountType>::iterator vit = telemetry.miners.begin();// miners;
	for (; vit != telemetry.miners.end(); vit++)
	{
		const  dev::eth::TelemetryAccountType& telemet = *vit;
		if (uniqueId == telemet.uniqueId)
		{
			rate = telemet.hashrate;
			return true;
		}

	}
	return false;


}
bool nvmlapi::get_uuid(int index, char* uuid, int size)
{
	nvmlReturn_t result = NVML_ERROR_UNKNOWN;
	nvmlDevice_t device = { 0 };
	result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetHandleByIndex, index, &device);
	if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
	{
		//printf("Failed to get handle for device %u: %s\n", i, SAFE_CALL(hNVMLModule_, nvmlErrorString, result));
		return false;
	}
	result = SAFE_CALL(hNVMLModule_, nvmlDeviceGetUUID, device, uuid, size);
	if (NVML_SUCCESS != result && NVML_ERROR_NOT_SUPPORTED != result)
	{

		return false;
	}
	return true;
}


bool nvmlapi::get_ram_type(NV_RAM_TYPE  nvRamType, std::string& ramTypeString)
{

	switch (nvRamType)
	{
	case NV_RAM_TYPE_SDRAM:
		ramTypeString = "SDRAM";
		break;
	case NV_RAM_TYPE_DDR1:
		ramTypeString = "DDR1";
		break;
	case NV_RAM_TYPE_DDR2:
		ramTypeString = "DDR2";
		break;
	case NV_RAM_TYPE_GDDR2:
		ramTypeString = "GDDR2";
		break;
	case NV_RAM_TYPE_GDDR3:
		ramTypeString = "GDDR3";
		break;
	case NV_RAM_TYPE_GDDR4:
		ramTypeString = "GDDR4";
		break;
	case NV_RAM_TYPE_DDR3:
		ramTypeString = "DDR3";
		break;
	case NV_RAM_TYPE_GDDR5:
		ramTypeString = "GDDR5";
#if 0
	case NV_RAM_TYPE_DDR5X:
		ramTypeString = "GDDR5X";
		break;

	case NV_RAM_TYPE_LPDDR2:
		ramTypeString = "LPDDR2";
		break;

	default:
		ramTypeString = std::to_string(nvRamType);
		break;
#endif
	}
	return true;

}



bool nvmlapi::get_ram_maker(NV_RAM_MAKER  nvRamMaker, std::string& ramMakerString)
{
	switch (nvRamMaker)
	{
	case NV_RAM_MAKER_SAMSUNG:
		ramMakerString = "Samsung";
		break;
	case NV_RAM_MAKER_QIMONDA:
		ramMakerString = "Qimonda";
		break;
	case NV_RAM_MAKER_ELPIDA:
		ramMakerString = "Elpida";
		break;
	case NV_RAM_MAKER_ETRON:
		ramMakerString = "Etron";
		break;
	case NV_RAM_MAKER_NANYA:
		ramMakerString = "Nanya";
		break;
	case NV_RAM_MAKER_HYNIX:
		ramMakerString = "Hynix";
		break;
	case NV_RAM_MAKER_MOSEL:
		ramMakerString = "Mosel";
		break;
	case NV_RAM_MAKER_WINBOND:
		ramMakerString = "Winbond";
		break;
	case NV_RAM_MAKER_ELITE:
		ramMakerString = "Elite";
		break;
	case NV_RAM_MAKER_MICRON:
		ramMakerString = "Micron";
		break;
	default:
		ramMakerString = std::to_string(nvRamMaker);
		break;
	}

	return true;

}



#define NVAPI_GPU_USAGES_GET 0x189A1FDF

NvAPI_Status nvmlapi::GetUsagesType(NvPhysicalGpuHandle gpuHandle, NvUsages* pnvUsages)
{
	static NvAPI_Status(*pointer)(NvPhysicalGpuHandle, NvUsages*) = NULL;

	if (pointer == NULL)
	{
		if (nvidia_query != NULL)
		{
			pointer = (NvAPI_Status(*)(NvPhysicalGpuHandle, NvUsages*))nvidia_query(NVAPI_GPU_USAGES_GET);

		}
	}
	if (pointer == NULL)
		return NVAPI_NO_IMPLEMENTATION;

	return (*pointer)(gpuHandle, pnvUsages);


}

