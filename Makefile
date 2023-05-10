linenoise_example: linenoise.c example.c
	$(CC) -lpthread -o linenoise_example linenoise.c example.c

clean:
	rm -f linenoise_example
