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

/* 
 * File:   code.h
 * Author: ayron
 *
 * Created on 07 May 2016, 19:09
 */

#ifndef CODE_H
#define CODE_H

#ifdef __cplusplus
extern "C" {
#endif

void encoderInit(int mode, int speed, int farnsworth);
void encoderSetMode(int mode);
void encoderSetSpeed(int speed, int farnsworth);
void encoderSetText(const char*ctext);
int getNext();



#ifdef __cplusplus
}
#endif

#endif /* CODE_H */

