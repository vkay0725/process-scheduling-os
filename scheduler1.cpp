#include<bits/stdc++.h>

using namespace std;

struct Burst {
    int duration;
    int ioDuration;
};

struct Process {
    int arrivalTime;
    vector<Burst> bursts;
    int remainingTime;
    int waitTime;
    int completionTime;
    int lastIOEndTime;
    int currentBurstIndex;
    bool inIO;
    bool completed;
    int id; 
    int turnAroundTime = 0; 
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
        if(line.find("</pre>") != string::npos){
            break;
        }
        stringstream iss(line);
        Process p;
        p.id = id++;
        p.arrivalTime = -1;
        p.remainingTime = 0;
        p.waitTime = 0;
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
    int maxexit=0;
    int minarr=0;

    for (const auto &p : processes) {
        int burstTotal = accumulate(p.bursts.begin(), p.bursts.end(), 0,
            [](int sum, const Burst &b) { return sum + b.duration; });
        totalWaitingTime += p.waitTime;
        totalCompletionTime += p.completionTime - p.arrivalTime;
        maxWaitingTime = max(maxWaitingTime, p.waitTime);
        maxCompletionTime = max(maxCompletionTime, p.completionTime - p.arrivalTime);
        maxexit=max(maxexit,p.completionTime);
        minarr=min(minarr,p.arrivalTime);
    }

    int makespan = maxexit-minarr;
    double avgCompletionTime = static_cast<double>(totalCompletionTime) / processes.size();
    double avgWaitingTime = static_cast<double>(totalWaitingTime) / processes.size();

    cout << "********* METRICS *********" << endl;
    for (const auto &p : processes) {
        int burstTotal = accumulate(p.bursts.begin(), p.bursts.end(), 0,
            [](int sum, const Burst &b) { return sum + b.duration; });
        cout << "P" << p.id+1 << " Arrival Time: " << p.arrivalTime
             << " Exit Time: " << p.completionTime
             << " Waiting Time: " << p.waitTime << endl;
    }
    cout << "-----------------" << endl;
    cout << "Make Span: " << makespan << endl;
    cout << "Completion Time: [AVG]: " << avgCompletionTime<<endl;
    cout << "Completion Time: [MAX]: " << maxCompletionTime << endl;
    cout << "Waiting Time: [AVG]: " << avgWaitingTime<<endl;
    cout<< "Waiting Time: [MAX]: " << maxWaitingTime << endl;
}

void psjfScheduler(vector<Process> &processes) {
    priority_queue<Process*, vector<Process*>, SJFComparator> readyQueue;  
    queue<Process*> waitQueue;  
    vector<tuple<int, int, int, int>> schedule;  
    int currentTime = 0;

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

    if (!processPtrs.empty() && processPtrs[0]->arrivalTime <= currentTime) {
        readyQueue.push(processPtrs[0]);
        processPtrs.erase(processPtrs.begin());
    }

    while (!readyQueue.empty() || !waitQueue.empty() || !processPtrs.empty()) {
        sortQueueByLastIOEndTime(waitQueue);
        while (!waitQueue.empty() && waitQueue.front()->lastIOEndTime <= currentTime) {
            Process* p = waitQueue.front();
            waitQueue.pop();
            p->inIO = false;  
            readyQueue.push(p);  
        }

        while (!processPtrs.empty() && processPtrs.front()->arrivalTime <= currentTime) {
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
                currentTime = nextEventTime;
            }
            continue;
        }

        Process* p = readyQueue.top();
        readyQueue.pop();

        if (p->arrivalTime > currentTime) {
            currentTime = p->arrivalTime;
        }

        if (p->currentBurstIndex < p->bursts.size()) {
            Burst &burst = p->bursts[p->currentBurstIndex];
            int burstStart = currentTime;
            int timeToExecute = min(1, burst.duration);  
            int burstEnd = burstStart + timeToExecute;
            schedule.emplace_back(p->id + 1, p->currentBurstIndex + 1, burstStart, burstEnd);

            currentTime = burstEnd;
            p->remainingTime -= timeToExecute;
            burst.duration -= timeToExecute;  
            priority_queue<Process*, vector<Process*>, SJFComparator> temp=readyQueue; 
            while(!readyQueue.empty()){
                Process* p1 = readyQueue.top();
                readyQueue.pop();
                p1->waitTime+=timeToExecute;
            }
            readyQueue=temp;

            if (burst.duration > 0) {
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

    if (!schedule.empty()) {
        int lastProcessID = get<0>(schedule[0]);
        int lastBurstIndex = get<1>(schedule[0]);
        int lastStartTime = get<2>(schedule[0]);
        int lastEndTime = get<3>(schedule[0]);

        for (size_t i = 1; i < schedule.size(); ++i) {
            int currentProcessID = get<0>(schedule[i]);
            int currentBurstIndex = get<1>(schedule[i]);
            int currentStartTime = get<2>(schedule[i]);
            int currentEndTime = get<3>(schedule[i]);

            if (currentProcessID == lastProcessID && currentBurstIndex == lastBurstIndex && currentStartTime == lastEndTime) {
                lastEndTime = currentEndTime;
            } else {
                cout << "P" << lastProcessID << "," << lastBurstIndex << "\t " << lastStartTime << "\t " << lastEndTime - 1 << endl;
                lastProcessID = currentProcessID;
                lastBurstIndex = currentBurstIndex;
                lastStartTime = currentStartTime;
                lastEndTime = currentEndTime;
            }
        }
        cout << "P" << lastProcessID << "," << lastBurstIndex << "\t " << lastStartTime << "\t " << lastEndTime - 1 << endl;
    }

    calculateAndDisplayMetrics(processes);
}

void fifoScheduler(vector<Process> &processes) {
    queue<Process*> readyQueue;
    queue<Process*> waitQueue;
    vector<tuple<int,int, int, int>> schedule; 
    int currentTime = 0;

    vector<Process*> processPtrs;
    for (auto &p : processes) {
        processPtrs.push_back(&p);
    }
    sort(processPtrs.begin(), processPtrs.end(), [](Process *a, Process *b) {
        return a->arrivalTime < b->arrivalTime;
    });

    if (!processPtrs.empty() && processPtrs[0]->arrivalTime <= currentTime) {
        readyQueue.push(processPtrs[0]);
        processPtrs.erase(processPtrs.begin());
    }

    while (!readyQueue.empty() || !waitQueue.empty() || !processPtrs.empty()) {
        sortQueueByLastIOEndTime(waitQueue);
        while (!waitQueue.empty() && waitQueue.front()->lastIOEndTime <= currentTime) {
            Process* p = waitQueue.front();
            waitQueue.pop();
            p->inIO = false;
            readyQueue.push(p);
        }

        while (!processPtrs.empty() && processPtrs.front()->arrivalTime <= currentTime) {
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
                currentTime = nextEventTime;
            }
            continue;
        }

        Process* p = readyQueue.front();
        readyQueue.pop();

        if (p->arrivalTime > currentTime) {
            currentTime = p->arrivalTime;
        }
        
        if (p->currentBurstIndex < p->bursts.size()) {
            Burst &burst = p->bursts[p->currentBurstIndex];
            int burstStart = currentTime;
            int burstEnd = burstStart + burst.duration;
            schedule.emplace_back(p->id+1, p->currentBurstIndex+1, burstStart, burstEnd);

            currentTime = burstEnd;
            p->remainingTime -= burst.duration;
            queue<Process*> temp=readyQueue; 
            while(!readyQueue.empty()){
                Process* p1 = readyQueue.front();
                readyQueue.pop();
                p1->waitTime+= burst.duration;
            }
            readyQueue=temp;

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

    for (const auto &entry : schedule) {
        cout << "P" << get<0>(entry) << "," << get<1>(entry) << "\t " << get<2>(entry)<< "\t " << get<3>(entry)-1 << endl;
    }

    calculateAndDisplayMetrics(processes);
}

void sjfScheduler(vector<Process> &processes) {
    priority_queue<Process*, vector<Process*>, SJFComparator> readyQueue;  
    queue<Process*> waitQueue;  
    vector<tuple<int, int, int, int>> schedule;  
    int currentTime = 0;

    vector<Process*> processPtrs;
    for (auto &p : processes) {
        processPtrs.push_back(&p);
    }
    sort(processPtrs.begin(), processPtrs.end(), [](Process *a, Process *b) {
        return a->arrivalTime < b->arrivalTime;
    });

    if (!processPtrs.empty() && processPtrs[0]->arrivalTime <= currentTime) {
        readyQueue.push(processPtrs[0]);
        processPtrs.erase(processPtrs.begin());
    }

    while (!readyQueue.empty() || !waitQueue.empty() || !processPtrs.empty()) {
        sortQueueByLastIOEndTime(waitQueue);

        while (!waitQueue.empty() && waitQueue.front()->lastIOEndTime <= currentTime) {
            Process* p = waitQueue.front();
            waitQueue.pop();

            p->inIO = false;  
            readyQueue.push(p);  
        }

        while (!processPtrs.empty() && processPtrs.front()->arrivalTime <= currentTime) {
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
                currentTime = nextEventTime;
            }
            continue;
        }

        Process* p = readyQueue.top();
        readyQueue.pop();

        if (p->arrivalTime > currentTime) {
            currentTime = p->arrivalTime;
        }
        
        if (p->currentBurstIndex < p->bursts.size()) {
            Burst &burst = p->bursts[p->currentBurstIndex];
            int burstStart = currentTime;
            int burstEnd = burstStart + burst.duration;
            schedule.emplace_back(p->id + 1, p->currentBurstIndex + 1, burstStart, burstEnd);
            priority_queue<Process*, vector<Process*>, SJFComparator> temp=readyQueue; 
            while(!readyQueue.empty()){
                Process* p1 = readyQueue.top();
                readyQueue.pop();
                p1->waitTime+=burst.duration;
            }
            readyQueue=temp;

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

    for (const auto &entry : schedule) {
        cout << "P" << get<0>(entry) << "," << get<1>(entry) << "\t " << get<2>(entry) << "\t " << get<3>(entry) - 1 << endl;
    }

    calculateAndDisplayMetrics(processes);
}

void rrScheduler(vector<Process> &processes,int timequantum) {
    queue<Process*> readyQueue;  
    queue<Process*> waitQueue;  
    vector<tuple<int, int, int, int>> schedule;  
    int currentTime = 0;

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

    if (!processPtrs.empty() && processPtrs[0]->arrivalTime <= currentTime) {
        readyQueue.push(processPtrs[0]);
        processPtrs.erase(processPtrs.begin());
    }

    while (!readyQueue.empty() || !waitQueue.empty() || !processPtrs.empty()) {
        sortQueueByLastIOEndTime(waitQueue);
        while (!waitQueue.empty() && waitQueue.front()->lastIOEndTime <= currentTime) {
            Process* p = waitQueue.front();
            waitQueue.pop();
            p->inIO = false;  
            readyQueue.push(p);  
        }

        while (!processPtrs.empty() && processPtrs.front()->arrivalTime <= currentTime) {
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
                currentTime = nextEventTime;
            }
            continue;
        }

        Process* p = readyQueue.front();
        readyQueue.pop();

        if (p->arrivalTime > currentTime) {
            currentTime = p->arrivalTime;
        }

        if (p->currentBurstIndex < p->bursts.size()) {
            Burst &burst = p->bursts[p->currentBurstIndex];
            int burstStart = currentTime;
            int timeToExecute = min(timequantum, burst.duration);  
            int burstEnd = burstStart + timeToExecute;
            schedule.emplace_back(p->id + 1, p->currentBurstIndex + 1, burstStart, burstEnd);

            currentTime = burstEnd;
            p->remainingTime -= timeToExecute;
            burst.duration -= timeToExecute;  
            queue<Process*> temp=readyQueue; 
            while(!readyQueue.empty()){
                Process* p1 = readyQueue.front();
                readyQueue.pop();
                p1->waitTime+=timeToExecute;
            }
            readyQueue=temp;

            if (burst.duration > 0) {
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

    if (!schedule.empty()) {
        int lastProcessID = get<0>(schedule[0]);
        int lastBurstIndex = get<1>(schedule[0]);
        int lastStartTime = get<2>(schedule[0]);
        int lastEndTime = get<3>(schedule[0]);

        for (size_t i = 1; i < schedule.size(); ++i) {
            int currentProcessID = get<0>(schedule[i]);
            int currentBurstIndex = get<1>(schedule[i]);
            int currentStartTime = get<2>(schedule[i]);
            int currentEndTime = get<3>(schedule[i]);

            if (currentProcessID == lastProcessID && currentBurstIndex == lastBurstIndex && currentStartTime == lastEndTime) {
                lastEndTime = currentEndTime;
            } else {
                cout << "P" << lastProcessID << ", " << lastBurstIndex << ", " << lastStartTime << ", " << lastEndTime - 1 << endl;
                lastProcessID = currentProcessID;
                lastBurstIndex = currentBurstIndex;
                lastStartTime = currentStartTime;
                lastEndTime = currentEndTime;
            }
        }
        cout << "P" << lastProcessID << ", " << lastBurstIndex << ",  " << lastStartTime << ",  " << lastEndTime - 1 << endl;
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
        fifoScheduler(processes);
    }
    else if (algorithm == "SJF") {
        vector<Process> processes = parseProcesses(filename);
        if (processes.empty()) {
            cerr << "No processes to schedule." << endl;
            return 1;
        }
        sjfScheduler(processes);
    }
    else if (algorithm == "RR") {
        vector<Process> processes = parseProcesses(filename);
        if (processes.empty()) {
            cerr << "No processes to schedule." << endl;
            return 1;
        }
        rrScheduler(processes,10);
    }
    else if (algorithm == "PSJF") {
        vector<Process> processes = parseProcesses(filename);
        if (processes.empty()) {
            cerr << "No processes to schedule." << endl;
            return 1;
        }
        psjfScheduler(processes);
    }
    else {
        cerr << "Unknown scheduling algorithm: " << algorithm << endl;
        return 1;
    }
    end = clock();
    double totruntime = double(end - start) / double(CLOCKS_PER_SEC);
    cout<<"Total execution time (in seconds): "<<totruntime<<endl;
    cout << "-----------------" << endl;

    return 0;
}
