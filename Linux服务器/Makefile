SOURCE:=$(wildcard *.c)
a.out:${SOURCE}
	gcc -g $^ -o $@ -pthread -L/usr/lib/mysql -lmysqlclient
clean:
	rm a.out

