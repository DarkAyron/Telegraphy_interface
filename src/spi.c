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

void spiSelect(uint8_t cs)
{
	SPI0->SPI_MR = SPI_MR_MSTR | SPI_MR_PCS(cs) | SPI_MR_MODFDIS;
}

void spiDeselect()
{
	SPI0->SPI_MR = SPI_MR_MSTR | SPI_MR_PCS(0x0f) | SPI_MR_MODFDIS;
}

uint8_t spiReadWrite(uint8_t data)
{
	/*SPI0->SPI_TDR = SPI_TDR_TD(data) | SPI_TDR_PCS(0);
	while (!(SPI0->SPI_SR & SPI_SR_RDRF));
	return SPI0->SPI_RDR;*/
	while(!(USART1->US_CSR & US_CSR_TXRDY));
	USART1->US_THR = data;
	while (!(USART1->US_CSR & US_CSR_TXEMPTY));
	return USART1->US_RHR & 0xff;
}
