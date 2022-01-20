# Default to debugging flags. Set to -O3 if you want optimisations
# (either change it here, or type 'make CFLAGS=-O3' in the shell).
CFLAGS?=-g -Wall -Wno-unused-value

all: caltrain 

caltrain: caltrain-runner.c caltrain.c pintos_thread.h
	$(CC) $(CFLAGS) -o caltrain caltrain-runner.c -lpthread 



run: caltrain 
	./caltrain
	./caltrain
	./caltrain
	

clean:
	$(RM) caltrain 
	$(RM) -r *.dSYM
