MEALS ?= 5

philosophers: FORCE philosophers.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -pthread -DMEALS=$(MEALS) philosophers.c -o $@

# force target
FORCE: ;

