/*
    usage:

        ./multi_level <TESTFILE>
        where <TESTFILE> is the name of a job list
*/

#include "multi_level.h"

/*
    Helper function
    Decide quantum to run based on current process level
*/
int getQuantum(PcbPtr p, int t0, int t1) {
    if (!p) {
        return 1;
    } else {
        int time_quantum;
        if (p->level == LEVEL_0) {
            time_quantum = t0;
        } else if (p->level == LEVEL_1) {
            time_quantum = t1;
        } else {
            time_quantum = 1;
        }
        if (p->remaining_cpu_time < time_quantum) {
            return p->remaining_cpu_time;
        } else {
            return time_quantum;
        }
    }
}

int main(int argc, char* argv[]) {

    /*** Main function variable declarations ***/

    FILE * input_list_stream = NULL;
    PcbPtr job_queue = NULL;
    PcbPtr l0_queue = NULL;
    PcbPtr l1_queue = NULL;
    PcbPtr l2_queue = NULL;
    PcbPtr current_process = NULL;
    PcbPtr process = NULL;
    int timer = 0;
    int quantum;

    int t0;
    int t1;
    int k;

    int turnaround_time;
    double av_turnaround_time = 0.0, av_wait_time = 0.0;
    int n = 0;

    // Init job queue

    if (argc <= 0)
    {
        fprintf(stderr, "FATAL: Bad arguments array\n");
        exit(EXIT_FAILURE);
    }
    else if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <TESTFILE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!(input_list_stream = fopen(argv[1], "r")))
    {
        fprintf(stderr, "ERROR: Could not open \"%s\"\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    while (!feof(input_list_stream)) {  // put processes into job_queue
        process = createnullPcb();
        if (fscanf(input_list_stream,"%d, %d",
             &(process->arrival_time), &(process->service_time)) != 2) {
            free(process);
            continue;
        }
	    process->remaining_cpu_time = process->service_time;
        process->status = PCB_INITIALIZED;
        job_queue = enqPcb(job_queue, process);
	    n++;
    }


    //  Ask the user to specify t0, t1 and k
    printf("Please enter 3 positive integers for t0, t1 and k: ");
    scanf("%d %d %d", &t0, &t1, &k);
    if (t0 <= 0 || t1 <= 0 || k <= 0)
    {
        printf("Invalid input t0, t1 and k.\n");
        exit(EXIT_FAILURE);
    }

    while(job_queue || l0_queue || l1_queue || l2_queue || current_process) {
        // Job-queue -> Level0-queue
        while(job_queue && job_queue->arrival_time <= timer) {
            process = deqPcb(&job_queue);
            process->status = PCB_READY;
            process->level = LEVEL_0;
            process->iteration = k;
            l0_queue = enqPcb(l0_queue, process);
        }
        // If a process is currently running;
        if (current_process) {
            current_process->remaining_cpu_time -= quantum;
            if (current_process->remaining_cpu_time <= 0) {
                terminatePcb(current_process);
                // Calculate and acumulate turnaround time and wait time;
 		       turnaround_time = timer - current_process->arrival_time;
		      av_turnaround_time += turnaround_time;
		      av_wait_time += (turnaround_time - current_process->service_time);   
		      printf ("turnaround time = %d, waiting time = %d\n", turnaround_time, 
		          turnaround_time - current_process->service_time);
                // Free up process structure memory
                free(current_process);
                current_process = NULL;
                // else if other processes are waiting in RR queue:
            } else {
                // suspend the process
                suspendPcb(current_process);
                if (current_process->level == LEVEL_0) {
                    // level0 -> level1
                    current_process->level = LEVEL_1;
                    l1_queue = enqPcb(l1_queue, current_process);
                } else if (current_process->level == LEVEL_1) {
                    current_process->iteration--;
                    if (current_process->iteration == 0) {
                        // level1 -> level2
                        current_process->level = LEVEL_2;
                        l2_queue = enqPcb(l2_queue, current_process);
                    } else {
                        // stay in level1
                        l1_queue = enqPcb(l1_queue, current_process);
                    }
                } else if (current_process->level == LEVEL_2) {
                    // insert at front (FCFS)
                    l2_queue = enqPcb(current_process, l2_queue);
                }
                current_process = NULL;
            }
        }

        if (!current_process) {
            if (l0_queue) {
                current_process = deqPcb(&l0_queue);
            } else if (l1_queue) {
                current_process = deqPcb(&l1_queue);
            } else if (l2_queue) {
                current_process = deqPcb(&l2_queue);
            }
            if (current_process) {
                startPcb(current_process);
            }
        }

       // decide quantum by level
       quantum = getQuantum(current_process, t0, t1);
       sleep(quantum);

       timer += quantum;

    }

    //  print out average turnaround time and average wait time
    av_turnaround_time = av_turnaround_time / n;
    av_wait_time = av_wait_time / n;
    printf("average turnaround time = %f\n", av_turnaround_time);
    printf("average wait time = %f\n", av_wait_time);
    
    //  4. Terminate the RR dispatcher
    exit(EXIT_SUCCESS);


}