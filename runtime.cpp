#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// print the procedure of function invacations
// #define PRINT_PROCEDURE

typedef void (*ptr_t)(char *);

static __thread std::chrono::time_point<std::chrono::steady_clock> start_time;
static __thread int depth = 0;
static std::map<char *, std::vector<double>> m_string_timevec;
static std::mutex g_lock;

extern "C" {

void do_statistic(void) {
    double total_time = 0.0;
    char out_border[] = "======================";
    char in_border[] = "--------------------";
    fprintf(stderr, "+%s+%s+%s+\n", out_border, out_border, out_border);
    for (auto iter = m_string_timevec.begin(); iter != m_string_timevec.end();
         ++iter) {
        fprintf(stderr, "| %-20s | %20s | %20s |\n", iter->first, "Invocation",
                "Time(s)");
        double f_total_time = 0.0;
        size_t cnt = 1;
        for (auto v_iter = iter->second.begin(); v_iter != iter->second.end();
             ++v_iter) {
            f_total_time += *v_iter;
            fprintf(stderr, "| %20s | %20u | %20f |\n", " ", cnt++, *v_iter);
        }
        fprintf(stderr, "| %20s + %s + %s +\n", " ", in_border, in_border);
        fprintf(stderr, "| %20s | %20s | %20f |\n", " ", "Function total time",
                f_total_time);
        fprintf(stderr, "+%s+%s+%s+\n", out_border, out_border, out_border);
        total_time += f_total_time;
    }
    fprintf(stderr, "| %-20s | %20s | %20f |\n", "Summary", "Total time(s)",
            total_time);
    fprintf(stderr, "+%s+%s+%s+\n", out_border, out_border, out_border);
}

void init_func() {
    atexit(do_statistic);
}

void before_call(void *func) {
#ifdef PRINT_PROCEDURE
    printf("in fuck ptr: %u\n", (unsigned)((ptr_t)func));
    for (auto i = 0; i < depth; ++i) printf(" ");
    depth += 4;
    printf("Invocation of function <%s> starts\n", (char *)func);
#endif
    start_time = std::chrono::steady_clock::now();
}

void after_call(void *func) {
#ifdef PRINT_PROCEDURE
    for (auto i = 0; i < depth - 4; ++i) printf(" ");
    depth -= 4;
#endif

    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> ms = end_time - start_time;
    double dur_time = ms.count();
    char *func_name = static_cast<char *>(func);

#ifdef PRINT_PROCEDURE
    fprintf(stderr, "Invocation of function <%s> ends. Duration time: %fs\n",
            func_name, dur_time);
#endif

    {
        std::unique_lock<std::mutex> lk(g_lock);
        auto item = m_string_timevec.find(func_name);
        if (item == m_string_timevec.end())
            m_string_timevec[func_name] = std::vector<double>{dur_time};
        else
            m_string_timevec[func_name].push_back(dur_time);
    }
}
}
