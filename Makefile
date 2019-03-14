iamroot: start_iamroot.o functions.o udp.o tcp.o querylist.o
	gcc -g -o iamroot start_iamroot.o functions.o udp.o tcp.o querylist.o

start_iamroot.o: start_iamroot.c functions.h udp.h tcp.h
	gcc -g -c start_iamroot.c

functions.o: functions.c functions.h udp.h tcp.h
	gcc -g -c functions.c

udp.o: udp.c udp.h
	gcc -g -c udp.c

tcp.o: tcp.c tcp.h
	gcc -g -c tcp.c

querylist.o: querylist.c querylist.h
	gcc -g -c querylist.c

clean:
	rm iamroot start_iamroot.o functions.o udp.o tcp.o querylist.o
