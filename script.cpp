/*
  SHOW COORDS XYZ AND ANGLE
  Author: FunGt
  Author angle: PlasticTangerine
  Author ScriptHookV: Alexander Blade
*/

#include "script.h"
#include <fstream>
#include <windows.h>
#include <atlstr.h>
#include <iostream>
#include <chrono> 
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

#pragma warning(disable : 4244 4305) // double <-> float conversions

void draw_text(char* text, float x, float y, float scale) {
	UI::SET_TEXT_FONT(0);
	UI::SET_TEXT_SCALE(scale, scale);
	UI::SET_TEXT_COLOUR(255, 255, 255, 245);
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(0);
	UI::SET_TEXT_DROPSHADOW(2, 2, 0, 0, 0);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(text);
	UI::_DRAW_TEXT(y, x);
}

//bool WriteComPort(CString PortSpecifier, CString data)
//{
	//DCB dcb;
	//DWORD byteswritten;
	//HANDLE hPort = CreateFile(

	//	PortSpecifier,
	//	GENERIC_WRITE,
	//	0,
	//	NULL,
	//	OPEN_EXISTING,
	//	0,
	//	NULL
	//);
	//if (!GetCommState(hPort, &dcb))
	//	return false;
	//dcb.BaudRate = CBR_9600; //9600 Baud
	//dcb.ByteSize = 8; //8 data bits
	//dcb.Parity = NOPARITY; //no parity
	//dcb.StopBits = ONESTOPBIT; //1 stop
	//if (!SetCommState(hPort, &dcb))
	//	return false;
	//bool retVal = WriteFile(hPort, data, 1, &byteswritten, NULL);
//	CloseHandle(hPort); //close the handle
//	return retVal;
//}

float distance(float x1, float y1, float z1, float x2, float y2, float z2)
{
	float d = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2) * 1.0);

	return d;
}

constexpr double kCheckComPortInterval = 10.0;
constexpr double kSendInterval = 3.0;
//bool connectToComDevice(DCB &dcb, HANDLE &hPort, DWORD &byteswritten) {
//	 hPort = CreateFile(
//
//		"COM3",
//		GENERIC_ALL/*GENERIC_WRITE*/,
//		0,
//		NULL,
//		OPEN_EXISTING,
//		0,
//		NULL
//	);
//	bool comm_state = GetCommState(hPort, &dcb);
//	if (comm_state == 0) {
//		return false;
//	}
//	dcb.BaudRate = CBR_115200; //9600 Baud
//	dcb.ByteSize = 8; //8 data bits
//	dcb.Parity = NOPARITY; //no parity
//	dcb.StopBits = ONESTOPBIT; //1 stop
//
//	return SetCommState(hPort, &dcb) != 0;
//}
void main() {
	bool widescreen = GRAPHICS::GET_IS_WIDESCREEN();
	Ped playerPed;
	Vector3 position;
	std::ofstream file;
	std::string coords_str;
	float d, angleToDestination;
	bool arrived = false, pressed = false, notpressed = true;
	unsigned char anglearea = 0;
	byte output[2] = { 0,0 }, last_output[2] = { 0,0 };
	float x = 668.738, y = 497.866;
	int endpoint = 0;


	DCB dcb;
	DWORD byteswritten = 0;
	HANDLE hPort;
	bool com_is_connected = false;
	//HANDLE hPort = CreateFile(

	//	"COM3",
	//	GENERIC_ALL/*GENERIC_WRITE*/,
	//	0,
	//	NULL,
	//	OPEN_EXISTING,
	//	0,
	//	NULL
	//);
	//bool comm_state = GetCommState(hPort, &dcb);

	//dcb.BaudRate = CBR_115200; //9600 Baud
	//dcb.ByteSize = 8; //8 data bits
	//dcb.Parity = NOPARITY; //no parity
	//dcb.StopBits = ONESTOPBIT; //1 stop
	//bool set_comm_state= 
	auto com_timer_start = std::chrono::high_resolution_clock::now(), send_timer_start = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> com_timer_elapsed, send_timer_elapsed;


	while (true) {
		playerPed = PLAYER::PLAYER_PED_ID();
		position = ENTITY::GET_ENTITY_COORDS(playerPed, 1);
		widescreen = GRAPHICS::GET_IS_WIDESCREEN();

		char coords[100], text[300], message[100];


		com_timer_elapsed = std::chrono::high_resolution_clock::now() - com_timer_start;
		double wait = 5.000;


		if (!com_is_connected) {

			hPort = CreateFile(
				"COM3",
				GENERIC_WRITE/*GENERIC_WRITE*/,
				0,
				NULL,
				OPEN_EXISTING,
				0,
				NULL
			);
			if (GetCommState(hPort, &dcb) == 0) {
				sprintf_s(text, "Error %d when GetComState", GetLastError());
				widescreen ? draw_text(text, 0.100, 0.205 - 0.03, 0.3) : draw_text(text, 0.938, 0.205, 0.3);
			}
			else {
				dcb.BaudRate = CBR_9600; //9600 Baud
				dcb.ByteSize = 8; //8 data bits
				dcb.Parity = NOPARITY; //no parity
				dcb.StopBits = ONESTOPBIT; //1 stop
				if (SetCommState(hPort, &dcb) == 0) {
					sprintf_s(text, "Error %d when SetCommState", GetLastError());
					widescreen ? draw_text(text, 0.200, 0.205 - 0.03, 0.3) : draw_text(text, 0.938, 0.205, 0.3);
				}
				else {
					sprintf_s(text, "COM Device connected");
					widescreen ? draw_text(text, 0.300, 0.205 - 0.03, 0.3) : draw_text(text, 0.938, 0.205, 0.3);
					com_is_connected = true;
				}
			}
			//if (connectToComDevice(dcb, hPort, byteswritten)) {
			// 				com_is_connected = true;
			//}
			//else {
			//	sprintf_s(message, "Error %d for init com state", GetLastError());
			//	widescreen ? draw_text(message, 0.500, 0.205 - 0.03, 0.3) : draw_text(message, 0.938, 0.205, 0.3);
			//}
		}
		else if (com_timer_elapsed.count() > kCheckComPortInterval) {
			if ( (hPort, &dcb) == 0) {
				sprintf_s(text, "Lost connection to com, error %d for init com state", GetLastError());
				widescreen ? draw_text(text, 0.400, 0.205 - 0.03, 0.3) : draw_text(text, 0.938, 0.205, 0.3);
				com_is_connected = false;
			}
			com_timer_start = std::chrono::high_resolution_clock::now();

		}

		/*file.open("coords.txt", std::ios_base::out);
		file << "x: " << position.x << " y: " << position.y  << " z: " << position.z << " angel: " << ENTITY::GET_ENTITY_HEADING(playerPed) << std::endl;
		file.close();*/


		sprintf_s(message, "COM: %d, Output: %d %d, Last: %d %d", com_is_connected, output[0], output[1], last_output[0],last_output[1]);
		widescreen ? draw_text(message, 0.500, 0.205 - 0.03, 0.3) : draw_text(message, 0.878, 0.205, 0.3);
		if (!KEY_DOWN(0x60)) pressed = false;

		if (KEY_DOWN(0x60) && pressed == false)
		{
			pressed = true;
			if (endpoint <= 5)  endpoint++;
			else endpoint = 0;

			if (endpoint == 1) { x = -128.54142835749522; y = 73.3094869941036; }
			else if (endpoint == 2) { x = -124.00551584233719; y = 78.90096987281086; }
			else if (endpoint == 3) { x = -104.68558033973733; y = 79.48212659567514; }
			else if (endpoint == 4) { x = -99.85290821227224; y = 74.54176267742784; }
			else if (endpoint == 5) { x = 668.738; y = 497.866; }
			widescreen ? draw_text("Taste NUMPAD gedrückt", 0.600, 0.205 - 0.03, 0.3) : draw_text("Taste NUMPAD gedrückt", 0.918, 0.205, 0.3);

		}

		//sprintf_s(message, "Neues Ziel: X: %.3f Y: %.3f", x, y);
		//widescreen ? draw_text(message, 0.938, 0.205 - 0.03, 0.3) : draw_text(message, 0.938, 0.205, 0.3);

		d = distance(position.x, position.y, position.z, x, y, position.z);
		arrived = d < 100;

		angleToDestination = acos(abs(position.x - x) / abs(position.y - y));

		//if (angleToDestination > 0 && angleToDestination < 15) anglearea = 0;
		//else if (angleToDestination >= 15 && angleToDestination < 45) anglearea = 1;
		//else if (angleToDestination >= 45 && angleToDestination < 90) anglearea = 2;
		//else if (angleToDestination >= 90 && angleToDestination < 135) anglearea = 3;
		//else if (angleToDestination >= 135 && angleToDestination < 180) anglearea = 4;
		//else if (angleToDestination >= 180 && angleToDestination < 225) anglearea = 4;
		//else if (angleToDestination >= 225 && angleToDestination < 270) anglearea = 5;
		//else if (angleToDestination >= 270 && angleToDestination < 315) anglearea = 6;
		//else if (angleToDestination >= 315 && angleToDestination < 345) anglearea = 7;
		//else if (angleToDestination >= 345 && angleToDestination <= 360) anglearea = 0;

		//output = anglearea  | (arrived << 4);
		uint16_t angle = (unsigned int)ENTITY::GET_ENTITY_HEADING(playerPed);
		output[1] = angle;
		output[0] = (angle >> 8 & 0x01);
		/*output[0] = angle;
		output[1] = (angle & 0xFE) >> 8;*/

		if (d < 100) {
			widescreen ? draw_text("Ziel erreicht", 0.500, 0.205 - 0.03, 0.8) : draw_text("Ziel erreicht", 0.978, 0.205, 0.8);
		}


		sprintf_s(text, "[0]%d [1]%d ", output[0], output[1]);
		widescreen ? draw_text(text, 0.938, 0.205 - 0.03, 0.3) : draw_text(text, 0.978, 0.205, 0.3);

		sprintf_s(text, "Arrvied: %u Angel: %u", (unsigned int)arrived, (unsigned int)ENTITY::GET_ENTITY_HEADING(playerPed));
		widescreen ? draw_text(text, 0.958, 0.205 - 0.03, 0.3) : draw_text(text, 0.958, 0.205, 0.3);

		sprintf_s(text, "X: %.3f Y: %.3f Z: %.3ff", position.x, position.y, position.z);
		widescreen ? draw_text(text, 0.978, 0.205 - 0.03, 0.3) : draw_text(text, 0.978, 0.205, 0.3);
		
		sprintf_s(text, "com: %f/%f, send: %f/%f", com_timer_elapsed.count(), kCheckComPortInterval, send_timer_elapsed.count(), kSendInterval);
		widescreen ? draw_text(text,  0.700, 0.205 - 0.03, 0.3) : draw_text(text, 0.978, 0.205, 0.3);



		send_timer_elapsed= std::chrono::high_resolution_clock::now() - send_timer_start;
		if (output[0] != last_output[0] || output[1] != last_output[1] && send_timer_elapsed.count() > kSendInterval) {
			//coords_str = output; 

			bool x = WriteFile(hPort, output, 2, &byteswritten, NULL);
			sprintf_s(text, "Written %d (%d Bytes)", x, byteswritten);
			widescreen ? draw_text(text, 0.900, 0.205 - 0.03, 0.3) : draw_text(text, 0.978, 0.205, 0.3);
			//WriteFile(hPort, coords_str.c_str(), coords_str.size(), &byteswritten, NULL);
			/*WriteFile(hPort, coords_str.c_str(), 1, &byteswritten, NULL)*/;
			send_timer_start = std::chrono::high_resolution_clock::now();
			last_output[0] = output[0];
			last_output[1] = output[1];
			if (x == 0) {
				com_is_connected = false;
			}
		};
		WAIT(0);

	}
	CloseHandle(hPort); //close the handle

}

void ScriptMain() {
	srand(GetTickCount64());
	main();
}
