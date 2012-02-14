CPPFLAGS ?= -pedantic-errors -Wall
CFLAGS   ?= -pipe -O2

CPPFLAGS += -std=c99 -D_BSD_SOURCE -fPIC
LDFLAGS  += -shared -ltokyocabinet

bayes.so: bayes.c

%.so: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $^

.c.so:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o $@ $>
