/*
 * This file tests the implementation in caltrain.c. You shouldn't need to
 * modify it, but you're welcome to do so. Please report any bugs to us via
 * Piazza or email (cs140ta@cs).
 *
 * Note that passing these tests doesn't guarantee that your code is correct
 * or meets the specifications given, but hopefully it's at least pretty
 * close.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#include "caltrain.c"

// tong so luong(thread) khach da hoan thanh ( station_wait_for_train
//tra ve) and dang cho lenh goi tu station_on_board().
volatile int threads_completed = 0;

void*
passenger_thread(void *arg)
{
	struct station *station = (struct station*)arg;
	station_wait_for_train(station);
	__sync_add_and_fetch(&threads_completed, 1);
	return NULL;
}

struct load_train_args {
	struct station *station;
	int free_seats;
};

volatile int load_train_returned = 0;

void*
load_train_thread(void *args)
{
	struct load_train_args *ltargs = (struct load_train_args*)args;
	station_load_train(ltargs->station, ltargs->free_seats);
	load_train_returned = 1;
	return NULL;
}

const char* alarm_error_str;
int alarm_timeout;

void
_alarm(int seconds, const char *error_str)
{
	alarm_timeout = seconds;
	alarm_error_str = error_str;
	alarm(seconds);
}

void
alarm_handler(int foo)
{
	fprintf(stderr, "Loi: Khong the hoan tat sau %d giay. Co gi do "
		"vo ly, Hoac do he thong xu ly tac vu cham. Loi co the o: [%s]\n",
		alarm_timeout, alarm_error_str);
	exit(1);
}

#ifndef MIN
#define MIN(_x,_y) ((_x) < (_y)) ? (_x) : (_y)
#endif

/*
 * dieu nay tao ra mot loat cac chuoi de mo hinh cac chuyen tau den va hanh khach.
 */
int
main()
{
	struct station station;
	station_init(&station);

	srandom(getpid() ^ time(NULL));

	signal(SIGALRM, alarm_handler);

	// Dam bao rang station_load_train() tra lai ngay lap tuc neu khong co hanh khach dang cho.
	_alarm(1, "station_load_train() did not return immediately when no waiting passengers");
	station_load_train(&station, 0);
	station_load_train(&station, 10);
	_alarm(0, NULL);

	// tao mot nhom 'hanh khach', moi hanh khach trong chuoi cua rieng ho.
	int i;
	const int total_passengers = 1000;
	int passengers_left = total_passengers;
	for (i = 0; i < total_passengers; i++) {
		pthread_t tid;
		int ret = pthread_create(&tid, NULL, passenger_thread, &station);
		if (ret != 0) {
			// Neu sai, co the do chung ta vuot qua gioi han cua he thong.
			// thu giam so luong hanh khach ('total_passengers')
			perror("pthread_create");
			exit(1);
		}
	}

	// Dam bao station_load_train() tra ve ngay lap tuc neu khong co ghe trong .
	_alarm(2, "station_load_train() da khong tra ve ngay lap tuc khi khong co ghe trong");
	station_load_train(&station, 0);
	_alarm(0, NULL);

	// Thu nghiem ngau nhien.
	int total_passengers_boarded = 0;
	const int max_free_seats_per_train = 50;
	int pass = 0;
	while (passengers_left > 0) {
		_alarm(2, "Mot so van de khien hanh khach "
			"khong len tau khi tau cap ben");

		int free_seats = random() % max_free_seats_per_train;

		printf("Tau cap ben voi %d ghe trong\n", free_seats);
		load_train_returned = 0;
		struct load_train_args args = { &station, free_seats };
		pthread_t lt_tid;
		int ret = pthread_create(&lt_tid, NULL, load_train_thread, &args);
		if (ret != 0) {
			perror("pthread_create");
			exit(1);
		}

		int threads_to_reap = MIN(passengers_left, free_seats);
		int threads_reaped = 0;
		while (threads_reaped < threads_to_reap) {
			if (load_train_returned) {
				fprintf(stderr, "Loi: station_load_train tra ve qua som!\n");
				exit(1);
			}
			if (threads_completed > 0) {
				if ((pass % 2) == 0)
					usleep(random() % 2);
				threads_reaped++;
				station_on_board(&station);
				__sync_sub_and_fetch(&threads_completed, 1);
			}
		}

		// Cho mot chut de dam bao station_load_train() khong co hanh khach nao len tau.
		for (i = 0; i < 1000; i++) {
			if (i > 50 && load_train_returned)
				break;
			usleep(1000);
		}

		if (!load_train_returned) {
			fprintf(stderr, "Loi: station_load_train loi khi tra ve\n");
			exit(1);
		}

		while (threads_completed > 0) {
			threads_reaped++;
			__sync_sub_and_fetch(&threads_completed, 1);
		}

		passengers_left -= threads_reaped;
		total_passengers_boarded += threads_reaped;
		printf("Tau vao nha ga voi %d hanh khach moi (uoc tinh %d)%s\n",
			threads_reaped, threads_to_reap,
			(threads_to_reap != threads_reaped) ? " *****" : "");

		if (threads_to_reap != threads_reaped) {
			fprintf(stderr, "Loi:Qua nhieu hanh khach tren tau!\n");
			exit(1);
		}

		pass++;
	}

	if (total_passengers_boarded == total_passengers) {
		printf("Looks good!\n");
		return 0;
	} else {
		
		fprintf(stderr, "Error: expected %d total boarded passengers, but got %d!\n",
			total_passengers, total_passengers_boarded);
		return 1;
	}
}
