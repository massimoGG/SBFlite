CC = gcc

source = sbflite.c
output = sbflite
LIBS   = bluetooth mariadbclient
CFLAGS = 
# -c -Wall -O2 -Wno-unused-local-typedefs

sbflite : $(output) $(source) 
	$(CC) -o $(output) $(CFLAGS) $(addprefix -l,$(LIBS)) $(source)

.PHONY: clean
clean :
	rm $(output)
