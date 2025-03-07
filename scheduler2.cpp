#include<bits/stdc++.h>

using namespace std;

struct Burst {
    int duration;
    int ioDuration;
    bool isIO;
};

struct Process {
    int arrivalTime;
    vector<Burst> bursts;
    int remainingTime;
    int totbursts;
    int completionTime;
    int lastIOEndTime;
    int currentBurstIndex;
    bool inIO;
    bool completed;
    int id;
    bool isRunning;
    int cpuAssigned;
    bool isIO;
    int waitTime;
};

struct ProcessRR {
    int processID;
    int arrivalTime;
    vector<int> bursts;
    int totbursts = 0;
    int turnaroundTime = 0;
    int currentBurstIndex = 0;
    int ioEndTime = 0;
    bool isComplete = false;
    bool inReadyQueue, inIOQueue, inpartialQueue;
    int comptime = -1;
    int cpuAssigned;
};

vector<Process> parseProcesses(const string &filename) {
    ifstream file(filename);
    vector<Process> processes;
    int id = 0;

    if (!file.is_open()) {
        cerr << "Failed to open file." << endl;
        return processes;
    }

    string line;
    getline(file, line);
    getline(file, line);
    getline(file, line);
    while (getline(file, line)) {
        if (line.find("</pre>") != string::npos) {
            break;
        }
        stringstream iss(line);
        Process p;
        p.id = id++;
        p.arrivalTime = -1;
        p.remainingTime = 0;
        p.totbursts = 0;
        p.completionTime = 0;
        p.lastIOEndTime = 0;
        p.currentBurstIndex = 0;
        p.inIO = false;
        p.completed = false;

        int value;
        while (iss >> value) {
            if (value == -1) {
                break;
            }
            if (p.arrivalTime == -1) {
                p.arrivalTime = value;
            } else {
                Burst burst;
                burst.duration = value;
                if (!(iss >> value) || value == -1) {
                    burst.ioDuration = 0;
                } else {
                    burst.ioDuration = value;
                }
                p.bursts.push_back(burst);
                p.totbursts += burst.ioDuration + burst.duration;
            }
        }
        processes.push_back(p);
    }
    file.close();
    return processes;
}

struct SJFComparator {
    bool operator()(const Process* a, const Process* b) {
        return a->bursts[a->currentBurstIndex].duration > b->bursts[b->currentBurstIndex].duration;
    }
};

void sortQueueByLastIOEndTime(queue<Process*>& waitQueue) {
    vector<Process*> waitVector;
    while (!waitQueue.empty()) {
        waitVector.push_back(waitQueue.front());
        waitQueue.pop();
    }

    sort(waitVector.begin(), waitVector.end(), [](Process* a, Process* b) {
        return a->lastIOEndTime < b->lastIOEndTime;
    });

    for (Process* p : waitVector) {
        waitQueue.push(p);
    }
}

void calculateAndDisplayMetrics(const vector<Process> &processes) {
    int totalCompletionTime = 0;
    int totalWaitingTime = 0;
    int maxCompletionTime = 0;
    int maxWaitingTime = 0;
    int maxexit = 0;
    int minarr = 0;

    for (const auto &p : processes) {
        int burstTotal = p.totbursts;
        int waitingTime = p.completionTime - p.arrivalTime - burstTotal;
        totalWaitingTime += waitingTime;
        totalCompletionTime += p.completionTime - p.arrivalTime;
        maxWaitingTime = max(maxWaitingTime, waitingTime);
        maxCompletionTime = max(maxCompletionTime, p.completionTime - p.arrivalTime);
        maxexit = max(maxexit, p.completionTime);
        minarr = min(minarr, p.arrivalTime);
    }
    int makespan = maxexit - minarr;
    double avgCompletionTime = static_cast<double>(totalCompletionTime) / processes.size();
    double avgWaitingTime = static_cast<double>(totalWaitingTime) / processes.size();

    cout << "********* METRICS *********" << endl;
    for (const auto &p : processes) {
        int burstTotal = p.totbursts;
        cout << "P" << p.id + 1 << " Arrival Time: " << p.arrivalTime
             << " Exit Time: " << p.completionTime
             << " Waiting Time: " << p.completionTime - p.arrivalTime - p.totbursts << endl;
    }
    cout << "-----------------" << endl;
    cout << "Make Span: " << makespan << endl;
    cout << "Completion Time: [AVG]: " << avgCompletionTime << endl;
    cout << "Completion Time: [MAX]: " << maxCompletionTime << endl;
    cout << "Waiting Time: [AVG]: " << avgWaitingTime << endl;
    cout << "Waiting Time: [MAX]: " << maxWaitingTime << endl;
}

void fifowith2Processor(vector<Process> &processes) {
    queue<Process*> readyQueue;
    queue<Process*> waitQueue;
    vector<tuple<int, int, int, int, int>> schedule;
    int currentTimeCPU0 = 0;
    int currentTimeCPU1 = 0;

    vector<Process*> processPtrs;
    for (auto &p : processes) {
        processPtrs.push_back(&p);
    }
    sort(processPtrs.begin(), processPtrs.end(), [](Process *a, Process *b) {
        return a->arrivalTime < b->arrivalTime;
    });

    if (!processPtrs.empty() && processPtrs[0]->arrivalTime <= min(currentTimeCPU0, currentTimeCPU1)) {
        readyQueue.push(processPtrs[0]);
        processPtrs.erase(processPtrs.begin());
    }

    while (!readyQueue.empty() || !waitQueue.empty() || !processPtrs.empty()) {
        sortQueueByLastIOEndTime(waitQueue);
        while (!waitQueue.empty() && waitQueue.front()->lastIOEndTime <= min(currentTimeCPU0, currentTimeCPU1)) {
            Process* p = waitQueue.front();
            waitQueue.pop();
            p->inIO = false;
            readyQueue.push(p);
        }

        while (!processPtrs.empty() && processPtrs.front()->arrivalTime <= min(currentTimeCPU0, currentTimeCPU1)) {
            readyQueue.push(processPtrs.front());
            processPtrs.erase(processPtrs.begin());
        }

        if (readyQueue.empty()) {
            int nextEventTime = INT_MAX;

            if (!waitQueue.empty()) {
                nextEventTime = waitQueue.front()->lastIOEndTime;
            }

            if (!processPtrs.empty()) {
                nextEventTime = min(nextEventTime, processPtrs.front()->arrivalTime);
            }

            if (nextEventTime != INT_MAX) {
                currentTimeCPU0 = max(currentTimeCPU0, nextEventTime);
                currentTimeCPU1 = max(currentTimeCPU1, nextEventTime);
            }
            continue;
        }

        int currentCPU = (currentTimeCPU0 <= currentTimeCPU1) ? 0 : 1;
        int &currentTime = (currentCPU == 0) ? currentTimeCPU0 : currentTimeCPU1;

        Process* p = readyQueue.front();
        readyQueue.pop();

        if (p->arrivalTime > currentTime) {
            currentTime = p->arrivalTime;
        }

        if (p->currentBurstIndex < p->bursts.size()) {
            Burst &burst = p->bursts[p->currentBurstIndex];
            int burstStart = currentTime;
            int burstEnd = burstStart + burst.duration;

            schedule.emplace_back(p->id + 1, p->currentBurstIndex + 1, burstStart, burstEnd, currentCPU);
            currentTime = burstEnd;
            p->remainingTime -= burst.duration;

            if (p->currentBurstIndex < p->bursts.size()) {
                Burst &nextBurst = p->bursts[p->currentBurstIndex];
                p->lastIOEndTime = currentTime + nextBurst.ioDuration;
                p->inIO = true;
                waitQueue.push(p);
                p->currentBurstIndex++;
                p->completionTime = currentTime;
                p->completed = true;
            }
        }
    }

    cout << "********* CPU 0 Schedule *********" << endl;
    for (const auto &entry : schedule) {
        if (get<4>(entry) == 0) {
            cout << "P" << get<0>(entry) << "," << get<1>(entry)
                 << "\t Start: " << get<2>(entry)
                 << "\t End: " << get<3>(entry) - 1 << endl;
        }
    }

    cout << "********* CPU 1 Schedule *********" << endl;
    for (const auto &entry : schedule) {
        if (get<4>(entry) == 1) {
            cout << "P" << get<0>(entry) << "," << get<1>(entry)
                 << "\t Start: " << get<2>(entry)
                 << "\t End: " << get<3>(entry) - 1 << endl;
        }
    }

    calculateAndDisplayMetrics(processes);
}

void sjfwith2Processor(vector<Process> &processes) {
    priority_queue<Process*, vector<Process*>, SJFComparator> readyQueue;
    queue<Process*> waitQueue;
    vector<tuple<int, int, int, int, int>> schedule;
    int currentTimeCPU0 = 0;
    int currentTimeCPU1 = 0;

    vector<Process*> processPtrs;
    for (auto &p : processes) {
        processPtrs.push_back(&p);
    }
    sort(processPtrs.begin(), processPtrs.end(), [](Process *a, Process *b) {
        return a->arrivalTime < b->arrivalTime;
    });

    while (!processPtrs.empty() && processPtrs[0]->arrivalTime <= min(currentTimeCPU0, currentTimeCPU1)) {
        readyQueue.push(processPtrs[0]);
        processPtrs.erase(processPtrs.begin());
    }

    while (!readyQueue.empty() || !waitQueue.empty() || !processPtrs.empty()) {
        sortQueueByLastIOEndTime(waitQueue);

        while (!waitQueue.empty() && waitQueue.front()->lastIOEndTime <= min(currentTimeCPU0, currentTimeCPU1)) {
            Process* p = waitQueue.front();
            waitQueue.pop();
            p->inIO = false;
            readyQueue.push(p);
        }

        while (!processPtrs.empty() && processPtrs.front()->arrivalTime <= min(currentTimeCPU0, currentTimeCPU1)) {
            readyQueue.push(processPtrs.front());
            processPtrs.erase(processPtrs.begin());
        }

        if (readyQueue.empty()) {
            int nextEventTime = INT_MAX;

            if (!waitQueue.empty()) {
                nextEventTime = waitQueue.front()->lastIOEndTime;
            }

            if (!processPtrs.empty()) {
                nextEventTime = min(nextEventTime, processPtrs.front()->arrivalTime);
            }

            if (nextEventTime != INT_MAX) {
                currentTimeCPU0 = max(currentTimeCPU0, nextEventTime);
                currentTimeCPU1 = max(currentTimeCPU1, nextEventTime);
            }
            continue;
        }

        int currentCPU = (currentTimeCPU0 <= currentTimeCPU1) ? 0 : 1;
        int &currentTime = (currentCPU == 0) ? currentTimeCPU0 : currentTimeCPU1;

        Process* p = readyQueue.top();
        readyQueue.pop();

        if (p->arrivalTime > currentTime) {
            currentTime = p->arrivalTime;
        }

        if (p->currentBurstIndex < p->bursts.size()) {
            Burst &burst = p->bursts[p->currentBurstIndex];
            int burstStart = currentTime;
            int burstEnd = burstStart + burst.duration;

            schedule.emplace_back(p->id + 1, p->currentBurstIndex + 1, burstStart, burstEnd, currentCPU);

            currentTime = burstEnd;

            if (currentCPU == 0) {
                currentTimeCPU0 = currentTime;
            } else {
                currentTimeCPU1 = currentTime;
            }

            p->remainingTime -= burst.duration;

            if (p->currentBurstIndex < p->bursts.size()) {
                Burst &nextBurst = p->bursts[p->currentBurstIndex];
                p->lastIOEndTime = currentTime + nextBurst.ioDuration;
                p->inIO = true;
                waitQueue.push(p);
                p->currentBurstIndex++;
                p->completionTime = currentTime;
                p->completed = true;
            }
        }
    }

    cout << "********* CPU 0 Schedule *********" << endl;
    for (const auto &entry : schedule) {
        if (get<4>(entry) == 0) {
            cout << "P" << get<0>(entry) << "," << get<1>(entry)
                 << "\t Start: " << get<2>(entry)
                 << "\t End: " << get<3>(entry) - 1 << endl;
        }
    }

    cout << "********* CPU 1 Schedule *********" << endl;
    for (const auto &entry : schedule) {
        if (get<4>(entry) == 1) {
            cout << "P" << get<0>(entry) << "," << get<1>(entry)
                 << "\t Start: " << get<2>(entry)
                 << "\t End: " << get<3>(entry) - 1 << endl;
        }
    }

    calculateAndDisplayMetrics(processes);
}

void printSchedule(const vector<tuple<int, int, int, int>> &schedule) {
    for (const auto &entry : schedule) {
        int processID = get<0>(entry);
        int burstIndex = get<1>(entry);
        int startTime = get<2>(entry);
        int endTime = get<3>(entry);
        cout << "P" << processID << "," << burstIndex << "\t" << startTime << "\t" << endTime - 1 << endl;
    }
}

void psjfSchedulerTwoProcessors(vector<Process> &processes) {
    priority_queue<Process*, vector<Process*>, SJFComparator> readyQueue;
    queue<Process*> waitQueue;
    vector<tuple<int, int, int, int, int>> schedule;
    int currentTimeCPU0 = 0, currentTimeCPU1 = 0;
    Process* prevp = nullptr;

    vector<Process*> processPtrs;
    for (auto &p : processes) {
        processPtrs.push_back(&p);
    }
    sort(processPtrs.begin(), processPtrs.end(), [](Process *a, Process *b) {
        return a->arrivalTime < b->arrivalTime;
    });

    for (auto &p : processes) {
        p.remainingTime = accumulate(p.bursts.begin(), p.bursts.end(), 0,
                                     [](int sum, const Burst &b) { return sum + b.duration; });
        p.currentBurstIndex = 0;
    }

    if (!processPtrs.empty() && processPtrs[0]->arrivalTime <= min(currentTimeCPU0, currentTimeCPU1)) {
        readyQueue.push(processPtrs[0]);
        processPtrs.erase(processPtrs.begin());
    }

    while (!readyQueue.empty() || !waitQueue.empty() || !processPtrs.empty()) {
        sortQueueByLastIOEndTime(waitQueue);
        while (!waitQueue.empty() && waitQueue.front()->lastIOEndTime <= min(currentTimeCPU0, currentTimeCPU1)) {
            Process* p = waitQueue.front();
            waitQueue.pop();
            p->inIO = false;
            readyQueue.push(p);
        }

        while (!processPtrs.empty() && processPtrs.front()->arrivalTime <= min(currentTimeCPU0, currentTimeCPU1)) {
            readyQueue.push(processPtrs.front());
            processPtrs.erase(processPtrs.begin());
        }

        if (readyQueue.empty()) {
            int nextEventTime = INT_MAX;

            if (!waitQueue.empty()) {
                nextEventTime = waitQueue.front()->lastIOEndTime;
            }

            if (!processPtrs.empty()) {
                nextEventTime = min(nextEventTime, processPtrs.front()->arrivalTime);
            }

            if (nextEventTime != INT_MAX) {
                currentTimeCPU0 = max(currentTimeCPU0, nextEventTime);
                currentTimeCPU1 = max(currentTimeCPU1, nextEventTime);
            }
            continue;
        }

        int currentCPU = (currentTimeCPU0 <= currentTimeCPU1) ? 0 : 1;
        int &currentTime = (currentCPU == 0) ? currentTimeCPU0 : currentTimeCPU1;
        Process* p;
        Process* pr;

        if (currentCPU == 1) {
            pr = readyQueue.top();
            if (pr != prevp) {
                p = pr;
                readyQueue.pop();
            } else {
                readyQueue.pop();
                if (readyQueue.empty()) {
                    readyQueue.push(pr);
                    currentTimeCPU1++;
                    continue;
                } else {
                    p = readyQueue.top();
                    readyQueue.pop();
                    readyQueue.push(pr);
                }
            }
        } else {
            p = readyQueue.top();
            readyQueue.pop();
        }

        if (p->arrivalTime > currentTime) {
            currentTime = p->arrivalTime;
        }

        if (p->currentBurstIndex < p->bursts.size()) {
            Burst &burst = p->bursts[p->currentBurstIndex];
            int burstStart = currentTime;
            int timeToExecute = min(1, burst.duration);
            int burstEnd = burstStart + timeToExecute;

            schedule.emplace_back(p->id + 1, p->currentBurstIndex + 1, burstStart, burstEnd, currentCPU);

            currentTime = burstEnd;
            p->remainingTime -= timeToExecute;
            burst.duration -= timeToExecute;

            if (burst.duration > 0) {
                if (currentCPU == 0) {
                    prevp = p;
                }
                readyQueue.push(p);
            } else {
                if (p->currentBurstIndex < p->bursts.size()) {
                    Burst &nextBurst = p->bursts[p->currentBurstIndex];
                    p->lastIOEndTime = currentTime + nextBurst.ioDuration;
                    p->inIO = true;
                    waitQueue.push(p);
                    p->currentBurstIndex++;
                    p->completionTime = currentTime;
                    p->completed = true;
                }
            }
        }
    }

    for (int cpu = 0; cpu <= 1; cpu++) {
        cout << "********* CPU " << cpu << " Schedule *********" << endl;
        vector<tuple<int, int, int, int>> consolidatedSchedule;
        int lastPID = -1, lastBurst = -1, lastEndTime = -1, lastStartTime = -1;

        for (const auto &entry : schedule) {
            if (get<4>(entry) == cpu) {
                int pid = get<0>(entry);
                int burst = get<1>(entry);
                int startTime = get<2>(entry);
                int endTime = get<3>(entry);

                if (pid == lastPID && burst == lastBurst && startTime == lastEndTime) {
                    lastEndTime = endTime;
                } else {
                    if (lastPID != -1) {
                        consolidatedSchedule.emplace_back(lastPID, lastBurst, lastStartTime, lastEndTime);
                    }
                    lastPID = pid;
                    lastBurst = burst;
                    lastStartTime = startTime;
                    lastEndTime = endTime;
                }
            }
        }
        if (lastPID != -1) {
            consolidatedSchedule.emplace_back(lastPID, lastBurst, lastStartTime, lastEndTime);
        }

        for (const auto &entry : consolidatedSchedule) {
            cout << "P" << get<0>(entry) << ", " << get<1>(entry)
                 << ", " << get<2>(entry)
                 << ", " << get<3>(entry) - 1 << endl;
        }
    }

    calculateAndDisplayMetrics(processes);
}

void rrSchedulerTwoProcessors(vector<Process> &processes, int timequantum) {
    deque<Process*> readyQueue;
    queue<Process*> waitQueue;
    vector<tuple<int, int, int, int, int>> schedule;
    int currentTimeCPU0 = 0, currentTimeCPU1 = 0;
    Process* prevp = nullptr;
    Process* partialp = NULL;

    vector<Process*> processPtrs;
    for (auto &p : processes) {
        processPtrs.push_back(&p);
    }
    sort(processPtrs.begin(), processPtrs.end(), [](Process *a, Process *b) {
        return a->arrivalTime < b->arrivalTime;
    });

    for (auto &p : processes) {
        p.remainingTime = accumulate(p.bursts.begin(), p.bursts.end(), 0,
                                     [](int sum, const Burst &b) { return sum + b.duration; });
        p.currentBurstIndex = 0;
    }

    if (!processPtrs.empty() && processPtrs[0]->arrivalTime <= min(currentTimeCPU0, currentTimeCPU1)) {
        readyQueue.push_back(processPtrs[0]);
        processPtrs.erase(processPtrs.begin());
    }

    while (!readyQueue.empty() || !waitQueue.empty() || !processPtrs.empty()) {
        sortQueueByLastIOEndTime(waitQueue);
        while (!waitQueue.empty() && waitQueue.front()->lastIOEndTime <= min(currentTimeCPU0, currentTimeCPU1)) {
            Process* p = waitQueue.front();
            waitQueue.pop();
            p->inIO = false;
            readyQueue.push_back(p);
        }

        while (!processPtrs.empty() && processPtrs.front()->arrivalTime <= min(currentTimeCPU0, currentTimeCPU1)) {
            readyQueue.push_back(processPtrs.front());
            processPtrs.erase(processPtrs.begin());
        }

        if (readyQueue.empty()) {
            int nextEventTime = INT_MAX;

            if (!waitQueue.empty()) {
                nextEventTime = waitQueue.front()->lastIOEndTime;
            }

            if (!processPtrs.empty()) {
                nextEventTime = min(nextEventTime, processPtrs.front()->arrivalTime);
            }

            if (nextEventTime != INT_MAX) {
                currentTimeCPU0 = max(currentTimeCPU0, nextEventTime);
                currentTimeCPU1 = max(currentTimeCPU1, nextEventTime);
            }
            continue;
        }

        int currentCPU = (currentTimeCPU0 <= currentTimeCPU1) ? 0 : 1;
        int &currentTime = (currentCPU == 0) ? currentTimeCPU0 : currentTimeCPU1;
        Process* p;
        Process* pr;

        if (currentCPU == 1) {
            pr = readyQueue.front();
            if (pr != prevp) {
                p = pr;
                readyQueue.pop_front();
            } else {
                readyQueue.pop_front();
                if (readyQueue.empty()) {
                    readyQueue.push_back(pr);
                    currentTimeCPU1++;
                    continue;
                } else {
                    p = readyQueue.front();
                    readyQueue.pop_front();
                    readyQueue.push_front(pr);
                }
            }
        } else {
            p = readyQueue.front();
            readyQueue.pop_front();
        }

        if (p->arrivalTime > currentTime) {
            currentTime = p->arrivalTime;
        }

        if (p->currentBurstIndex < p->bursts.size()) {
            Burst &burst = p->bursts[p->currentBurstIndex];
            int burstStart = currentTime;
            int timeToExecute = min(timequantum, burst.duration);
            int burstEnd = burstStart + timeToExecute;

            schedule.emplace_back(p->id + 1, p->currentBurstIndex + 1, burstStart, burstEnd, currentCPU);

            currentTime = burstEnd;
            p->remainingTime -= timeToExecute;
            burst.duration -= timeToExecute;

            if (burst.duration > 0) {
                if (currentCPU == 0) {
                    prevp = p;
                }
                readyQueue.push_back(p);
            } else {
                if (p->currentBurstIndex < p->bursts.size()) {
                    Burst &nextBurst = p->bursts[p->currentBurstIndex];
                    p->lastIOEndTime = currentTime + nextBurst.ioDuration;
                    p->inIO = true;
                    waitQueue.push(p);
                    p->currentBurstIndex++;
                    p->completionTime = currentTime;
                    p->completed = true;
                }
            }
        }
    }

    cout << "********* CPU 0 Schedule *********" << endl;
    for (const auto &entry : schedule) {
        if (get<4>(entry) == 0) {
            cout << "P" << get<0>(entry) << "," << get<1>(entry)
                 << "\t Start: " << get<2>(entry)
                 << "\t End: " << get<3>(entry) - 1 << endl;
        }
    }

    cout << "********* CPU 1 Schedule *********" << endl;
    for (const auto &entry : schedule) {
        if (get<4>(entry) == 1) {
            cout << "P" << get<0>(entry) << "," << get<1>(entry)
                 << "\t Start: " << get<2>(entry)
                 << "\t End: " << get<3>(entry) - 1 << endl;
        }
    }

    calculateAndDisplayMetrics(processes);
}

int main(int argc, char *argv[]) {
    clock_t start, end;
    start = clock();

    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <scheduling_algorithm> <file>" << endl;
        return 1;
    }

    string algorithm = argv[1];
    string filename = argv[2];

    if (algorithm == "FIFO") {
        vector<Process> processes = parseProcesses(filename);
        if (processes.empty()) {
            cerr << "No processes to schedule." << endl;
            return 1;
        }
        fifowith2Processor(processes);
    } else if (algorithm == "SJF") {
        vector<Process> processes = parseProcesses(filename);
        if (processes.empty()) {
            cerr << "No processes to schedule." << endl;
            return 1;
        }
        sjfwith2Processor(processes);
    } else if (algorithm == "PSJF") {
        vector<Process> processes = parseProcesses(filename);
        if (processes.empty()) {
            cerr << "No processes to schedule." << endl;
            return 1;
        }
        psjfSchedulerTwoProcessors(processes);
    } else if (algorithm == "RR") {
        vector<Process> processes = parseProcesses(filename);
        if (processes.empty()) {
            cerr << "No processes to schedule." << endl;
            return 1;
        }
        rrSchedulerTwoProcessors(processes, 10);
    } else {
        cerr << "Unknown scheduling algorithm: " << algorithm << endl;
        return 1;
    }
    end = clock();
    double totruntime = double(end - start) / double(CLOCKS_PER_SEC);
    cout << "Total execution time (in seconds): " << totruntime << endl;
    cout << "-----------------" << endl;

    return 0;
}
