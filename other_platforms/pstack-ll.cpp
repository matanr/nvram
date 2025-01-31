#include <iostream>
#include <fstream>
#include <cstring>
#include "PBenchmarkStacks.hpp"
#include "TMLinkedListStackByRef.hpp"
#include "ptm.h"

// Macros suck, but we can't have multiple PTMs at the same time (too much memory)
#ifdef USE_CXPTM
#include "ptms/cxptm/CXPTM.hpp"
#define DATA_FILE "data/pstack-ll-cxptm.txt"
#elif defined USE_CXREDO
#include "ptms/cxredo/CXRedo.hpp"
#define DATA_FILE "data/pstack-ll-cxredo.txt"
#elif defined USE_CXREDOTIMED
#include "ptms/cxredotimed/CXRedoTimed.hpp"
#define DATA_FILE "data/pstack-ll-cxredotimed.txt"
#elif defined USE_ROMLR
#include "ptms/romuluslr/RomulusLR.hpp"
#define DATA_FILE "data/pstack-ll-romlr.txt"
#elif defined USE_ROM_LOG_FC
    #include "romulus/RomLogFC.hpp"
    #ifdef SAME100_BENCH
        #define DATA_FILE "../data/same100-green-pstack-ll-romlogfc.txt"
        #define PDATA_FILE "../data/same100-pwb-pfence-romlogfc.txt"
    #elif defined(RANDOP)
        #define DATA_FILE "../data/randop-green-pstack-ll-romlogfc.txt"
        #define PDATA_FILE "../data/randop-pwb-pfence-romlogfc.txt"
    #else
        #define DATA_FILE "../data/green-pstack-ll-romlogfc.txt"
        #define PDATA_FILE "../data/pwb-pfence-romlogfc.txt"
    #endif

#elif defined USE_OFWF
#include "one_file/OneFilePTMWF.hpp"
#define DATA_FILE "../data/green-pstack-ll-ofwf.txt"
#endif

#ifndef DATA_FILE
    #ifdef RANDOP
        #define DATA_FILE "../data/randop-green-pstack-ll-" PTM_FILEXT ".txt"
    #elif defined(SAME100_BENCH)
        #define DATA_FILE "../data/same100-green-pstack-ll-" PTM_FILEXT ".txt"
    #else 
        #define DATA_FILE "../data/green-pstack-ll-" PTM_FILEXT ".txt"
    #endif
#endif

#ifndef PDATA_FILE
    #ifdef RANDOP
        #define PDATA_FILE "../data/randop-pwb-pfence-" PTM_FILEXT ".txt"
    #elif defined SAME100_BENCH
        #define PDATA_FILE "../data/same100-pwb-pfence-" PTM_FILEXT ".txt"
    #else
        #define PDATA_FILE "../data/pwb-pfence-" PTM_FILEXT ".txt"
    #endif

#endif

#if defined(USE_PMDK)
// extern __thread uint64_t tl_num_pwbs;
thread_local uint64_t tl_num_pwbs = 0;
#endif



#define MILLION  1000000LL

int main(void) {
    const std::string dataFilename { DATA_FILE };
    const std::string pdataFilename { PDATA_FILE };
    vector<int> threadList = { 1, 2, 4, 8, 10, 16, 24, 32, 40 };     // For Castor
    // std::vector<int> threadList = { 1, 2, 4, 8, 10, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 64, 68, 72, 76, 80 };     // For Castor
    const int numRuns = 10;
    // const int numRuns = 1;                                           // Number of runs
    const long numPairs = 1*MILLION;                                 // 1M is fast enough on the laptop

    tuple<uint64_t, double, double> results[threadList.size()];
    std::string cName;
    // Reset results
    std::memset(results, 0, sizeof(uint64_t)*threadList.size());

    // Enq-Deq Throughput benchmarks
    for (int it = 0; it < threadList.size(); it++) {
        int nThreads = threadList[it];
        PBenchmarkStacks bench(nThreads);
        std::cout << "\n----- pstack-ll (push-pop)   threads=" << nThreads << "   pairs=" << numPairs/MILLION << "M   runs=" << numRuns << " -----\n";
#ifdef USE_CXPTM
            results[it] = bench.pushPop<TMLinkedListStack<uint64_t,cx::CX,cx::persist>,                              cx::CX>                  (cName, numPairs, numRuns);
#elif defined USE_CXREDO
            results[it] = bench.pushPop<TMLinkedListStack<uint64_t,cxredo::CXRedo,cxredo::persist>,                  cxredo::CXRedo>          (cName, numPairs, numRuns);
#elif defined USE_CXREDOTIMED
            results[it] = bench.pushPop<TMLinkedListStack<uint64_t,cxredotimed::CXRedoTimed,cxredotimed::persist>,   cxredotimed::CXRedoTimed>(cName, numPairs, numRuns);
#elif defined USE_ROMLR
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,romuluslr::RomulusLR,romuluslr::persist>,    romuluslr::RomulusLR>    (cName, numPairs, numRuns);
#elif defined USE_ROMLOG
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,romuluslog::RomulusLog,romuluslog::persist>, romuluslog::RomulusLog>  (cName, numPairs, numRuns);
#elif defined USE_ROM_LOG_FC
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,romlogfc::RomLog,romlogfc::persist>,         romlogfc::RomLog>  (cName, numPairs, numRuns);
#elif defined USE_OFWF
            results[it] = bench.pushPop<TMLinkedListStack<uint64_t,pofwf::OneFileWF,pofwf::tmtype>,                  pofwf::OneFileWF>        (cName, numPairs, numRuns);
#elif defined USE_DUALZONE_2F_FC
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,dualzone2ffc::DualZone,dualzone2ffc::persist>, dualzone2ffc::DualZone>(cName, numPairs, numRuns);
#elif defined USE_DZ_V1
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,dzv1::DualZone,dzv1::persist>,               dzv1::DualZone>          (cName, numPairs, numRuns);
#elif defined USE_DZ_V2
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,dzv2::DualZone,dzv2::persist>,               dzv2::DualZone>          (cName, numPairs, numRuns);
#elif defined USE_DZ_V3
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,dzv3::DualZone,dzv3::persist>,               dzv3::DualZone>          (cName, numPairs, numRuns);
#elif defined USE_DZ_V4
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,dzv4::DualZone,dzv4::persist>,               dzv4::DualZone>          (cName, numPairs, numRuns);
#elif defined USE_DZ_TL2
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,dztl2::DualZone,dztl2::persist>,             dztl2::DualZone>         (cName, numPairs, numRuns);
#elif defined USE_REDO_LOG_FC
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,redologfc::RedoLog,redologfc::persist>,      redologfc::RedoLog>      (cName, numPairs, numRuns);
#elif defined USE_REDO_LOG_TL2
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,redologtl2::RedoLog,redologtl2::persist>,    redologtl2::RedoLog>     (cName, numPairs, numRuns);
#else
            results[it] = bench.pushPop<TMLinkedListStackByRef<uint64_t,PTM_CLASS,PTM_TYPE>, PTM_CLASS>(cName, numPairs, numRuns);
#endif
    }
    #if defined(COUNT_PWB) && (defined(USE_ROM_LOG_FC) || defined(USE_OFLF))
    // Export tab-separated values to a file to be imported in gnuplot or excel
    ofstream pdataFile;
    pdataFile.open(pdataFilename);
    pdataFile << "Threads\t";
    // Printf class names for each column
    pdataFile << cName<<"-PWB" << "\t" << cName<<"-PFENCE" << "\t";
    pdataFile << "\n";
    for (int it = 0; it < threadList.size(); it++) {
        pdataFile << threadList[it] << "\t";
        pdataFile << get<1>(results[it]) << "\t";
        pdataFile << get<2>(results[it]) << "\t";
        pdataFile << "\n";
    }
    pdataFile.close();
    std::cout << "\nSuccessfuly saved results in " << pdataFilename << "\n";
    #else
    // Export tab-separated values to a file to be imported in gnuplot or excel
    ofstream dataFile;
    dataFile.open(dataFilename);
    dataFile << "Threads\t";
    // Printf class names for each column
    dataFile << cName << "\t";
    dataFile << "\n";
    for (int it = 0; it < threadList.size(); it++) {
        dataFile << threadList[it] << "\t";
        dataFile << get<0>(results[it]) << "\t";
        dataFile << "\n";
    }
    dataFile.close();
    std::cout << "\nSuccessfuly saved results in " << dataFilename << "\n";
    #endif

    return 0;
}
