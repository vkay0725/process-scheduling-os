# Process Scheduling

## Overview
This repository contains two implementations of CPU scheduling algorithms:

- **Single Processor Scheduling** (`scheduler1.cpp`)
- **Dual Processor Scheduling** (`scheduler2.cpp`)

Both implementations support the following scheduling algorithms:
- First In First Out (FIFO)
- Non Pre-emptive Shortest Job First (SJF)
- Pre-emptive Shortest Job First (PSJF)
- Round Robin (RR)

---

## Part I: Single Processor Scheduling (`scheduler1.cpp`)

### Algorithms Implemented
#### 1. First In First Out (FIFO)
- Processes are executed in the order of their arrival.
- No pre-emption; once a process starts, it runs to completion.

#### 2. Non Pre-emptive Shortest Job First (SJF)
- The process with the shortest CPU burst is executed first.
- No pre-emption; the selected process runs to completion.

#### 3. Pre-emptive Shortest Job First (PSJF)
- The process with the shortest remaining CPU burst is executed first.
- Pre-emption occurs if a new process arrives with a shorter burst time.

#### 4. Round Robin (RR)
- Processes are executed in a cyclic manner with a fixed time quantum.
- Pre-emption occurs after the time quantum expires.

### How to Run

#### Compile the program:
```bash
g++ scheduler1.cpp -o scheduler1
```

#### Run the program:
```bash
./scheduler1 <algorithm> <input_file>
```
Where:
- `<algorithm>`: FIFO, SJF, PSJF, or RR.
- `<input_file>`: Path to the input file containing process details.

### Input File Format
The input file should contain process details in the following format:
```
<arrival_time> <burst_time_1> <io_time_1> <burst_time_2> <io_time_2> ... -1
```
#### Example:
```
0 5 3 4 2 -1
2 3 1 6 0 -1
```

### Output
- **Schedule:** Displays the start and end times of each process burst.
- **Metrics:**
  - Make Span
  - Average and Maximum Completion Time
  - Average and Maximum Waiting Time

---

## Part II: Dual Processor Scheduling (`scheduler2.cpp`)

### Algorithms Implemented
#### 1. First In First Out (FIFO)
- Processes are assigned to the first available processor in the order of their arrival.

#### 2. Non Pre-emptive Shortest Job First (SJF)
- Processes are assigned to the processor with the shortest burst time.

#### 3. Pre-emptive Shortest Job First (PSJF)
- Processes are assigned to the processor with the shortest remaining burst time.
- Pre-emption occurs if a new process arrives with a shorter burst time.

#### 4. Round Robin (RR)
- Processes are assigned to processors in a cyclic manner with a fixed time quantum.
- Pre-emption occurs after the time quantum expires.

### How to Run

#### Compile the program:
```bash
g++ scheduler2.cpp -o scheduler2
```

#### Run the program:
```bash
./scheduler2 <algorithm> <input_file>
```
Where:
- `<algorithm>`: FIFO, SJF, PSJF, or RR.
- `<input_file>`: Path to the input file containing process details.

### Input File Format
Same as Part I.

### Output
- **Schedule:** Displays the start and end times of each process burst for both processors.
- **Metrics:**
  - Make Span
  - Average and Maximum Completion Time
  - Average and Maximum Waiting Time

---

## Code Structure

### Common Functions
- `parseProcesses`: Reads the input file and creates a list of processes.
- `calculateAndDisplayMetrics`: Computes and displays performance metrics.
- `sortQueueByLastIOEndTime`: Sorts the wait queue based on I/O completion time.

### Scheduler Functions
#### Single Processor:
- `fifoScheduler`
- `sjfScheduler`
- `psjfScheduler`
- `rrScheduler`

#### Dual Processor:
- `fifowith2Processor`
- `sjfwith2Processor`
- `psjfSchedulerTwoProcessors`
- `rrSchedulerTwoProcessors`

---

## Example Usage
### Single Processor
```bash
./scheduler1 FIFO input.txt
```

### Dual Processor
```bash
./scheduler2 RR input.txt
```

---

## Performance Metrics
- **Make Span:** Total time taken to complete all processes.
- **Completion Time:** Time at which each process completes.
- **Waiting Time:** Total time a process spends waiting in the ready queue.

