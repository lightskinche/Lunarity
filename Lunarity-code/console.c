#include "main_h.h"

void CONSOLE_ThrowWarning(unsigned int flags, const char* message, const char* fname) {
	char* color = "\033[33;1m", *message_color = "\033[97;0m";
	if (flags & WARNING_EXTREME)
		color = "\033[33;41;1m", message_color = color;
	if (flags & WARNING_SERVERE)
		message_color = color;

	printf("%sWARNING, \033[34;1min file %s: %s%s\n\033[97;0m", color, fname, message_color, message);

}