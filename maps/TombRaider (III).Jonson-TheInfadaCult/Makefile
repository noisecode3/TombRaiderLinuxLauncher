CC = g++
CFLAGS = -Wall -Wextra -std=c++17
LIBS = -lcurl -lzip -lssl -lcrypto
TARGET = Downloader
SRC = Jonson-TheInfadaCult-tr3.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -f $(TARGET)

