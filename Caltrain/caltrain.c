#include "pintos_thread.h"
#include <stdio.h>

struct station {
	struct condition *cond_train_arrived;
	struct condition *cond_all_passengers_seated;
	struct lock *lck;
	int station_waiting_passengers;
	int train_empty_seats;
	int train_standing_passengers;
};
void print_station(struct station *station) 
{
	printf("[Nha ga|Hanh khach dang cho: %d, Hanh khach tren tau: %d, Ghe tau trong: %d]\n",
			station->station_waiting_passengers,station->train_standing_passengers,station->train_empty_seats);
	//In ra thong bao so khach dang doi, so khach dang dung tren tau, so ghe trong tren tau
}

void
station_init(struct station *station)
{
	station->cond_train_arrived = malloc(sizeof(struct condition)); //Cap phat bo nho dong cho bien con tro
	station->cond_all_passengers_seated= malloc(sizeof(struct condition));//Cap phat bo nho dong cho bien con tro
	station->lck = malloc(sizeof(struct lock));//Cap phat bo nho dong cho bien con tro
	cond_init(station->cond_train_arrived);
	cond_init(station->cond_all_passengers_seated);
	lock_init(station->lck);
	station->station_waiting_passengers = 0;//khoi tao so khach doi = 0
	station->train_empty_seats = 0;//khoi tao so ghe trong tren tau = 0
	station->train_standing_passengers = 0; //khoi tao so khach dang dung tren tau = 0
	printf("init ->"); print_station(station);
}

void
station_load_train(struct station *station, int count)
{
	lock_acquire(station->lck);
	station->train_empty_seats = count;//gan so ghe trong tren tau = count
	printf("Nha ga cap ben (Dem: %d)->", count); print_station(station); //In ra thong bao tau den ga voi bao nhieu ghe trong

	while ((station->station_waiting_passengers > 0) && (station->train_empty_seats > 0)) {
		cond_broadcast(station->cond_train_arrived,station->lck);
		cond_wait(station->cond_all_passengers_seated,station->lck);
	}

	//Tat ca hanh khach da tren tau
	//printf("train left ->"); print_station(station);

	//Reset cho chuyen tau tiep theo
	station->train_empty_seats = 0; //So ghe trong tren tau = 0
	lock_release(station->lck);
}

void
station_wait_for_train(struct station *station)
{
	lock_acquire(station->lck);
	station->station_waiting_passengers++; //So khach den ga doi se tang len
	printf("Hanh khach toi nha ga ->"); print_station(station);//In ra thong bao khach da den ga
	while (station->train_standing_passengers == station->train_empty_seats) //wait for train with empty seats space
		cond_wait(station->cond_train_arrived,station->lck);
	station->train_standing_passengers++; //So khach dang dung tren tau tang len
	station->station_waiting_passengers--;//So khach dung doi giam (vi khach da len tau dung)
	printf("Hanh khach len tau ->"); print_station(station);//In ra thong bao khach dang len tau
	lock_release(station->lck);
}

void
station_on_board(struct station *station)
{
	lock_acquire(station->lck);
	station->train_standing_passengers--; //So khach dang dung tren tau giam (Vi khach da ngoi xuong ghe trong)
	station->train_empty_seats--;//So ghe trong giam
	printf("Hanh khach tren tau"); print_station(station);//In ra thong bao khach da tren tau ( Tuc la khach da ngoi xuong ghe trong)
	if ((station->train_empty_seats == 0) || (station->train_standing_passengers == 0))//Neu so ghe trong tren tau = so khach dang dung tren tau
		cond_signal(station->cond_all_passengers_seated,station->lck); //Tat ca cac hanh khach da duoc ngoi
	
	lock_release(station->lck);
}
