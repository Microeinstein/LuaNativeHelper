//Native-code helper for Lua 5.3
//by Microeinstein

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#if defined(_MSC_VER)
    //Microsoft 
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

#define length(x)  (sizeof(x) / sizeof((x)[0]))
typedef unsigned long long ulong;
typedef struct {
	double s0, s1, s2, c;
	int first;
} aleaData;

aleaData state;

int zfrs(int n, int s) {
	if (s < 1)
		return n;
	n = n & 0x7FFFFFFF;
	n = n >> s;
	return n;
}
ulong getTick() {
	SYSTEMTIME st;
	FILETIME ft;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
	
	ulong res = (ulong)ft.dwHighDateTime;
	res = res << (sizeof(DWORD) * 8);
	res = res | ft.dwLowDateTime;
	return res;
}

int clearAll() {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD count;
	DWORD cellCount;
	COORD homeCoords = {0, 0};
	
	if (hOut == INVALID_HANDLE_VALUE)
		return 1;
	
	if (!GetConsoleScreenBufferInfo(hOut, &csbi))
		return 2;
	
	cellCount = csbi.dwSize.X * csbi.dwSize.Y;
	
	if (!FillConsoleOutputCharacter(hOut, (TCHAR)' ', cellCount, homeCoords, &count))
		return 3;
	
	if (!FillConsoleOutputAttribute(hOut, csbi.wAttributes, cellCount, homeCoords, &count))
		return 4;
	
	SetConsoleCursorPosition(hOut, homeCoords);
	return 0;
}
int setCursor(int x, int y) {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
	if (hOut == INVALID_HANDLE_VALUE)
		return 1;
	
	COORD pos = {x, y};
	SetConsoleCursorPosition(hOut, pos);
	return 0;
}
int moveCursor(int x, int y) {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	
	if (hOut == INVALID_HANDLE_VALUE)
		return 1;
	
	if (!GetConsoleScreenBufferInfo(hOut, &csbi))
		return 2;
	
	COORD pos = csbi.dwCursorPosition;
	pos.X += x;
	pos.Y += y;
	SetConsoleCursorPosition(hOut, pos);
	return 0;
}

//Porting of the script obtained here: https://ss64.com/nt/syntax-random.html
double mash(const char *data, double **n) {
	if (*n == NULL) {
		*n = malloc(sizeof(double));
		**n = 0xefc8249d;
	}
	double nn = **n;
	for (int i = 0; data[i] != '\0'; i++) {
		nn += (int)data[i];
		double h = 0.02519603282416938 * nn;
		nn = (int)h;
		h -= nn;
		h *= nn;
		nn = (int)h;
		h -= nn;
		nn += h * 0x100000000L;
	}
	**n = nn;
	return ((int)nn) * 2.3283064365386963e-10;
}
aleaData alea(const char *args[]) {
	int len = length(args);
	double s0, s1, s2;
	
	double *n = NULL;
	s0 = mash(" ", &n);
	s1 = mash(" ", &n);
	s2 = mash(" ", &n);
	//printf("%lf\n%lf\n%lf\n\n", s0, s1, s2);
	
	for (int i = 0; i < len; i++) {
		s0 -= mash(args[i], &n);
		if (s0 < 0)
			s0 += 1;
		s1 -= mash(args[i], &n);
		if (s1 < 0)
			s1 += 1;
		s2 -= mash(args[i], &n);
		if (s2 < 0)
			s2 += 1;
	}
	free(n);
	//printf("%lf\n%lf\n%lf\n\n", s0, s1, s2);
	
	aleaData data = {s0, s1, s2, 1, 1};
	return data;
}
double random(aleaData *data) {
	double
		s0 = data->s0,
		s1 = data->s1,
		s2 = data->s2,
		c  = data->c,
	    t;
	int chk;
	do {
		t = 2091639 * s0 + c * 2.3283064365386963e-10;
		s0 = s1;
		s1 = s2;
		c = (int)t;
		s2 = t - c;
		chk = s2 < 1e-7;
	} while (data->first && chk);
	data->s0 = s0;
	data->s1 = s1;
	data->s2 = s2;
	data->c  = c;
	data->first = 0;
	return s2;
}


int l_moveCursor(lua_State *L) {
	int abs = (int)lua_toboolean(L, 1);
	int x = (int)luaL_checknumber(L, 2);
	int y = (int)luaL_checknumber(L, 3);
	int ret;
	if (abs) {
		ret = setCursor(x, y);
	} else {
		ret = moveCursor(x, y);
	}
	if (ret != 0) {
		lua_pushliteral(L, "winTermHelper: Internal error (move cursor)");
		lua_error(L);
	}
	return 0;
}
int l_clearAll(lua_State *L) {
	int ret = clearAll();
	if (ret != 0) {
		lua_pushliteral(L, "winTermHelper: Internal error (clear)");
		lua_error(L);
	}
	return 0;
}
int l_timeTick(lua_State *L) {
	ulong num = getTick();
	lua_pushinteger(L, num);
	return 1;
}
int l_random(lua_State *L) {
	double num = random(&state);
	lua_pushnumber(L, num);
	return 1;
}

int EXPORT libinit(lua_State* L) {
	ulong num = getTick();
	char now[21];
	sprintf(now, "%llu", num);
	char to[15];
	memcpy(to, now, 14);
	to[15] = '\0';
	
	const char *args[1] = { to };
	state = alea(args);
	
	//Ind(ic|ex)es starts at 1... damn lua
	//1 (first element pushed), 2, 3...
	lua_newtable(L);
	
	lua_pushstring(L, "moveCursor");
	lua_pushcfunction(L, l_moveCursor);
	lua_settable(L, 1);
	
	lua_pushstring(L, "clearAll");
	lua_pushcfunction(L, l_clearAll);
	lua_settable(L, 1);
	
	lua_pushstring(L, "timeTick");
	lua_pushcfunction(L, l_timeTick);
	lua_settable(L, 1);
	
	lua_pushstring(L, "random");
	lua_pushcfunction(L, l_random);
	lua_settable(L, 1);
	
	lua_setglobal(L, "nativeHelper");
	//lua_register(L, "wth_moveCursor", l_moveCursor);
	//lua_register(L, "wth_clearAll", l_clearAll);
	return 0;
}