//SID: 500037218
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    int head;

    int size;
} Seats;

typedef struct {
    int is_busy;
} BaberStatus;

void *custormer_routine(void *arg);
void *assistant_routine(void *arg);
void *baber_routine(void *arg);

BaberStatus *init_babers(int n);
int find_free_babers();
int all_babers_are_free();

int acquire_seat(Seats *seats);
void release_seat(Seats *seats, int ticket);

int rand_range(int min, int max);
void read_input(const char *prompt, int *var);

void log_custormer_arrive(int id);
void log_custormer_leave(int id);
void log_custormer_get_seat(int id, int ticket);
void log_custormer_called(int c_id, int ticket, int b_id);
void log_custormer_haircut_done(int c_id, int b_id);
void log_baber_ready(int id);
void log_baber_start_service(int id, int ticket);
void log_baber_finish_service(int id, int ticket);
void log_baber_finish_work(int id);
void log_assistant_wait_custormer();
void log_assistant_wait_baber();
void log_assistant_assign_service(int ticket, int b_id);
void log_assistant_finish_work();
void log_main_thread_end();


int num_custormers;
int num_babers;
int num_chairs;
int min_service_time;
int max_service_time;
int min_arrival_time;
int max_arrival_time;

Seats seats = {.head = 0, .size = 0};
BaberStatus *babers;

pthread_mutex_t seats_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t seat_released_cond;

pthread_mutex_t baber_status_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t haircut_done_cond_1;
pthread_cond_t haircut_done_cond_2;

pthread_mutex_t custormer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t custormer_cond;
int custormer = 0;
pthread_cond_t assistant_to_c_cond;
int assistant_to_c = 0;
pthread_cond_t custormer_get_baber_id_cond;
int custormer_get_baber_id = 0;

pthread_mutex_t baber_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t *assistant_to_b_conds;
int *assistant_to_b = 0;
pthread_cond_t baber_get_ticket_cond;
int baber_get_ticket = 0;

int current_baber_id;
int current_custormer_ticket;

int main() {

    read_input("the total number of customers", &num_custormers);
    read_input("the number of barbers working in the salon", &num_babers);
    read_input("the total number of waiting chairs in the salon", &num_chairs);
    read_input("the minimum number of units of time for a barber to service a "
               "customer",
               &min_service_time);
    read_input("the maximum number of units of time for a barber to service a "
               "customer",
               &max_service_time);
    read_input(
        "the minimum number of units of time between two customer arrivals",
        &min_arrival_time);
    read_input(
        "the maximum number of units of time between two customer arrivals",
        &max_arrival_time);

    pthread_cond_init(&seat_released_cond, NULL);
    pthread_cond_init(&custormer_cond, NULL);
    pthread_cond_init(&assistant_to_c_cond, NULL);
    pthread_cond_init(&custormer_get_baber_id_cond, NULL);
    assistant_to_b_conds = malloc(sizeof(pthread_cond_t) * num_babers);
    assert(assistant_to_b_conds != NULL);
    assistant_to_b = malloc(sizeof(int) * num_babers);
    assert(assistant_to_b != NULL);
    for (int i = 0; i < num_babers; i++) {
        pthread_cond_init(&assistant_to_b_conds[i], NULL);
        assistant_to_b[i] = 0;
    }
    pthread_cond_init(&baber_get_ticket_cond, NULL);
    pthread_cond_init(&haircut_done_cond_1, NULL);
    pthread_cond_init(&haircut_done_cond_2, NULL);

    babers = init_babers(num_babers);


    int num_threads = 1 + num_babers + num_custormers;
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    assert(threads != NULL);
    int *baber_ids = (int*)malloc(sizeof(int) * num_babers);
    assert(baber_ids != NULL);
    int *custormer_ids = (int*)malloc(sizeof(int) * num_custormers);
    assert(custormer_ids != NULL);
    for (int i = 0; i < num_threads; i++) {
        int rc;
        if (i == 0) {
            rc = pthread_create(&threads[i], NULL, assistant_routine, NULL);
            assert(rc == 0);
        } else if (1 <= i && i < 1 + num_babers) {
            int id = i; 
            baber_ids[id - 1] = id;
            rc = pthread_create(&threads[i], NULL, baber_routine,
                                (void *)&(baber_ids[id - 1]));
            assert(rc == 0);
        } else {
            int id = i - num_babers; 
            custormer_ids[id - 1] = id;
            rc = pthread_create(&threads[i], NULL, custormer_routine,
                                (void *)&(custormer_ids[id - 1]));
            assert(rc == 0);
            sleep(rand_range(min_arrival_time, max_arrival_time));
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&seats_mutex);
    pthread_mutex_destroy(&baber_status_mutex);
    pthread_mutex_destroy(&custormer_mutex);
    pthread_mutex_destroy(&baber_mutex);
    pthread_cond_destroy(&seat_released_cond);
    pthread_cond_destroy(&haircut_done_cond_1);
    pthread_cond_destroy(&haircut_done_cond_2);
    pthread_cond_destroy(&custormer_cond);
    pthread_cond_destroy(&assistant_to_c_cond);
    pthread_cond_destroy(&custormer_get_baber_id_cond);
    for (int i = 0; i < num_babers; i++) {
        pthread_cond_destroy(&assistant_to_b_conds[i]);
    }
    pthread_cond_destroy(&baber_get_ticket_cond);

    free(assistant_to_b_conds);
    free(assistant_to_b);
    free(babers);
    free(threads);
    free(baber_ids);
    free(custormer_ids);

    log_main_thread_end();
    return 0;
}

void *custormer_routine(void *arg) {
    int c_id = *(int *)arg;
    log_custormer_arrive(c_id);
    pthread_mutex_lock(&seats_mutex);
    int ticket = acquire_seat(&seats);
    if (ticket == 0) {
        log_custormer_leave(c_id);
    } else {
        log_custormer_get_seat(c_id, ticket);
        while (ticket - 1 != seats.head) {
            pthread_cond_wait(&seat_released_cond, &seats_mutex);
        }
    }
    pthread_mutex_unlock(&seats_mutex);

    pthread_mutex_lock(&custormer_mutex);
    current_custormer_ticket = ticket;
    pthread_cond_signal(&custormer_cond);
    custormer++;
    pthread_mutex_unlock(&custormer_mutex);

    if (ticket == 0) {
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&custormer_mutex);
    if (assistant_to_c == 0) {
        pthread_cond_wait(&assistant_to_c_cond, &custormer_mutex);
    }
    assistant_to_c--;
    int b_id = current_baber_id;
    log_custormer_called(c_id, ticket, b_id);
    custormer_get_baber_id++;
    pthread_cond_signal(&custormer_get_baber_id_cond);
    pthread_mutex_unlock(&custormer_mutex);

    pthread_mutex_lock(&seats_mutex);
    release_seat(&seats, ticket);
    pthread_cond_broadcast(&seat_released_cond);
    pthread_mutex_unlock(&seats_mutex);

    pthread_mutex_lock(&baber_status_mutex);
    while (babers[b_id - 1].is_busy) {
        pthread_cond_wait(&haircut_done_cond_1, &baber_status_mutex);
    }
    log_custormer_haircut_done(c_id, b_id);
    pthread_cond_signal(&haircut_done_cond_2);
    pthread_mutex_unlock(&baber_status_mutex);

    pthread_exit(NULL);
}

void *assistant_routine(void *arg) {
    int serviced_custormers = 0;
    int ticket, baber_id;
    while (serviced_custormers < num_custormers) {
        pthread_mutex_lock(&custormer_mutex);
        if (custormer == 0) {
            log_assistant_wait_custormer();
            pthread_cond_wait(&custormer_cond, &custormer_mutex);
        }
        custormer--;
        ticket = current_custormer_ticket;
        if (ticket == 0) {
            serviced_custormers++;
            pthread_mutex_unlock(&custormer_mutex);
            continue;
        }

        pthread_mutex_lock(&baber_status_mutex);
        baber_id = find_free_babers();
        if (baber_id == 0) {
            log_assistant_wait_baber();
            pthread_cond_wait(&haircut_done_cond_2, &baber_status_mutex);
            baber_id = find_free_babers();
            assert(baber_id != 0);
        }
        babers[baber_id - 1].is_busy = 1;
        pthread_mutex_unlock(&baber_status_mutex);
        pthread_mutex_lock(&baber_mutex);
        current_baber_id = baber_id;
        log_assistant_assign_service(ticket, baber_id);
        pthread_cond_signal(&assistant_to_c_cond);
        assistant_to_c++;
        pthread_cond_signal(&assistant_to_b_conds[baber_id - 1]);
        assistant_to_b[baber_id - 1]++;

        pthread_mutex_unlock(&custormer_mutex);
        pthread_mutex_unlock(&baber_mutex);

        pthread_mutex_lock(&custormer_mutex);
        if (custormer_get_baber_id == 0) {
            pthread_cond_wait(&custormer_get_baber_id_cond, &custormer_mutex);
            custormer_get_baber_id--;
        }
        pthread_mutex_unlock(&custormer_mutex);

        pthread_mutex_lock(&baber_mutex);
        if (baber_get_ticket == 0) {
            pthread_cond_wait(&baber_get_ticket_cond, &baber_mutex);
            baber_get_ticket--;
        }
        pthread_mutex_unlock(&baber_mutex);

        serviced_custormers++;
    }

    pthread_mutex_lock(&baber_status_mutex);
    while (!all_babers_are_free()) {
        pthread_cond_wait(&haircut_done_cond_2, &baber_status_mutex);
    }
    pthread_mutex_unlock(&baber_status_mutex);

    log_assistant_finish_work();
    pthread_mutex_lock(&baber_mutex);
    current_baber_id = -1;
    for (int i = 0; i < num_babers; i++) {
        pthread_cond_signal(&assistant_to_b_conds[i]);
        assistant_to_b[i]++;
    }
    pthread_mutex_unlock(&baber_mutex);

    pthread_exit(NULL);
}

void *baber_routine(void *arg) {
    int b_id = *(int *)arg;
    int ticket;
    while (1) {
        log_baber_ready(b_id);
        pthread_mutex_lock(&baber_mutex);
        if (assistant_to_b[b_id - 1] == 0) {
            pthread_cond_wait(&assistant_to_b_conds[b_id - 1], &baber_mutex);
        }
        assistant_to_b[b_id - 1]--;
        if (current_baber_id == -1) {
            pthread_mutex_unlock(&baber_mutex);
            break;
        }
        ticket = current_custormer_ticket;
        pthread_cond_signal(&baber_get_ticket_cond);
        baber_get_ticket++;
        pthread_mutex_unlock(&baber_mutex);

        log_baber_start_service(b_id, ticket);
        sleep(rand_range(min_service_time, max_service_time));
        log_baber_finish_service(b_id, ticket);

        pthread_mutex_lock(&baber_status_mutex);
        babers[b_id - 1].is_busy = 0;
        pthread_cond_broadcast(&haircut_done_cond_1);
        pthread_mutex_unlock(&baber_status_mutex);
    }

    log_baber_finish_work(b_id);

    pthread_exit(NULL);
}

BaberStatus *init_babers(int n) {
    BaberStatus *babers = (BaberStatus *)malloc(sizeof(BaberStatus) * n);
    assert(babers != NULL);
    for (int i = 0; i < n; i++) {
        babers[i].is_busy = 0;
    }
    return babers;
}

int find_free_babers() {
    for (int i = 0; i < num_babers; i++) {
        if (!babers[i].is_busy) {
            return i + 1;
        }
    }
    return 0;
}

int all_babers_are_free() {
    for (int i = 0; i < num_babers; i++) {
        if (babers[i].is_busy) {
            return 0;
        }
    }
    return 1;
}

int acquire_seat(Seats *seats) {
    int ticket;
    if (seats->size == num_chairs) {
        ticket = 0;
    } else {
        ticket = (seats->head + seats->size) % num_chairs + 1;
        seats->size++;
    }
    return ticket;
}

void release_seat(Seats *seats, int ticket) {
    assert(ticket == seats->head + 1);
    seats->head = (seats->head + 1) % num_chairs;
    seats->size--;
}

int rand_range(int min, int max) { return rand() % (max - min + 1) + min; }

void read_input(const char *prompt, int *var) {
    printf("Enter %s (int):\n", prompt);
    scanf("%d", var);
}

void log_custormer_arrive(int id) {
    printf("Customer %d: I have arrived at the barber shop.\n", id);
}

void log_custormer_leave(int id) {
    printf(
        "Customer %d: Oh no! All seats have been taken and I'll leave now!\n",
        id);
}

void log_custormer_get_seat(int id, int ticket) {
    printf(
        "Customer %d: I'm lucky to get a free seat and a ticket numbered %d\n",
        id, ticket);
}

void log_custormer_called(int c_id, int ticket, int b_id) {
    printf(
        "Customer %d: My ticket number %d has been called. Hello, Barber %d.\n",
        c_id, ticket, b_id);
}

void log_custormer_haircut_done(int c_id, int b_id) {
    printf("Customer %d: Well done. Thanks Barber %d. Bye!\n", c_id, b_id);
}

void log_baber_ready(int id) {
    printf("Barber %d: I'm now ready to accept a customer.\n", id);
}

void log_baber_start_service(int id, int ticket) {
    printf("Barber %d: Hello, Customer %d.\n", id, ticket);
}

void log_baber_finish_service(int id, int ticket) {
    printf("Barber %d: Finished cutting. Good bye Customer %d.\n", id, ticket);
}

void log_baber_finish_work(int id) {
    printf("Barber %d: Thanks Assistant. See you tomorrow!\n", id);
}

void log_assistant_wait_custormer() {
    printf("Assistant: I'm waiting for customers.\n");
}

void log_assistant_wait_baber() {
    printf("Assistant: I'm waiting for a barber to become available.\n");
}

void log_assistant_assign_service(int ticket, int b_id) {
    printf("Assistant: Assign Customer %d to Barber %d.\n", ticket, b_id);
}

void log_assistant_finish_work() {
    printf("Assistant: Hi barbers. We've finished the work for the day. "
            "See you all tomorrow!\n");
}

void log_main_thread_end() {
    printf("Main thread: All customers have now been served. "
            "Salon is closed now.\n");
}
