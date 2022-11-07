#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <3ds.h>
#include <curl/curl.h>
#include <sys/socket.h>

#define SOC_ALIGN       0x1000
#define SOC_BUFFERSIZE  0x100000

static u32 *SOC_buffer = NULL;

PrintConsole top_screen, bottom_screen;

size_t write_data(char *buffer, size_t item_size, size_t item_amount)
{
	size_t bytes = item_size * item_amount;
	int buffer_len = strlen(buffer);
	int pos = 50 - buffer_len;
	pos = pos / 2;
	pos += 1;

	consoleSelect(&top_screen);
	printf("\x1b[15;%dH%s\n", pos, buffer);

	consoleSelect(&bottom_screen);
	printf("\x1b[30;10HPress START to exit.\n");
	return bytes;
}

int main(int argc, char *argv[])
{
	gfxInitDefault();
	atexit(gfxExit);
	consoleInit(GFX_TOP, &top_screen);
	consoleInit(GFX_BOTTOM, &bottom_screen);
	int ret;

	SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);

	if(SOC_buffer == NULL) {
		return 1;
	}

	if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
		return 1;
	}

	CURL *curl = curl_easy_init();
	if (!curl)
	{
		return 1;
	}
	curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/zen");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Zen3DS");
	CURLcode result = curl_easy_perform(curl);
	if (result != CURLE_OK)
	{
		printf(curl_easy_strerror(result), "\n");
	}

	curl_easy_cleanup(curl);

	atexit(socExit);

	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break;
	}

	return 0;
}
