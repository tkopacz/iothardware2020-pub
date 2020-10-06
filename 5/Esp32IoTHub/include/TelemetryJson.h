#pragma once
#ifndef __TelemetryJsonV2__
#define __TelemetryJsonV2__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum class eItemTypeV2 { eEmpty, eInt, eLong, eString, eFloat, eDateTime };

class ItemTypeV2 {
private:
	const int maxNameLen = 30;
	char* name;
	long data_i;
	float data_f;
	char* data_s;
	eItemTypeV2 type;

public:
	ItemTypeV2() {
		type = eItemTypeV2::eEmpty;
		data_i = 0;
		data_f = 0;
		name = data_s = nullptr;
	}
	ItemTypeV2(const char* n, int val) {
		type = eItemTypeV2::eInt;
		data_i = val;
		SetName(n);
	}
	ItemTypeV2(const char* n, long val) {
		type = eItemTypeV2::eLong;
		data_i = val;
		SetName(n);
	}
	ItemTypeV2(const char* n, float val) {
		type = eItemTypeV2::eFloat;
		data_f = val;
		SetName(n);
	}
	ItemTypeV2(const char *n, const char* val) {
		type = eItemTypeV2::eString;
		SetValue(val);
		SetName(n);
	}

	~ItemTypeV2() {
		delete name;
	}
	const char* GetName() { return name; }
	eItemTypeV2 GetType() { return type; }
	int         GetInt() { if (type != eItemTypeV2::eInt) throw; else return data_i; }
	long        GetLogn() { if (type != eItemTypeV2::eLong) throw; else return data_i; }
	float       GetFloat() { if (type != eItemTypeV2::eFloat) throw; else return data_f; }
	const char* GetString() { if (type != eItemTypeV2::eString) throw; else return data_s; }

	void SetName (const char* val) { int l = strlen(val) + 1; if (l > 0 && l < maxNameLen) { name = new char[l]; strncpy(name, val, l); } }
	void SetValue(int val)         { if (type != eItemTypeV2::eInt && type != eItemTypeV2::eEmpty) throw; else { data_i = val; type = eItemTypeV2::eInt; } }
	void SetValue(long val)        { if (type != eItemTypeV2::eLong && type != eItemTypeV2::eEmpty) throw; else { data_i = val; type = eItemTypeV2::eLong; } }
	void SetValue(float val)       { if (type != eItemTypeV2::eFloat && type != eItemTypeV2::eEmpty) throw; else { data_f = val; type = eItemTypeV2::eFloat; } }
	void SetValue(const char* val) { 
		if (type != eItemTypeV2::eString && type != eItemTypeV2::eEmpty) throw; 
		else { 
			int l = strlen(val) + 1;
			if (data_s != nullptr) delete data_s;
			data_s = new char[l];
			strncpy(data_s, val, l);
			type = eItemTypeV2::eString; 
		} 
	}

	bool operator ==(const char* p1) {
		return strcmp(name, p1);
	}
};

class TelemetryJsonV2
{
private:
	static const int maxFields = 20;
	static const int jsonBufSize = 500;


	/// <summary>
	/// Cnt updated during read
	/// </summary>
	long readCnt;
	/// <summary>
	/// Ticks to be sent (need confirmation)
	/// if readCnt == sentCnt - no point in sending
	/// </summary>
	long sentCnt;
	long sendingCnt; //cnt used for sending
	//It is possible to read (add) new values before confirming previous one

	/// <summary>
	/// Main array
	/// </summary>
	ItemTypeV2 data[maxFields]; //Max 20 fields
	int last_index = 0;

	int findByName(const char* val);
	char jsonBuf[jsonBufSize];
public:
	TelemetryJsonV2();
	void SetValue(const char *name, int value);
	void SetValue(const char* name, long value);
	void SetValue(const char* name, float value);
	void SetValue(const char* name, double value);
	void SetValue(const char* name, const char* value);
	const char * GetJson();
	void ConfirmSend();
	bool ShouldSend();
	void Clean();

};

#endif