CC=g++
SRC=src
OBJ=obj
CFLAGS=-g -Wall
ALL_TARGS=	$(SRC)/libgolgi.cpp \
		linux_client

LINUX_CLIENT_SRC=$(SRC)/libgolgi.cpp $(SRC)/String.cpp $(SRC)/Serial.cpp $(SRC)/GolgiNetLinux.cpp $(SRC)/golgi_fixup.cpp $(SRC)/linux_client.cpp $(SRC)/GolgiWrapper.cpp
LINUX_CLIENT_OBJ=$(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(LINUX_CLIENT_SRC))

all:$(ALL_TARGS)

linux_client:$(LINUX_CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lcrypto -lssl -lrt

$(OBJ)/linux_client.o:$(SRC)/linux_client.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/libgolgi.o:$(SRC)/libgolgi.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/String.o:$(SRC)/String.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/Serial.o:$(SRC)/Serial.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/GolgiNetLinux.o:$(SRC)/GolgiNetLinux.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/GolgiWrapper.o:$(SRC)/GolgiWrapper.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ)/golgi_fixup.o:$(SRC)/golgi_fixup.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(SRC)/libgolgi.cpp:
	@echo "*********"
	@echo
	@echo "No libgolgi.cpp present - you will need libgolgi.cpp and libgolgi.h to proceed"
	@echo "Download a copy of libgolgi \"Other\" embedded code for your API at https://devs.golgi.io"
	@echo
	@echo "*********"
	@echo
	@exit -1

clean:
	rm -f $(OBJ)/*.o
	rm -f linux_client

