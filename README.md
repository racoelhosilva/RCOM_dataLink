<h1 align="center">RCOM - Data Link</h1>

The objective of this project is to implement a data link protocol for communication between two Linux computers, connected through a serial port, along with a basic application protocol for file transfer between the two computers.

The project involved the use of Linux drivers for sending bits through the serial port, which the Data Link Layer used for establishing a reliable communication, using a basic XOR based error verification and Stop and Wait ARQ. Then, the Application Layer used the Data Link Layer to send files between the two computers.

It was also an objective to measure statistics about the protocol, such as the number of retransmissions and the protocol efficiency, for varying frame sizes and Bit Error Ratios, which are all documented on the report.

---

> Class: 3LEIC01 Group: 7  
> Final Grade: 20.0  
> Professors: Hélder Fontes  
> Created in October 2024 for RCOM (Redes de Computadores)  