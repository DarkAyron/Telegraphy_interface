/*
 * Copyright (C) 2016 ayron
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cpu.h"
#include <string.h>
#include "telegraph.h"

const char*alpha = "abcdefghijklmnopqrstuvwxyz1234567890.,!?=-_:;'\"/()$&@~+# ";
const char*internationalMorse[] = {
	/* abcdefghijklmnopqrstuvwxyz1234567890.,!?=-_:;'"/()$&@~+ */
	".-",
	"-...",
	"-.-.",
	"-..",
	".",
	"..-.",
	"--.",
	"....",
	"..",
	".---",
	"-.-",
	".-..",
	"--",
	"-.",
	"---",
	".--.",
	"--.-",
	".-.",
	"...",
	"-",
	"..-",
	"...-",
	".--",
	"-..-",
	"-.--",
	"--..",
	".----",
	"..---",
	"...--",
	"....-",
	".....",
	"-....",
	"--...",
	"---..",
	"----.",
	"-----",
	".-.-.-",
	"--..--",
	"-.-.--",
	"..--..",
	"-...-",
	"-....-",
	"..--.-",
	"---...",
	"-.-.-.",
	".----.",
	".-..-.",
	"-..-.",
	"-.--.",
	"-.--.-",
	"...-..-",
	".-...",
	".--.-.",
	"-.-.-",
	".-.-.",
	"#",
	"  "
};

const char*railRoadMorse[] = {
	/* abcdefghijklmnopqrstuvwxyz1234567890.,!?=-_:;'"/()$&@~+ */
	"./",
	"/...",
	".. .",
	"/..",
	".",
	"./.",
	"//.",
	"....",
	"..",
	"/./.",
	"/./",
	"_",
	"//",
	"/.",
	". .",
	".....",
	"../.",
	". ..",
	"...",
	"/",
	"../",
	".../",
	".//",
	"./..",
	".. ..",
	"... .",
	".//.",
	"../..",
	".../.",
	"..../",
	"///",
	"......",
	"//..",
	"/....",
	"/../",
	"\\",
	"..//..",
	"././",
	"///.",
	"/../.",
	"  ",
	".... ./..",
	"  ",
	"/./ . .",
	"... ..",
	"../. ./..",
	"../. /.",
	"../ /",
	"..... /.",
	"..... .. ..",
	"  ",
	". ...",
	"  ",
	"  ",
	"  ",
	"#",
	"  "
};

static int charPos;
static const char*actChar;
static const char*text;
static int morseMode;
static int farnsworthDots;
static int remainingFarnsworthDots;

static const char*encodeChar(char c)
{
	int n;
	for (n = 0; n < strlen(alpha); n++) {
		if (alpha[n] == c) {
			if (morseMode == MORSE_CODE_INTERNATIONAL)
				return internationalMorse[n];
			else
				return railRoadMorse[n];
		}
	}
	
	return 0;
}

void encoderInit(int mode, int speed, int farnsworth)
{
	int rc;
	morseMode = mode;
	farnsworthDots = farnsworth;

	rc = (1200 / speed) * (SystemCoreClock / 2000);
	TC_SetRC(TC0, 0, rc);
}

void encoderSetMode(int mode)
{
	morseMode = mode;
}

void encoderSetSpeed(int speed, int farnsworth)
{
	encoderInit(morseMode, speed, farnsworth);
}

void encoderSetText(const char*ctext)
{
	text = ctext;
	actChar = encodeChar(*text);
	charPos = 0;
	remainingFarnsworthDots = 0;
}

int getNext()
{
	char c;
	
	if (remainingFarnsworthDots) {
		remainingFarnsworthDots--;
		return 3;
	}

	c = *actChar++;
	if (!c) {
		if (!text[1])
			return 0; /* done */
		actChar = encodeChar(*++text);
		remainingFarnsworthDots = farnsworthDots;
		if (*text == ' ')
			remainingFarnsworthDots *= 2;
		return 4; /* pause (dash) */
	}
	
	switch (c) {
	case '.':
		return 1; /* dot */
	case '-':
		return 2; /* dash */
	case ' ':
		return 3; /* pause (dot) */
	case '#':
		return 5; /* long pause */
	case '/':
		return 6; /* short dash */
	case '_':
		return 7; /* longer dash (l) */
	case '\\':
		return 8; /* very long dash (0) */
	}
	return -1;
}
