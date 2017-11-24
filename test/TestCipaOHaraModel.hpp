/*

Copyright (c) 2005-2017, University of Oxford.
All rights reserved.

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
 * Neither the name of the University of Oxford nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TESTCIPAOHARAMODEL_HPP_
#define TESTCIPAOHARAMODEL_HPP_


#include <cxxtest/TestSuite.h>
#include "CellProperties.hpp"
#include "SteadyStateRunner.hpp"
#include "AbstractCvodeCell.hpp"
#include "RegularStimulus.hpp"
#include "AbstractIvpOdeSolver.hpp"
#include "ohara_rudy_2011_endo_dyHergCvode.hpp"

/* This test is always run sequentially (never in parallel)*/
#include "FakePetscSetup.hpp"


class TestCipaOHaraModel : public CxxTest::TestSuite
{
public:
    void TestCipaVersionOfOharaModel()
    {

#ifdef CHASTE_CVODE

        boost::shared_ptr<RegularStimulus> p_stimulus;
        boost::shared_ptr<AbstractIvpOdeSolver> p_solver;
        boost::shared_ptr<AbstractCvodeCell> p_model(new Cellohara_rudy_2011_endo_dyHergFromCellMLCvode(p_solver, p_stimulus));

        /*
         * Once the model is set up we can tell it to use the the default stimulus from CellML,
         * (if one has been labelled, you get an exception if not), and return it.*/
        boost::shared_ptr<RegularStimulus> p_regular_stim = p_model->UseCellMLDefaultStimulus();

        /*
         * Now you can modify certain parameters of the stimulus function, such as the period
         */
	// Set CL as the one CiPA use
        p_regular_stim->SetPeriod(2000.0);

        /*
         * == Changing Parameters in the Cell Model ==
         *
         * You can also change any parameters that are labelled in the cell model.
         *
         * Instructions for annotating parameters can be found at [wiki:ChasteGuides/CodeGenerationFromCellML]
         *
         * Here we show how to change the parameter dictating the maximal conductance of the IKs current.
         * Note this call actually leaves it unchanged from the default,
         * you can experiment with changing it and examine the impact on APD.
         */
        //p_model->SetParameter("membrane_slow_delayed_rectifier_potassium_current_conductance", 0.07);


	// Check dy at t=0
	//std::vector<double> states = p_model->GetstdVecStatVariables();
	//TODO: set initial values to match CiPA one
	double getTime = 0;
	N_Vector Y;
	Y = p_model->GetStateVariables();
	N_Vector dY;
	dY = p_model->GetStateVariables(); // Not the best way of initialising dY
	p_model->EvaluateYDerivatives(getTime, Y, dY);
	for (int i; i<48; i++) {
		std::cout << NV_Ith_S(dY,i) << "\n"; //TODO: Print out for now, need to compare with CiPA output
	}


        /*
         * == Getting detail for paces of interest ==
         *
         * Now we solve for the number of paces we are interested in.
         *
         * The absolute values of start time and end time are typically only relevant for the stimulus, in general
         * nothing else on the right-hand side of the equations uses time directly.
         *
         * i.e. if you have a `RegularStimulus` of period 1000ms then you would get exactly the same results
         * calling Solve(0,1000,...) twice, as you would calling Solve(0,1000,...) and Solve(1000,2000,...).
         *
         * Single cell results can be very sensitive to the sampling time step, because of the steepness of the upstroke.
         *
         * For example, try changing the line below to 1 ms. The upstroke velocity that is detected will change
         * from 339 mV/ms to around 95 mV/ms. APD calculations will only ever be accurate to sampling timestep
         * for the same reason.
         */
        double max_timestep = 0.1;
        p_model->SetMaxTimestep(max_timestep);

	// Set sampling step size as the one CiPA use
        double sampling_timestep = 1.0;
        double start_time = 0.0;
        double end_time = 1000.0;
        OdeSolution solution = p_model->Compute(start_time, end_time, sampling_timestep);

        /*
         * `p_model` retains the state variables at the end of `Solve`, if you call `Solve` again the state
         * variables will evolve from their new state, not the original initial conditions.
         *
         * Write the data out to a file.
         */
        solution.WriteToFile("TestCipaOHaraModel","OHaraDyHergCvode","ms");

        /*
         * == Calculating APD and Upstroke Velocity ==
         */
        unsigned voltage_index = p_model->GetSystemInformation()->GetStateVariableIndex("membrane_voltage");
        std::vector<double> voltages = solution.GetVariableAtIndex(voltage_index);
        CellProperties cell_props(voltages, solution.rGetTimes());

        //double apd = cell_props.GetLastActionPotentialDuration(90);

        //TS_ASSERT_DELTA(apd, 268.92, 1e-2);
	
#else
        std::cout << "Cvode is not enabled.\n";
#endif
    }
};

#endif /*TESTCIPAOHARAMODEL_HPP_*/
