#include "TelemetryJson.h"


TelemetryJsonV2::TelemetryJsonV2() {
	readCnt = -1;
	sentCnt = -2;
	sendingCnt = -3;
	jsonBuf[0] = 0;
}
int TelemetryJsonV2::findByName(const char* val) {
	int result = -1;
	for (int i = 0; i < last_index; i++)
	{
		if (strcmp(val, data[i].GetName()) == 0) return i;
	}
	return result;
}

void TelemetryJsonV2::SetValue(const char* name, int value) {
	int idx = findByName(name);
	if (idx != -1) { 
		data[idx].SetValue(value); 
	}
	else {
		idx = last_index;
		last_index++;
		if (last_index >= maxFields) throw;
		ItemTypeV2& t = data[idx];
		t.SetName(name);
		t.SetValue(value);
	}
	readCnt++;
}

void TelemetryJsonV2::SetValue(const char* name, long value) {
	int idx = findByName(name);
	if (idx != -1) {
		data[idx].SetValue(value);
	}
	else {
		idx = last_index;
		last_index++;
		if (last_index >= maxFields) throw;
		ItemTypeV2& t = data[idx];
		t.SetName(name);
		t.SetValue(value);
	}
	readCnt++;
}
void TelemetryJsonV2::SetValue(const char* name, double value) {
	SetValue(name, (float)value);
}
void TelemetryJsonV2::SetValue(const char* name, float value) {
	int idx = findByName(name);
	if (idx != -1) {
		data[idx].SetValue(value);
	}
	else {
		idx = last_index;
		last_index++;
		if (last_index >= maxFields) throw;
		ItemTypeV2& t = data[idx];
		t.SetName(name);
		t.SetValue(value);
	}
	readCnt++;
}

void TelemetryJsonV2::SetValue(const char* name, const char* value) {
	int idx = findByName(name);
	if (idx != -1) {
		data[idx].SetValue(value);
	}
	else {
		idx = last_index;
		last_index++;
		if (last_index >= maxFields) throw;
		ItemTypeV2& t = data[idx];
		t.SetName(name);
		t.SetValue(value);
	}
	readCnt++;
}

const char * TelemetryJsonV2::GetJson() {
	sendingCnt = readCnt;
	int cnt = 0;
	int curIdx = 0;
	jsonBuf[curIdx] = 0;
	strncat(jsonBuf, "{", jsonBufSize);
	char buf[50];
	for (int i = 0; i < last_index; i++)
	{
		if (cnt++ > 0) {
			strncat(jsonBuf, ",", jsonBufSize);
		}
		ItemTypeV2& it = data[i];
		snprintf(buf, sizeof(buf), "\"%s\":", it.GetName());
		strncat(jsonBuf, buf, jsonBufSize);

		switch (it.GetType()) {
		case eItemTypeV2::eString:
			snprintf(buf, sizeof(buf), "\"%s\"", it.GetString());
			break;
		case eItemTypeV2::eFloat:
			snprintf(buf, sizeof(buf), "%.3f", it.GetFloat());
			break;
		case eItemTypeV2::eInt:
			snprintf(buf, sizeof(buf), "%d", it.GetInt());
			break;
		case eItemTypeV2::eLong:
			snprintf(buf, sizeof(buf), "%ld", it.GetLogn());
			break;
		}
		strncat(jsonBuf, buf, jsonBufSize);
	}
	strncat(jsonBuf, "}", jsonBufSize);
	return jsonBuf;
}

void TelemetryJsonV2::ConfirmSend() {
	sentCnt = sendingCnt;
}
bool TelemetryJsonV2::ShouldSend() {
	return this->readCnt != this->sentCnt;
}