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
 * File:   spi.h
 * Author: ayron
 *
 * Created on 21 August 2016, 13:07
 */

#ifndef SPI_H
#define SPI_H

/*#define SPI_MISO	PIO_PA25
#define SPI_MOSI	PIO_PA26
#define SPI_SCK		PIO_PA27
*/
#define SPI_MISO	PIO_PA12
#define SPI_MOSI	PIO_PA13
#define SPI_SCK		PIO_PA16
#define SPI_CS		PIO_PA28

uint8_t spiReadWrite(uint8_t data);
#define SPI_SELECT_ENC	PIOA->PIO_CODR = SPI_CS
#define SPI_DESELECT	PIOA->PIO_SODR = SPI_CS

#endif /* SPI_H */

