/*


Weather Text by ELY M.  



*/



#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include <switch.h>
#include "download.h"
#include "jsmn.h"

#define HALF_SCREEN 40
struct V {
  short int half;
  short int half_length;
  short int final_length;
} var;



	
	
const char* const months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
const char* const weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


void title(char *str) {
	var.half = strlen (str) / 2;
	var.half_length = var.half;
	var.final_length = HALF_SCREEN;

	for (; var.half <= var.half_length; var.half--) {
		var.final_length--;
		if (var.half <= 0) break;
	}

	printf ("\x1b[3;%dH%s%s%s", var.final_length, CONSOLE_CYAN, str, CONSOLE_RESET);
	return;
}



void footer(char *str) {
	printf ("\x1b[43;0H%s%s%s", CONSOLE_CYAN, str, CONSOLE_RESET);
	return;
}



char	*popKeyboard(char *message, size_t size)
{
	SwkbdConfig	skp; // Software Keyboard Pointer
	Result		rc = swkbdCreate(&skp, 0);
	char		*tmpout = NULL;

	// +1 for the '\0'
	tmpout = (char *)calloc(sizeof(char), size + 1);
	if (tmpout == NULL)
		return (NULL);

	if (R_SUCCEEDED(rc)) {
		swkbdConfigMakePresetDefault(&skp);
		swkbdConfigSetGuideText(&skp, message);
		rc = swkbdShow(&skp, tmpout, size);
		swkbdClose(&skp);
	} else {
		free(tmpout);
		tmpout = NULL;
	}

	return (tmpout);
}


char * removeSpaces(char *string)
{
    // non_space_count to keep the frequency of non space characters
    int non_space_count = 0;
 
    //Traverse a string and if it is non space character then, place it at index non_space_count
    for (int i = 0; string[i] != '\0'; i++)
    {
        if (string[i] != ' ')
        {
            string[non_space_count] = string[i];
            non_space_count++;//non_space_count incremented
        }   
		
    }
    
    //Finally placing final character at the string end
    string[non_space_count] = '\0';
    return string;
}

static bool	setMyGPS(void)
{
	FILE		*fp = NULL;
	char		*tmpout = NULL;
	bool		err = false;

	tmpout = popKeyboard("set your GPS like this 39.232,-93.75", 256);

	if (tmpout != NULL) {
		if (*tmpout == 0) {
			err = true;
		} else {
			if ((fp = fopen("sdmc:/switch/weather-gps.txt", "wb")) != NULL) {
				fprintf(fp, "%s", tmpout);
				fclose(fp);
			} else {
				err = true;
			}
		}
		
		//printf("%s My GPS: %s \n%s", CONSOLE_CYAN, tmpout, CONSOLE_RESET);

		free(tmpout);
		tmpout = NULL;

	} else {
		err = true;
	}

	return (err);
}


char *readMyGPS(void)
{

	FILE		*fp = NULL;
	char		*buffer = NULL;
	size_t		nbytes = 0;
	struct stat	st;

	// if tmpfile is empty, return true and pop the keyboard.
	stat("sdmc:/switch/weather-gps.txt", &st);
	if (st.st_size == 0) {
		setMyGPS();
	}

	// if calloc fail, pop the keyboard
	buffer = (char *)calloc(sizeof(char), st.st_size);
	if (buffer == NULL) {
		setMyGPS();
	}

	
 
	fp = fopen("sdmc:/switch/weather-gps.txt", "r");
	if (fp != NULL) {
		nbytes = fread(buffer, sizeof(char), st.st_size, fp);
		if (nbytes > 0) {
			//mygps = buffer;
			//remove spaces
			//printf("readMyGPS(): trying to remove spaces\n");
			buffer = removeSpaces(buffer);
			strtok(buffer, "\n");
		}
		fclose(fp);
	}
	

	printf("%s My GPS: %s \n%s", CONSOLE_CYAN, buffer, CONSOLE_RESET);
	// free memory
	// free(buffer);
	// buffer = NULL;


	return (buffer);
}



static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}



void *getjson(char *JsonString) {
	int i;
	int r;
	jsmn_parser p;
	jsmntok_t t[256]; /* We expect no more than 128 tokens */
	
	char temp[256];
	char weather[256]; 
	char weatherimage[256];  
	char location[256]; 
	
	char string[256];
	 

	jsmn_init(&p);
	r = jsmn_parse(&p, JsonString, strlen(JsonString), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		printf("Failed to parse JSON: %d\n", r);
		return 0;
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\n");
		return 0;
	}


	
	for (i = 1; i < r; i++) {
		if (jsoneq(JsonString, &t[i], "Temp") == 0) {
			//printf("Temp: %.*s\n", t[i+1].end-t[i+1].start, JsonString + t[i+1].start);
			snprintf(temp, sizeof(temp)+1, "%.*s", t[i+1].end-t[i+1].start, JsonString + t[i+1].start); 
			i++;
		} 
				
		else if (jsoneq(JsonString, &t[i], "Weather") == 0) {
			//printf("Weather: %.*s\n", t[i+1].end-t[i+1].start, JsonString + t[i+1].start);
			snprintf(weather, sizeof(weather)+1, "%.*s", t[i+1].end-t[i+1].start, JsonString + t[i+1].start);
			i++;
		} else if (jsoneq(JsonString, &t[i], "Weatherimage") == 0) {
			//printf("Weatherimage: %.*s\n", t[i+1].end-t[i+1].start, JsonString + t[i+1].start);
			snprintf(weatherimage, sizeof(weatherimage)+1, "%.*s", t[i+1].end-t[i+1].start, JsonString + t[i+1].start);
			i++;
		} 
		
		
		
		
		
		else if (jsoneq(JsonString, &t[i], "name") == 0) {
			//printf("Location: %.*s\n", t[i+1].end-t[i+1].start, JsonString + t[i+1].start);
			snprintf(location, sizeof(location)+1, "%.*s", t[i+1].end-t[i+1].start, JsonString + t[i+1].start);  
			i++;
		} 
		
		
		
		snprintf(string, sizeof(string)+1, "\n\n\n\nTemp: %s\nWeather: %s\nLocation: %s\n", temp, weather, location); 
	
		
		
		/*else if (jsoneq(JsonString, &t[i], "groups") == 0) {
			int j;
			printf("- Groups:\n");
			if (t[i+1].type != JSMN_ARRAY) {
				continue; // We expect groups to be an array of strings
			}
			for (j = 0; j < t[i+1].size; j++) {
				jsmntok_t *g = &t[i+j+2];
				printf("  * %.*s\n", g->end - g->start, JsonString + g->start);
			}
			i += t[i+1].size + 1;
		}  else {
			printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
					JsonString + t[i].start);
		}
		*/
		
		
	}
	
	
	printf("%s%s%s", CONSOLE_CYAN, string, CONSOLE_RESET);  
		
	return JsonString;
}


char *readWeather(void)
{

	FILE		*fp = NULL;
	char		*buffer = NULL;
	size_t		nbytes = 0;
	struct stat	st;
	

	// if tmpfile is empty, return true and pop the keyboard.
	stat("sdmc:/switch/weather.txt", &st);
	if (st.st_size == 0) {
		return 0;
	}

	// if calloc fail, pop the keyboard
	buffer = (char *)calloc(sizeof(char), st.st_size);
	if (buffer == NULL) {
		return 0;
	}

	
 
	fp = fopen("sdmc:/switch/weather.txt", "r");
	if (fp != NULL) {
		nbytes = fread(buffer, sizeof(char), st.st_size, fp);
		if (nbytes > 0) {
			//mygps = buffer;
		}
		fclose(fp);
	}
	
	//parse the json file//  
	getjson(buffer);
	

	
	//printf("%s Weather: %s\n%s", CONSOLE_CYAN, buffer, CONSOLE_RESET);
	// free memory
	// free memory
	// free(buffer);
	// buffer = NULL;


	return (buffer);
}




int main(int argc, char **argv)
{
	
	
	char *mygps = "";
	char *lon = "";
   	char *lat = "";
	consoleInit(NULL);
	curlInit();
 
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);
	PadState pad;
	padInitializeDefault(&pad);
	
	
	
	// Main loop
	while(appletMainLoop())
	{
		
		
		//Print current time
		time_t unixTime = time(NULL);
		//struct tm* timeStruct = gmtime((const time_t *)&unixTime); //GMT time
        struct tm* timeStruct = localtime((const time_t *)&unixTime); //localtime
		int hours = timeStruct->tm_hour;
		int minutes = timeStruct->tm_min;
		int seconds = timeStruct->tm_sec;
		int day = timeStruct->tm_mday;
		int month = timeStruct->tm_mon;
		int year = timeStruct->tm_year +1900;
		int wday = timeStruct->tm_wday;
		//printf("%s %s %i %i  %02i:%02i:%02i", weekDays[wday], months[month], day, year, hours, minutes, seconds);
		
		printf("\x1b[1;1H%s%s %s %i %i %02i:%02i:%02i", CONSOLE_CYAN, weekDays[wday], months[month], day, year, hours, minutes, seconds);
		//printf("%s%s %s %i %i %02i:%02i:%02i\n%s", CONSOLE_CYAN, weekDays[wday], months[month], day, year, hours, minutes, seconds, CONSOLE_RESET);
		
		mygps = readMyGPS();
		//split gps 
	    lat = strtok(mygps, ",");
   	    lon = strtok(NULL, ",");
	
		
		title("Weather Text by ELY M.\n");
		
		
		//Scan all the inputs. This should be done once for each frame
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

		if (kDown & HidNpadButton_Plus)  { 
		//exit to hbmenu
		break; 
		}
		
		
		if (kDown & HidNpadButton_Minus)  { 
		setMyGPS();	
		}


		if (kDown & HidNpadButton_A)  { 
		//printf("\n\nA pressed\n\nlat: %s lon: %s\n\n", lat, lon);
		FILE_TRANSFER_HTTP(lat, lon); 
		//sleep(1); 
		//readWeather(); 
		}
	
		
		if (kDown & HidNpadButton_X)  { 
		FILE_TRANSFER_HTTP(lat, lon); 
		sleep(1);
		readWeather(); 

		}		
		


		
		footer("Press - for GPS Setting | Press A or X for Weather Info | Press + to exit");
		consoleUpdate(NULL);		
		
		
	}
	

	
	curlExit();
	consoleExit(NULL);
	return 0;
	
}

