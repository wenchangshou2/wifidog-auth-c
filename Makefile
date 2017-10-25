SOURCES =  ./mongoose.c  ./tools.h
wifidog_auth: wifidog_auth.o
	$(CC) $(SOURCES) $(LDFLAGS) wifidog-auth.o -o  wifidog-auth
wifidog_auth.o: wifidog-auth.c
	$(CC) $(CFLAGS) -c wifidog-auth.c
clear:
	rm *.o wifidog_auth