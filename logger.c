#include "logger.h"
#include "syshead.h"

void logger_(int level, const char* funcname, void* format, ...)
{
	char buf[1024] = { 0, };

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	if ((level & LOG_LEVEL) == 0) {
		return ;
	}

	if (level == 1) {
		sprintf(buf, "%d-%d-%d %d:%d:%d [INFO] [FUNC: %s] ", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, funcname);
	}
	else if (level == 2) {
		sprintf(buf, "%d-%d-%d %d:%d:%d [DEBUG] [FUNC: %s] ", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,  funcname);
	}
	else if (level == 4) {
		if (strstr(format, "%s") != NULL) {
			sprintf(buf, "%d-%d-%d %d:%d:%d [KEY] [FUNC: %s] ", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, funcname);
		}
	}

	if (level == 4) {
		if (strstr(format, "%s") == NULL) {
			dump_hex(format, strlen(format));
			return ;
		}
	}

	va_list va;
	va_start(va, format);
	vsprintf(buf + strlen(buf), format, va);
	va_end(va);
	// openlog("Keystore", LOG_PID, LOG_LOCAL5);
	// syslog(LOG_INFO,"%s", buf);
	// closelog();
	puts(buf);
}


void dump_hex(const void* data, int size) {
	char ascii[17];
	int i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		printf("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		}
		else {
			ascii[i % 16] = '.';
		}
		if ((i + 1) % 8 == 0 || i + 1 == size) {
			printf(" ");
			if ((i + 1) % 16 == 0) {
				printf("|  %s \n", ascii);
			}
			else if (i + 1 == size) {
				ascii[(i + 1) % 16] = '\0';
				if ((i + 1) % 16 <= 8) {
					printf(" ");
				}
				for (j = (i + 1) % 16; j < 16; ++j) {
					printf("   ");
				}
				printf("|  %s \n", ascii);
			}
		}
	}
}
