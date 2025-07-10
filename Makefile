all:
	gcc main.c -o wifi-manager `pkg-config --cflags --libs gtk+-3.0`

clean:
	rm -f wifi-manager

