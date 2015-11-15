# Smart-Library-System-Design
Smart Library System Design(Micro-processor Part)

This project is a Smart Library System designed to speed up the process of checking out and returning library books by using RFID wireless technology. Developed a chip driver for TRF7960A RFID reader on MSP430 micro-controller based on ISO15693 standard for wireless processing in C++. Designed and implemented library database using MySQL, and connected to the hardware device to update the book data.

trf7960a.c is the driver
iso15693.c is the standard algorithm function
spi.c and uart.c is based on TRF7960A and MSP430G2553