/*
This file is part of BCP-MAPF.

BCP-MAPF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

BCP-MAPF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BCP-MAPF.  If not, see <https://www.gnu.org/licenses/>.

Author: Edward Lam <ed@ed-lam.com>
*/

#include "Includes.h"
#include "Reader.h"
#include "Output.h"
#include "scip/scip.h"
#include "scip/scipshell.h"
#include "scip/scipdefplugins.h"
#include "cxxopts.hpp"
#include "Benchmarker.h"
#include <stdlib.h>
#include "CTPL/ctpl_stl.h"

static SCIP_RETCODE setup_solver(SCIP** scip, bool multi_threaded)
{
    SCIP_CALL(SCIPcreate(scip));

    // Set up plugins.
    {
        // Include default SCIP plugins.
        SCIP_CALL(SCIPincludeDefaultPlugins(*scip));

        // Disable parallel solve.
        SCIP_CALL(SCIPsetIntParam(*scip, "parallel/maxnthreads", 1));
        SCIP_CALL(SCIPsetIntParam(*scip, "lp/threads", 1));

        // Set parameters.
        SCIP_CALL(SCIPsetIntParam(*scip, "presolving/maxrounds", 0));
        SCIP_CALL(SCIPsetIntParam(*scip, "propagating/rootredcost/freq", -1));
        SCIP_CALL(SCIPsetIntParam(*scip, "separating/maxaddrounds", -1));
        SCIP_CALL(SCIPsetIntParam(*scip, "separating/maxstallrounds", 5));
        SCIP_CALL(SCIPsetIntParam(*scip, "separating/maxstallroundsroot", 20));
        SCIP_CALL(SCIPsetIntParam(*scip, "separating/cutagelimit", -1));

        if (multi_threaded)
        {
            SCIP_CALL(SCIPsetIntParam(*scip, "display/verblevel", 0));
        }

        // Turn off all separation algorithms.
        SCIP_CALL(SCIPsetSeparating(*scip, SCIP_PARAMSETTING_OFF, TRUE));

        // Turn on aggressive primal heuristics.
        SCIP_CALL(SCIPsetHeuristics(*scip, SCIP_PARAMSETTING_AGGRESSIVE, TRUE));

        // Turn off some primal heuristics.
        {
            const auto nheurs = SCIPgetNHeurs(*scip);
            auto heurs = SCIPgetHeurs(*scip);
            for (Int idx = 0; idx < nheurs; ++idx)
            {
                auto heur = heurs[idx];
                const String name(SCIPheurGetName(heur));
                if (name == "alns" ||
                    name == "bound" ||
                    name == "coefdiving" ||
                    name == "crossover" ||
                    name == "dins" ||
                    name == "fixandinfer" ||
                    name == "gins" ||
                    name == "guideddiving" ||
                    name == "intdiving" ||
                    name == "localbranching" ||
                    name == "locks" ||
                    name == "mutation" ||
                    name == "oneopt" ||
                    name == "rens" ||
                    name == "repair" ||
                    name == "rins" ||
                    name == "trivial" ||
                    name == "zeroobj" ||
                    name == "zirounding" ||
                    name == "proximity" || // Buggy
                    name == "twoopt")      // Buggy
                {
                    SCIPheurSetFreq(heur, -1);
                }
            }
        }
    }
    return SCIP_OKAY;
}

static SCIP_RETCODE cleanup_solver(SCIP* scip)
{
    // Free memory.
    SCIP_CALL(SCIPfree(&scip));

    // Check if memory is leaked.
    BMScheckEmptyMemory();

    return SCIP_OKAY;
}

static SCIP_RETCODE run_file_solver(String instance_file, SCIP_Real time_limit, Agent agents_limit)
{
    // Initialize SCIP.
    SCIP* scip = nullptr;
    SCIP_CALL(setup_solver(&scip, false));


    // Read instance.
    release_assert(agents_limit > 0, "Cannot limit to {} number of agents", agents_limit);
    SCIP_CALL(read_instance(scip, instance_file.c_str(), agents_limit));

    // Set time limit.
    if (time_limit > 0)
    {
        SCIP_CALL(SCIPsetRealParam(scip, "limits/time", time_limit));
    }

    // Solve.
    SCIP_CALL(SCIPsolve(scip));

    // Output.
    {
        // Print.
        println("");
        SCIP_CALL(SCIPprintStatistics(scip, NULL));

        // Write best solution to file.
        SCIP_CALL(write_best_solution(scip));
    }

    // Clean up
    SCIP_CALL(cleanup_solver(scip));

    return SCIP_OKAY;

}

static SCIP_Retcode run_instance(int id, Problem* problem, int index, int number, SCIP_Real time_limit, bool multi_threaded)
{
    // Start benchmark clock
    println("  Start solving {}", number);
    problem->start_clock();

    // Initialize SCIP.
    SCIP* scip = nullptr;
    SCIP_CALL(setup_solver(&scip, multi_threaded));

    // Read instance.
    SCIP_CALL(read_instance(scip, problem));

    // Set time limit.
    if (time_limit > 0)
    {
        SCIP_CALL(SCIPsetRealParam(scip, "limits/time", time_limit));
    }
    // Solve.
    try {
        SCIP_CALL(SCIPsolve(scip));
    }
    catch (std::bad_alloc& ba)
    {
        println("bad_alloc caught: {}", ba.what());
    }
    catch (std::overflow_error& oe)
    {
        println("Overflow error caught: {}", oe.what());
    }

    // Output.
    {
//        // Print.
//        println("");
//        SCIP_CALL(SCIPprintStatistics(scip, NULL));
//
//        // Write best solution to file.
//        SCIP_CALL(write_best_solution(scip));

        // Save best solution to problem
        SCIP_CALL(save_best_solution(scip, problem));

        // Stop benchmark clock
        problem->stop_clock();
        println("{} Finished solving {:<4} {} ms", problem->solved ? "!" : " ", number, problem->elapsed.count());
    }

    // Clean up
    SCIP_CALL(cleanup_solver(scip));
}

static SCIP_RETCODE run_index_solver(std::vector<int> instance_index, SCIP_Real time_limit, Agent agents_limit, const bool debug, const int threadcount)
{

    for (int i = 0; i < instance_index.size(); i++)
    {
        Benchmarker bm;

        bm.load(instance_index[i], debug);

        int run_count = 0;
        bool has_new_problems = true;
        while(has_new_problems)
        {
            if (threadcount > 1)
            {
                ctpl::thread_pool thread_pool_(threadcount);

                for (int i = 0; i < bm.problems.size(); i++)
                {
                    Problem* problem = bm.problems[i];

                    thread_pool_.push(run_instance, problem, i, 1+i+run_count*50, (bm.timeout > 0 ? bm.timeout : time_limit), true);
                }
                thread_pool_.stop(true);

                has_new_problems = bm.submit();
                run_count++;
            }
            else
            {
                for (int i = 0; i < bm.problems.size(); i++)
                {
                    Problem* problem = bm.problems[i];

                    run_instance(0, problem, i, 1+i+run_count*50, (bm.timeout > 0 ? bm.timeout : time_limit), bm.problems.size() == 1 ? false :true);
                }

                has_new_problems = bm.submit();
                run_count++;
            }
        }
    }


    return SCIP_OKAY;
}

static
SCIP_RETCODE start_solver(
    int argc,      // Number of shell parameters
    char** argv    // Array with shell parameters
)
{
    // Parse program options.
    String instance_file;
    bool file_mode = false;
    std::vector<int> instance_index;
    bool index_mode = false;
    SCIP_Real time_limit = 0;
    bool debug = false;
    int thread_count = 1;
    Agent agents_limit = std::numeric_limits<Agent>::max();
    try
    {
        // Create program options.
        cxxopts::Options options(argv[0],
                                 "BCP-MAPF - branch-and-cut-and-price solver for "
                                 "multi-agent path finding");
        options.positional_help("instance_file").show_positional_help();
        options.add_options()
            ("help", "Print help")
            ("f,file", "Path to instance file", cxxopts::value<Vector<String>>())
            ("i,index", "Index on instance in database", cxxopts::value<Vector<int>>())
            ("t,time-limit", "Time limit in seconds", cxxopts::value<SCIP_Real>())
            ("a,agents-limit", "Read first N agents only", cxxopts::value<int>())
            ("p,parallel", "Number of parallel runs to use for benchmarking", cxxopts::value<int>())
            ("d,debug", "Run in debug mode")
        ;
        options.parse_positional({"file"});

        // Parse options.
        auto result = options.parse(argc, argv);

        // Print help.
        if (result.count("help") || !(result.count("file") || result.count("index")))
        {
            println("{}", options.help());
            exit(0);
        }

        // Get path to instance.
        if (result.count("file"))
        {
            file_mode = true;
            instance_file = result["file"].as<Vector<String>>().at(0);
        }

        // Get path to instance.
        if (result.count("index"))
        {
            index_mode = true;
            instance_index = result["index"].as<Vector<int>>();
        }

        // Get thread count.
        if (result.count("parallel"))
        {
            thread_count = result["parallel"].as<int>();
        }

        // Get debug.
        if (result.count("debug"))
        {
            debug = true;
        }

        // Get time limit.
        if (result.count("time-limit"))
        {
            time_limit = result["time-limit"].as<SCIP_Real>();
        }

        // Get agents limit.
        if (result.count("agents-limit"))
        {
            agents_limit = result["agents-limit"].as<int>();
        }
    }
    catch (const cxxopts::OptionException& e)
    {
        err("{}", e.what());
    }

    // Print.
    println("Branch-and-cut-and-price solver for multi-agent path finding");
    println("Edward Lam <ed@ed-lam.com>");
    println("Monash University, Melbourne, Australia");
#ifdef DEBUG
    println("Compiled in debug mode");
#ifdef USE_WAITEDGE_CONFLICTS
    println("Using wait-edge conflict constraints");
#endif
#ifdef USE_RECTANGLE_KNAPSACK_CONFLICTS
    println("Using rectangle knapsack conflict constraints");
#endif
#ifdef USE_CORRIDOR_CONFLICTS
    println("Using corridor conflict constraints");
#endif
#ifdef USE_WAITDELAY_CONFLICTS
    println("Using wait-delay conflict constraints");
#endif
#ifdef USE_EXITENTRY_CONFLICTS
    println("Using exit-entry conflict constraints");
#endif
#ifdef USE_TWOEDGE_CONFLICTS
    println("Using two-edge conflict constraints");
#endif
#ifdef USE_GOAL_CONFLICTS
    println("Using goal conflict constraints");
#endif
#endif
    println("");


    if (file_mode)
    {
        SCIP_CALL(run_file_solver(instance_file, time_limit, agents_limit));
    }

    if (index_mode)
    {
        SCIP_CALL(run_index_solver(instance_index, time_limit, agents_limit, debug, thread_count));
    }

    // Done.
    return SCIP_OKAY;
}

int main(int argc, char** argv)
{
    const SCIP_RETCODE retcode = start_solver(argc, argv);
    if (retcode != SCIP_OKAY)
    {
        SCIPprintError(retcode);
        return -1;
    }
    return 0;
}
