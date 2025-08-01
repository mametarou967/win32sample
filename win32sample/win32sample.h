#pragma once

#include "resource.h"

void InitLines();
void CreateButtons(HWND);
void UpdateLayout(HWND);
void DrawContents(HWND);
void ButtonDown(HWND,LPARAM);
void ButtonUp(HWND);
void MouseMove(HWND hWnd,LPARAM);
void CommandExecute(HWND,WPARAM);
void TimerAction();
void DestroyAction(HWND);