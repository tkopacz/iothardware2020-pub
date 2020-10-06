#include "sha256.h"
#include <sstream>
#include <time.h>
#include "SasKeyForIotHub.h"

String SasKeyForIotHub::urlEncode(const String url)
{
	static const char* hex = "0123456789ABCDEF";
	static const std::string specials = "-._";

	String result = "";

	for (uint8_t i = 0; i < url.length(); i++)
	{
		if (('a' <= url[i] && url[i] <= 'z') ||
			('A' <= url[i] && url[i] <= 'Z') ||
			('0' <= url[i] && url[i] <= '9') ||
			(std::string::npos != specials.find(url[i], 0)))
		{
			result += url[i];
		}
		else
		{
			result += '%';
			result += hex[url[i] >> 4];
			result += hex[url[i] & 15];
		}
	}

	return result;
}

int SasKeyForIotHub::decodeBase64(const String input, uint8_t* output, uint32_t outputLength)
{
	int b[4];

	if (input.length() % 4 != 0)
		return -1;    // Base64 string's length must be a multiple of 4
	int ii = input.indexOf('=');
	uint32_t requiredLen = (input.length() * 3) / 4 - (ii != -1 ? (input.length() - ii) : 0);

	if (outputLength == 0 || output == NULL)
		return requiredLen;

	if (requiredLen > outputLength)
		return -2;    // Output buffer is too short

	int j = 0;

	for (uint32_t i = 0; i < input.length(); i += 4)
	{
		b[0] = CODES.indexOf(input[i]);
		b[1] = CODES.indexOf(input[i + 1]);
		b[2] = CODES.indexOf(input[i + 2]);
		b[3] = CODES.indexOf(input[i + 3]);

		output[j++] = ((b[0] << 2) | (b[1] >> 4));

		if (b[2] < 64)
		{
			output[j++] = ((b[1] << 4) | (b[2] >> 2));

			if (b[3] < 64)
			{
				output[j++] = ((b[2] << 6) | b[3]);
			}
		}
	}

	return requiredLen;
}

String SasKeyForIotHub::encodeBase64(const uint8_t* input, int inputLength)
{
	String result = "";

	int8_t b;

	for (int i = 0; i < inputLength; i += 3)
	{
		b = (input[i] & 0xfc) >> 2;
		result += CODES[b];
		b = (input[i] & 0x03) << 4;

		if (i + 1 < inputLength)
		{
			b |= (input[i + 1] & 0xF0) >> 4;
			result += CODES[b];
			b = (input[i + 1] & 0x0F) << 2;

			if (i + 2 < inputLength)
			{
				b |= (input[i + 2] & 0xC0) >> 6;
				result += CODES[b];
				b = input[i + 2] & 0x3F;
				result += CODES[b];
			}
			else
			{
				result += CODES[b];
				result += ('=');
			}
		}
		else
		{
			result += CODES[b];
			result += ("==");
		}
	}
	return result;
}

String SasKeyForIotHub::hashIt(const String data, uint8_t* key, size_t keyLength)
{
	uint8_t signedOut[32];
	String work;

	generateHash(signedOut, (uint8_t*)data.c_str(), (size_t)data.length(), key, keyLength);
	work = encodeBase64(signedOut, sizeof(signedOut));
	return urlEncode(work);
}

void SasKeyForIotHub::init() {
	IoTHub_resourceURI = IoTHub_name + ".azure-devices.net";
	IoTHub_apiVersion = "2018-06-30";

	fileUploadRequestURI = "https://" + IoTHub_resourceURI + "/devices/" + IoTHub_deviceId + "/files?api-version=" + IoTHub_apiVersion;
	notificationURI = "https://" + IoTHub_resourceURI + "/devices/" + IoTHub_deviceId + "/files/notifications?api-version=" + IoTHub_apiVersion;
}


/*-------------------------*/

String SasKeyForIotHub::get_iot_hub_sas_token() {
	int32_t epoch = (int32_t)time(0);
	int32_t tokenExpiry = epoch + IoTHub_tokenExpirationPeriod;
	char buf[30];
	itoa(tokenExpiry,buf,10);
	String tokenExpiryString = buf;

	String sign_key = urlEncode(IoTHub_resourceURI) + "\n" + tokenExpiryString;
	log_v("Sign key:%s\r\n",sign_key.c_str());

	int keyLen = decodeBase64(IoTHub_key, NULL, 0);
	uint8_t* keyBase64 = new uint8_t[keyLen];
	keyLen = decodeBase64(IoTHub_key, keyBase64, keyLen);
	String signature = hashIt(sign_key, keyBase64, keyLen);
	log_v("SignaturekK12:%s\r\n",signature.c_str());
	String result = "SharedAccessSignature sr=" + IoTHub_resourceURI + "&sig=" + signature + "&se=" + tokenExpiryString;
	log_v("Result:%s\r\n",result.c_str());

	//return urlEncode(result);
	return result;
}

SasKeyForIotHub::SasKeyForIotHub()
{
	init();
}

SasKeyForIotHub::SasKeyForIotHub(const String name, const String key, const String devicesId)
{
	this->IoTHub_name = name;
	this->IoTHub_deviceId = devicesId;
	this->IoTHub_key = key;
	init();
}

SasKeyForIotHub::~SasKeyForIotHub()
{
}

String SasKeyForIotHub::getFileUploadURI() {
	return fileUploadRequestURI;
}
String SasKeyForIotHub::getNotificationURI(){
	return notificationURI;
}

