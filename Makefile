
all:
	gcc ntpdatehttp.c -lcurl -o ntpdatehttp

clean:
	$(RM) ntpdatehttp

