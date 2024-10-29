# Makefile to build the project
# NOTE: This file must not be changed.

# Parameters
CC = gcc
CFLAGS = -Wall
# TODO: Remove sanitizers

SRC = src/
INCLUDE = include/
BIN = bin/
CABLE_DIR = cable/

TX_SERIAL_PORT = /dev/ttyS10
RX_SERIAL_PORT = /dev/ttyS11

BAUD_RATE = 9600

TX_FILE = penguin.gif
RX_FILE = penguin-received.gif

# Targets
.PHONY: all
all: $(BIN)/main $(BIN)/ $(BIN)/cable

$(BIN)/main: main.c $(SRC)/*.c
	$(CC) $(CFLAGS) -o $@ $^ -I$(INCLUDE)

$(BIN)/main-dbg: main.c $(SRC)/*.c
	$(CC) $(CFLAGS) -DDEBUG -o $@ $^ -I$(INCLUDE)

$(BIN)/main-stats: main.c $(SRC)/*.c
	$(CC) $(CFLAGS) -DSTATISTICS=1 -o $@ $^ -I$(INCLUDE)

$(BIN)/cable: $(CABLE_DIR)/cable.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: run_tx
run_tx: $(BIN)/main
	./$(BIN)/main $(TX_SERIAL_PORT) $(BAUD_RATE) tx $(TX_FILE)

.PHONY: run_rx
run_rx: $(BIN)/main
	./$(BIN)/main $(RX_SERIAL_PORT) $(BAUD_RATE) rx $(RX_FILE)

.PHONY: run_tx_dbg
run_tx_dbg: $(BIN)/main-dbg
	./$(BIN)/main-dbg $(TX_SERIAL_PORT) $(BAUD_RATE) tx $(TX_FILE)

.PHONY: run_rx_dbg
run_rx_dbg: $(BIN)/main-dbg
	./$(BIN)/main-dbg $(RX_SERIAL_PORT) $(BAUD_RATE) rx $(RX_FILE)

.PHONY: run_tx_stats
run_tx_stats: $(BIN)/main-stats
	./$(BIN)/main-stats $(TX_SERIAL_PORT) $(BAUD_RATE) tx $(TX_FILE)

.PHONY: run_rx_stats
run_rx_stats: $(BIN)/main-stats
	./$(BIN)/main-stats $(RX_SERIAL_PORT) $(BAUD_RATE) rx $(RX_FILE)

.PHONY: run_cable
run_cable: $(BIN)/cable
	./$(BIN)/cable

.PHONY: check_files
check_files:
	diff -s $(TX_FILE) $(RX_FILE) || exit 0

.PHONY: clean
clean:
	rm -f $(BIN)/main
	rm -f $(BIN)/main-dbg
	rm -f $(BIN)/main-stats
	rm -f $(BIN)/cable
	rm -f $(RX_FILE)
