#include "DeviceInfoTest.h"
#include "Platform/DeviceInfo.h"

DeviceInfoTest::DeviceInfoTest()
:	TestTemplate<DeviceInfoTest>("DeviceInfoTest")
{
	RegisterFunction(this, &DeviceInfoTest::TestFunction, Format("DeviceInfo test"), NULL);
}

void DeviceInfoTest::LoadResources()
{
}

void DeviceInfoTest::UnloadResources()
{
}

void DeviceInfoTest::Draw(const DAVA::UIGeometricData &geometricData)
{
}

void DeviceInfoTest::TestFunction(TestTemplate<DeviceInfoTest>::PerfFuncData *data)
{
    String platform = DeviceInfo::GetPlatformString();
    String version = DeviceInfo::GetVersion();
    String manufacturer = DeviceInfo::GetManufacturer();
    String model = DeviceInfo::GetModel();
    String locale = DeviceInfo::GetLocale();
    String region = DeviceInfo::GetRegion();
    String timezone = DeviceInfo::GetTimeZone();
    String udid = DeviceInfo::GetUDID();

	Logger::Debug("********** Device info **********");
	Logger::Debug("Platform: %s", platform.c_str());
	Logger::Debug("OS version: %s", version.c_str());
	Logger::Debug("Manufacturer: %s", manufacturer.c_str());
	Logger::Debug("Model: %s", model.c_str());
	Logger::Debug("Locale: %s", locale.c_str());
	Logger::Debug("Region: %s", region.c_str());
	Logger::Debug("Time zone: %s", timezone.c_str());
    Logger::Debug("UDID: %s", udid.c_str());
	Logger::Debug("********** Device info **********");

	data->testData.message = "DeviceInfo test - passed";
	TEST_VERIFY(true);
}
