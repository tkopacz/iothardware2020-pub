#include "IoTBlobHelper.h"
#include <ArduinoJson.h>

IoTBlobHelper::IoTBlobHelper()
{
    m_sas = SasKeyForIotHub();
    m_clientIoT.setCACert(certificates);
    m_clientBlob.setCACert(certificates);
}

IoTBlobHelper::IoTBlobHelper(const String name, const String key, const String devicesId)
{
    m_sas = SasKeyForIotHub(name, key, devicesId);
    m_clientIoT.setCACert(certificates);
    m_clientBlob.setCACert(certificates);
}

int IoTBlobHelper::UploadBlob(const String blobName, uint8_t *data, size_t size, const String contentType)
{
    String cid;
    String strSas;
    String uploadURL;
    prep(blobName,cid,strSas,uploadURL);
    HTTPClient https;
    https.begin(m_clientBlob, uploadURL.c_str());
    https.addHeader("x-ms-blob-type", "BlockBlob");
    
    if (contentType!=NULL) https.addHeader("Content-Type", contentType.c_str()); //"application/json"

    int httpCode = https.PUT(data, size);
    https.end();
    log_v("UploadBlob, HTTP: %d\r\n",httpCode);
    notification(cid, strSas);
    return 0;
}

int IoTBlobHelper::UploadBlob(const String blobName, String data)
{
    return UploadBlob(blobName,(uint8_t *) data.c_str(), data.length(),"application/json");
}

void IoTBlobHelper::prep(String blobName,String &cid, String &strSas, String &uploadURL) {
    strSas = m_sas.get_iot_hub_sas_token();
    log_v("SAS:\r\n%s\r\n",strSas.c_str());
    HTTPClient https;
    https.begin(m_clientIoT, m_sas.getFileUploadURI().c_str());
    https.addHeader("Authorization", strSas.c_str());
    https.addHeader("Content-Type", "application/json");
    String postBody = "{ \"blobName\": \"" + blobName + "\"}";
    int httpCode = https.POST(postBody);
    log_v("Body:\r\n%s\r\nRES:\r\n%d\r\n",postBody.c_str(),httpCode);
    String resp = https.getString();
    log_v("Resp:\r\n%s\r\n",resp.c_str());
    https.end();
    StaticJsonDocument<500> doc;
    deserializeJson(doc, resp);
    String hn = doc["hostName"];
    String cn = doc["containerName"];
    String bn = doc["blobName"];
    String sas = doc["sasToken"];
    cid = doc["correlationId"].as<String>();
    uploadURL = "https://" + hn + "/" + cn + "/" + bn + sas;
    log_v("\r\n---------------\r\n%s\r\n-----------------\r\n",uploadURL.c_str());
}

void IoTBlobHelper::notification(String cid, String strSas)
{
    HTTPClient https;
    https.begin(m_clientIoT, m_sas.getNotificationURI().c_str());
    https.addHeader("Authorization", strSas.c_str());
    https.addHeader("Content-Type", "application/json");
    int httpCode = https.POST("{ \"correlationId\": \"" + cid + "\"}");
    https.end();
    log_v("\r\nNOTIFICATION, %d\r\n",httpCode);
}