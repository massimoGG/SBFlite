source = sbflite.c
output = sbflite
libs = bluetooth

compiler = gcc

sbflite : $(output) $(source) 
	$(compiler) -o $(output) -l$(libs) $(source)

.PHONY: clean
clean :
	rm $(output)