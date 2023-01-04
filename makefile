parta: ping.c
	gcc ping.c -o parta

partb: new_ping.c watchdog
	gcc new_ping.c -o partb

watchdog: watchdog.c
	gcc watchdog.c -o watchdog

clean: 
	rm *.o parta partb watchdog