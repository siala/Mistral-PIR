#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <iomanip>

#include "flatzinc.hpp"
#include <set>
#include <map>
using namespace std;
#include <typeinfo>

#include <math.h>


#ifdef _PARALLEL
#include "omp.h"
#include <limits>
#include <math.h>
#endif

#ifdef _VERIFICATION
void write_solution(FlatZinc::FlatZincModel *fm, string filename)
{
	if (fm->finished())
	{
		//cout << fm->verif_constraints.size() << filename <<endl;
		unsigned int __size=filename.size();
		for (__size; __size>0; __size--)
			if (filename[__size-1] == '/')
				break;
		filename.insert(__size,"sol_" );

		ofstream myfile;
		myfile.open (filename.c_str());

		//cout << filename <<endl;
		for (unsigned int i = 0; i < fm->verif_constraints.size() ; i++)
		{
			if (fm->verif_constraints[i].first != "int_in")
			{
				myfile << "constraint " << fm->verif_constraints[i].first << "(" ;
				int size = fm->verif_constraints[i].second.size();
				for (unsigned int j = 0; j <size ; j++)
				{
					myfile << fm->verif_constraints[i].second[j].get_string() ;
					if (j< (size -1))
						myfile << " , ";
				}
				myfile << ");" << endl;
			}
		}
		myfile <<"solve satisfy;" << endl;
		myfile.close();
		std::cout <<" c DONE" << endl;
	}
}
#endif


int main(int argc, char *argv[])
{
#ifdef _FLATZINC_OUTPUT
	cout << "%";
#endif

	//std::cout.precision(2);

	SolverCmdLine cmd("Mistral (fzn)", ' ', "2.0");

	TCLAP::SwitchArg annotationArg("","follow_annotations","Uses the annotations", false);
	cmd.add( annotationArg );

	TCLAP::ValueArg<int> parityArg("","parity","Uses parity processing", false, 0, "int");
	cmd.add( parityArg );

	TCLAP::SwitchArg simple_rewriteArg("","simple_rewrite","Uses simple rewriting", false);
	cmd.add( simple_rewriteArg );

	TCLAP::SwitchArg branchOnaux("","branch_on_aux","Branching on auxiliary variables", false);
	cmd.add( branchOnaux );

	TCLAP::ValueArg<int>  recommended_Arg("","recommended","use our recommended configurations for search", false, 0, "int");
	cmd.add( recommended_Arg );


	TCLAP::SwitchArg details("","details","print the features and statistics details", false);
	cmd.add( details );

	TCLAP::ValueArg<int>  nb_conf_arg("","nb_configurations","number of configurations to try", false, 6, "int");
	cmd.add( nb_conf_arg );


#ifdef _PARALLEL
	//std::cout << "PARALLEL \n \n \n " << std::endl;
	TCLAP::ValueArg<int> threadsArg("p","number-threads","Use multithreading with this option.", false, 4, "int");
	cmd.add( threadsArg );
#endif

	cmd.parse(argc, argv);

#ifdef _PARALLEL
	int total = threadsArg.getValue();
	if (cmd.enumerate_solutions())
		total=1;
	else{
		//std::cout << "Available  threads : " << omp_get_max_threads() << std::endl;
		//int recommended= floor((double) (omp_get_max_threads()*3) / 4.0 )   ;
		int recommended= floor((double) (omp_get_max_threads()) / 2.0 )   ;
		if (total >recommended){
			//std::cout << " % " << " high value of -p. The solver will use only " << recommended << " threads (recommended) " << std::endl;
			total=recommended;
		}
	}
	//  else
	//	  std::cout << " % " << " will use " << total << " threads " << std::endl;
	omp_set_num_threads(total);
	long int global_obj =std::numeric_limits<int>::max();
	bool solution_found_elsewhere= false;

#endif



#ifdef _PARALLEL
#pragma omp parallel
	{
		//printf("multicore user! %d \n" , omp_get_thread_num());
		int id = omp_get_thread_num();
#endif


		int thread_seed = cmd.get_seed();

#ifdef _PARALLEL
		thread_seed+=id;
#endif
		usrand(thread_seed);





		float nb_var= 1 ;
		float nb_initial_search_var=1 ;
		float nb_small_additionnal_var=  1;
		float nb_last_search_var=1;

		float nb_constraints=1;
		float nb_global= 1;


		bool verbos = details.getValue();

		Vector<int> decay;
		Vector<int> restart;
		Vector<int> aux;
		Vector<int> selector;
		Vector<int> var_heuristic;
		Vector<int> val_heuristic;

		decay.add(1);
		decay.add(2) ;

		restart.add(1);
		restart.add(2) ;

		aux.add(1);
		aux.add(2) ;

		selector.add(1);
		selector.add(2) ;

		var_heuristic.add(1);
		var_heuristic.add(2) ;
		var_heuristic.add(3) ;

		val_heuristic.add(1);
		val_heuristic.add(2) ;
		val_heuristic.add(3) ;


		int conf ;
		Vector<int> all;

		all.add(0) ;

		for (unsigned int d = 0; d< decay.size ; ++d)
			for (unsigned int s = 0; s< selector.size ; ++s)
				for (unsigned int h_val = 0; h_val< val_heuristic.size ; ++h_val)
					for (unsigned int h_var = 0; h_var< var_heuristic.size ; ++h_var)
						for (unsigned int a = 0; a< aux.size ; ++a)
							for (unsigned int r = 0; r< restart.size ; ++r)
							{
								conf = decay[d] + 10* restart[r] + 100* aux[a] + 1000 * selector[s] + 10000 * var_heuristic[h_var] + 100000 * val_heuristic[h_val];
								//std::cout << conf<<  " is a new conf "<< std::endl;
								all.add (conf) ;
								/*
								if (verbos)
								{
									
									std::cout <<  std::endl;
									std::cout <<  " c restart  " << restart[r] << std::endl;
									std::cout <<  " c aux  " << aux[a] << std::endl;
									std::cout <<  " c var_heuristic  " << var_heuristic[h_var] << std::endl;
									std::cout <<  " c val_heuristic  " << val_heuristic[h_val] << std::endl;
									std::cout <<  " c selector  " << selector[s] << std::endl;
									std::cout <<  " c decay  " << decay[d] << std::endl;
								}
								*/
							}

		//if (verbos)
		//	std::cout <<  " c total number of configurations: " << all.size -1 << std::endl;


		Vector<int> configurations;

		configurations.add(0);

		int nb_configurations = nb_conf_arg.getValue() ;

		assert (nb_configurations<= all.size) ;
		for (int i =1 ; i <= nb_configurations; i++){
			//if (i % 2)
				configurations.add(all[i] );
			//else
			//	configurations.add(all[all.size - i]);
			//std::cout << configurations[configurations.size-1] <<  " is a new configuration to try "<< std::endl;
		}

		if (verbos){

			//std::cout <<  " c Total number configurations: " << all.size << std::endl;
			std::cout <<  " c The number of selected configurations: " << configurations.size -1 << std::endl;
			//std::cout <<  " c TIMEOUT: " << cmd.g << std::endl;
		}



		//std::cout << configurations.size <<  " to try! YOUPPY!!  "<< std::endl;


		Vector<bool> vector_is_optimal ;
		Vector<bool> has_solution ;
		Vector<double> vector_cpu_time ;
		Vector<double> vector_best_time;
		Vector<long int> vector_objectiveValues ;

		bool is_maximization = false;
		bool is_optimisation = false;
		bool is_solved_once = false;
		bool is_optimal = false;


		double best_besttime = INFTY;
		double best_cpu_time = INFTY;
		int   best_conf = 0;
		bool best_is_optimal = false ;
		long int best_objective_value=0;



		for (int i = 0 ; i < configurations.size ; i++){


			if (verbos && i>0)
			{
//				std::cout << std::endl ;
				std::cout << " \n  \n c Configuration " << i ;
				std::cout << std::endl ;
			}	
			Solver s;

			cmd.set_parameters(s);
			//decay will change later;

			std::string policy;
#ifdef _PARALLEL
			if ( id%2 != 0)
				if (cmd.get_restart_policy()=="geom")
					policy="luby";
				else
					policy="geom";
			else
#endif

				policy =cmd.get_restart_policy();

			bool branch_on_auxilary =   branchOnaux.getValue();
			//std::cout << "branch_on_auxilary " << branch_on_auxilary << std::endl;
			//exit(1);


#ifdef _PARALLEL
			//  bool branch_on_auxilary = true;
			if ( id%4 < 2 )
				branch_on_auxilary=false;
			//#pragma omp critical
			//  std::cout << " " << s.parameters.prefix_statistics << "ID:  " << id  << " " << policy << " branch_on_auxilary " << branch_on_auxilary <<  "  "<< thread_seed <<  std::endl;
#endif

			double cpu_time = get_run_time() ;

#ifdef _VERBOSE_PARSER
			std::cout << " " << s.parameters.prefix_comment << " Parse: ";
#endif

			FlatZinc::Printer p;
			FlatZinc::FlatZincModel *fm = 0L;

			fm = parse(cmd.get_filename(), s, p);
			fm->print = verbos;

			if( !fm )
#ifdef _PARALLEL
				exit(1);
#else
			return 0;
#endif

			fm->set_enumeration(cmd.enumerate_solutions());
			double parse_time = get_run_time() - cpu_time;
#ifdef _VERBOSE_PARSER
			std::cout << std::endl;
#endif

#ifdef _PARALLEL
			//#pragma omp critical
			//cout << " " << s.parameters.prefix_statistics << " PARSETIME " << parse_time << std::endl;
#else
			if(s.parameters.verbosity >0)
				cout << " " << s.parameters.prefix_statistics << " PARSETIME " << parse_time << std::endl;
#endif

			FlatZinc::SolutionPrinter *sp = new FlatZinc::SolutionPrinter(&p, fm, &s);
			s.add(sp);


			if(cmd.print_statistics())
				if(s.parameters.time_limit>0) std::cout << " " << s.parameters.prefix_statistics
				<< " CUTOFF " << s.parameters.time_limit << std::endl;


			fm->branch_on_auxilary=branch_on_auxilary;


			//recommended_Arg._value =  configurations[i] ;

			//  if (!recommended_Arg.getValue())
			//	  fm->set_strategy(cmd.get_variable_ordering(), cmd.get_value_ordering(), cmd.get_randomization(), policy);
			//  else
			//	  fm->recommended_configs(recommended_Arg.getValue());
			//fm->recommended_configs(configurations[i]);
			if (i>0)
				fm->specific_configuration(configurations[i]);

			fm->set_display_collection(cmd.print_instance());
			//  fm->set_strategy(cmd.get_variable_ordering(), cmd.get_value_ordering(), cmd.get_randomization(), policy);
			fm->set_display_model(cmd.print_model());
			fm->set_display_solution(cmd.print_solution());
			fm->set_annotations(annotationArg.getValue());
			//fm->set_annotations(false);
			fm->set_rewriting(cmd.use_rewrite());
			fm->set_simple_rewriting(simple_rewriteArg.getValue());
			fm->set_parity_processing(parityArg.getValue());
			fm->encode_clauses();

#ifdef _PARALLEL
			if (fm->method() == FlatZinc::FlatZincModel::MAXIMIZATION)
				global_obj = std::numeric_limits<int>::min();
			fm->best_kown_objective = &global_obj;
			fm->set_solution_found_elsewhere(&solution_found_elsewhere);
#endif


			fm->run(cout , p,i==0);
			is_optimisation = fm->is_optimisation() ;
			is_maximization=fm->is_maximisation();

			//if (i>0){

			//double time = fm->cpu_time ;
			vector_cpu_time.add(fm->cpu_time ) ;
			vector_best_time.add (fm->cpu_besttime) ;
			vector_is_optimal.add(fm->optimal);
			vector_objectiveValues .add(fm->objective_value);
			//if (i>0){
			if (fm->found_a_solution){
				is_solved_once = true;
				has_solution.add(true);
			}
			else
				has_solution.add(false);

			if (fm->optimal)
				is_optimal = true;


			//}

#ifdef _PARALLEL
#pragma omp critical
			{
				if (!solution_found_elsewhere){
					solution_found_elsewhere=true;
					//Secure shared memory
#pragma omp flush
					// }
					// else
#endif

					if (i!=0){
						if(cmd.print_solution())
							fm->print_final(cout , p);


#ifdef _PARALLEL
					}
				}
#endif

				if(cmd.print_statistics())
					s.statistics.print_full(std::cout);
			}
			else{

				nb_var=fm->nb_var ;
				nb_initial_search_var=fm->nb_initial_search_var;
				nb_small_additionnal_var=  fm->nb_small_additionnal_var;
				nb_last_search_var=fm->nb_last_search_var;

				nb_constraints=fm->nb_constraints;
				nb_global= fm->nb_global;

				/*
	  std::cout << " t Variables " << nb_var  << std::endl;
	  std::cout << " t InitialSearchVariables " << nb_initial_search_var  << std::endl;
	  std::cout << " t nb_last_search_var " << nb_last_search_var  << std::endl;
	  std::cout << " t SmallAdditionnalVariables " << nb_small_additionnal_var << std::endl;
	  std::cout << " t Constraints " << nb_constraints << std::endl;
	  std::cout << " t GlobalConstraints " << nb_global << std::endl;


	  std::cout << " t InitialSearchVariables By Variables " << nb_initial_search_var/nb_var  << std::endl;
	  std::cout << " t SmallAdditionnalVariables By Variables " << nb_small_additionnal_var/nb_var << std::endl;
	  std::cout << " t LastSearchVariables By Variables " << nb_last_search_var/nb_var  << std::endl;

	  std::cout << " t Constraints " << nb_constraints << std::endl;
	  std::cout << " t GlobalConstraints By Constraints " << nb_global/nb_constraints << std::endl;
	  std::cout << " t Var By Constraints " << nb_var/nb_constraints  << std::endl;
				 */
			}



#ifdef _VERIFICATION
			write_solution(fm, args.back());
#endif


			delete fm;
			delete sp;
			//exit(1);

#ifdef _PARALLEL
		}
#endif



		//bool easy = false;
		if (i>0)
		{


			//if (configurations.size >2)

			//std::cout << "vector_objectiveValues size "  << vector_objectiveValues.size  << std::endl ;
			//std::cout << "configurations.size "  << configurations.size  << std::endl ;

			//assert(vector_objectiveValues.size == (configurations.size));

			if (verbos)
			{
//				std::cout << std::endl ;
//				std::cout << " Configuration " << i ;
//				std::cout << std::endl ;
				//if (!is_solved_once)
				//std::cout << "   NO Solution!!!!!!   "  << std::endl ;
				if (fm->found_a_solution)
					std::cout << " c Found a solution" ;
				else
					std::cout << " c No solution found" ;


				std::cout << std::endl ;

				std::cout << " c CPUtime" ;
				//for (int i = 0; i< vector_cpu_time.size; ++i)
				std::cout << " " <<  vector_cpu_time[i] ;


				std::cout << std::endl ;



				//std::cout << " BestTime" ;
				//std::cout << " " <<  vector_best_time[i] ;

				if (is_optimisation){
					std::cout << " c Objective Value " ;
					std::cout << " " <<  vector_objectiveValues[i] ;

					std::cout << std::endl ;
				}

				std::cout << " c IsOptimal ? " <<  vector_is_optimal[i]  << std::endl ;

			}

			//std::cout << std::endl ;


			//if (configurations.size )
			//{
			if (is_solved_once)
			{


				if (i==1)
				{
					best_objective_value = vector_objectiveValues[1] ;
					best_cpu_time = vector_cpu_time[1] ;
					best_besttime = vector_best_time[1] ;
					best_is_optimal  = vector_is_optimal[1] ;
					//best_conf = configurations[1] ;
					best_conf=1;

					/*
					std::cout << "   Here i = 1 !!!!!!   "  << std::endl ;
					std::cout << " best_objective_value" << best_objective_value << std::endl ;
					std::cout << " best_cpu_time" << best_cpu_time << std::endl ;
					std::cout << " best_besttime" <<  best_besttime << std::endl ;
					std::cout << " best_is_optimal" <<  best_is_optimal<< std::endl ;
					std::cout << " best_conf" <<  best_conf<< std::endl ;
					 */

				}
				else
				{
					if (is_optimisation){
						//assert(vector_cpu_time.size == (configurations.size ));

						//std::cout << "   Here i =  " << i << std::endl ;

						//if (verbos) {
						//	std::cout << std::endl ;
						//	std::cout << " Objective Value " ;
						//}

						for (int i = 2; i< vector_objectiveValues.size; ++i){
							//if (verbos)
							//	std::cout << " " <<  vector_objectiveValues[i] ;

							if (is_maximization){
								if (vector_objectiveValues[i]>= best_objective_value )
								{
									bool update =false;

									if (vector_objectiveValues[i]== best_objective_value){

										if (vector_is_optimal[i])
										{
											if (best_is_optimal){
												if(vector_cpu_time[i] < best_cpu_time  )
													update = true;
											}
											else
												update =true;

										}
										else
											if (!best_is_optimal)
												if(vector_best_time[i] < best_besttime  )
													update = true;

									}
									else
										update = true;



									if (update){
										best_is_optimal = vector_is_optimal [i] ;
										best_objective_value = vector_objectiveValues[i] ;
										best_besttime = vector_best_time[i] ;
										best_cpu_time = vector_cpu_time[i] ;
										//best_conf = configurations[i] ;
										best_conf = i;
									}


								}
							}
							else
								//minimization
								if (vector_objectiveValues[i]<= best_objective_value )
								{
									bool update =false;

									if (vector_objectiveValues[i]== best_objective_value){

										if (vector_is_optimal[i])
										{
											if (best_is_optimal){
												if(vector_cpu_time[i] < best_cpu_time  )
													update = true;
											}
											else
												update =true;

										}
										else
											if (!best_is_optimal)
												if(vector_best_time[i] < best_besttime  )
													update = true;

									}
									else
										update = true;



									if (update){
										best_is_optimal = vector_is_optimal [i] ;
										best_objective_value = vector_objectiveValues[i] ;
										best_besttime = vector_best_time[i] ;
										best_cpu_time = vector_cpu_time[i] ;
										//best_conf = configurations[i] ;
										best_conf = i;
									}
								}
						}

					}
					else{
						best_cpu_time=vector_cpu_time[1] ;
						best_conf=1 ;
						for (int i = 1; i< vector_cpu_time.size; ++i)
							if ( vector_cpu_time[i]  < best_cpu_time ){
								best_cpu_time=vector_cpu_time[i];
								//best_conf = configurations[i];
								best_conf = i;
							}
					}
				}


			}
		}
		else
		{

			if (verbos){
				std::cout << " c Caracteristics: Satisfaction/Minization/Maximisation ? (1,2, or 3)  Variables SmallVariables Constraints GlobalConstraints SearchVars/Vars SmallVars/vars GlobalConstraints/Constraints Vars/Constraints " <<	std::endl;
			}

			//std::cout << " " << cmd.get_seed() ;
			if (is_optimisation){
				if (is_maximization)
					std::cout  << "3"  ;
				else
					std::cout  << "2"  ;
			}
			else
			std::cout  << "1"  ;

			std::cout  << " " << nb_var  ;
			std::cout << " " << nb_initial_search_var ;
			//std::cout << " " << nb_last_search_var ;
			std::cout << " " << nb_small_additionnal_var ;
			std::cout << " " << nb_constraints ;
			std::cout << " " << nb_global ;

			double value = nb_initial_search_var/nb_var ;
			value= round( value * 100.0 ) / 100.0;
			std::cout << " " << value ;

			value = nb_small_additionnal_var/nb_var  ;
			value= round( value * 100.0 ) / 100.0;
			std::cout << " " << value ;

			value= nb_global/nb_constraints ;
			value= round( value * 100.0 ) / 100.0;
			std::cout << " " << value  ;

			value=  nb_var/nb_constraints ;
			value= round( value * 100.0 ) / 100.0;
			//std::cout << " " << value ;
			//std::cout << std::endl ;
			//std::cout << " " << cmd.get_seed() ;
			std::cout <<std::flush;

		}

	}

	if (verbos) {

		std::cout << std::endl ;
		if (is_solved_once)
		{
			if (is_optimisation){
				if (is_maximization)
					std::cout <<  " c Final Maximization best objective  " << best_objective_value << "\n c TotalTime " << best_cpu_time << "\n c IsOptimal? " << is_optimal << " Configuration " << best_conf << std::endl ;
				else
					std::cout <<  " c Final Minimization best objective  " << best_objective_value << "\n c  TotalTime " << best_cpu_time << "\n c IsOptimal? " << is_optimal  << " Configuration " << best_conf << std::endl ;
			}
			else
				std::cout <<  " c Best time across all configurations " << best_cpu_time << " with configuration " << best_conf << std::endl ;
		}
		else
			std::cout <<  " c Not solved" << std::endl ;


	}

	Vector<double> scores;

	if (verbos)
		std::cout <<  " c The scores (from the first to the last configuration)" << std::endl ;

	assert (has_solution.size == configurations.size);
	for (int z = 1; z<configurations.size; ++z){

		double value = 0.0;
		if (has_solution[z]){
			if (is_optimisation){
				if (best_is_optimal){
					if (vector_is_optimal[z]){
						value+= 33.33;
						value+= 33.33 * (double)best_cpu_time / (double) vector_cpu_time[z];
					}
				}
				else
					value+=66.66;

				if (is_maximization)
					value+= 33.33 * (double) vector_objectiveValues[z] / (double) best_objective_value ;
				else
					value+= 33.33 * ((double) best_objective_value / (double) vector_objectiveValues[z]);
			}
			else
				value = 100.00  *  (double) best_cpu_time / (double) vector_cpu_time[z];
			//value = ceil (1000 * value);
		}
		//			std::cout <<  " Final Satisfaction best time " << best_cpu_time << " with conf " << best_conf << std::endl ;
		assert(value <= 100.00) ;
		scores.add(value);
		std::cout  <<  " " <<  value;
	}

	/*
	if (is_solved_once)
	{
		if (is_optimisation){
			if (is_maximization)
				std::cout <<  " Final Maximization best objective  " << best_objective_value << " TotalTime " << best_cpu_time << " BestTime " << best_besttime << " Configuration " << best_conf << std::endl ;
			else
				std::cout <<  " Final Minimization best objective  " << best_objective_value << " TotalTime " << best_cpu_time << " BestTime " << best_besttime << " Configuration " << best_conf << std::endl ;
		}
		else
			std::cout <<  " Final Satisfaction best time " << best_cpu_time << " with conf " << best_conf << std::endl ;
	}
	else
		std::cout <<  " Final No solution" << std::endl ;
	 */


	std::cout << std::endl ;

	return 0;
}
