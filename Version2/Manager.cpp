#include "Manager.h"

#include <chrono>
#include <iostream>
#include <numeric>

#include "demofuncs.h"
#include "TimeUtils.h"

namespace spos::lab1::version2 {
	inline void Manager::StopRunningProcesses()
	{
		std::set<int>& s = running_processes;
		for (int index : s)
			child_processes[index].terminate();
		s.clear();
	}
	inline void Manager::DivideTasks(const string& exec_name, int arg)
	{
		for (int i = 0; i < tasks_amount; ++i)
		{
			in_pipes[i] << i << endl;
			in_pipes[i] << arg << endl;
		}
		for (int i = 0; i < tasks_amount; ++i)
			child_processes[i] = bp::child(exec_name, "OPTIONAL", bp::std_in < in_pipes[i], bp::std_out > out_pipes[i], bp::std_err > stderr);
	}
	void Manager::UpdateRunningProcesses()
	{
		std::set<int>& s = running_processes;
		for (auto it = s.begin(); it != s.end();) {
			int i = *it;
			if (!child_processes[i].running()) {
				int tmp_res;
				out_pipes[i] >> tmp_res;

				if (!ProcessComputationalResult(tmp_res))
					return;

				res_vec[i] = tmp_res;
				it = s.erase(it);
				child_processes[i].wait();
			}
			else
				it++;
		}
	}
	void Manager::UpdatePromptMessage()
	{
		if (!show_prompt || !timer->Finished())
			return;


		cout << prompt_message;
		int choice;
		cin >> choice;
		switch (choice)
		{
		case 2:
			show_prompt = false;
			break;
		case 3:
			stop_job = true;
			break;
		case 1:
			timer->Start(prompt_interval);
			break;
		default:
			cout << "Unknown message type" << endl;
		}
		system("cls");
	}
	int Manager::ProcessComputationalResult(int tmp_res)
	{
		if (tmp_res == 0)
		{
			res = 0;
			StopRunningProcesses();
			return 0;
		}
		return running_processes.size(); //return number of active child processes for which manager should wait
	}
	void Manager::ComputeResult()
	{
		if (res.has_value())
			return;
		res = std::reduce(res_vec.begin(), res_vec.end(), 0, std::move(res_func));
	}
	Manager::Manager() :
		stop_job(false), show_prompt(true), tasks_amount(0), res(std::nullopt)
	{
		prompt_message = "----Choose your option----\n"
						 "1.Continue................\n"
						 "2.Continue without prompt.\n"
						 "3.Cancel..................\n"
						  "Choose your action: ";
		timer = new SimpleTimer();
	}

	Manager::~Manager()
	{
		in_pipes.clear();
		out_pipes.clear();
		child_processes.clear();
		res_vec.clear();
		running_processes.clear();
		delete timer;
	}

	void Manager::SetUp(int tasks_amount, std::chrono::milliseconds&& duration, std::function<int(int, int)>&& res_func)
	{
		this->tasks_amount = tasks_amount;
		this->prompt_interval = duration;
		in_pipes.resize(tasks_amount);
		out_pipes.resize(tasks_amount);
		child_processes.resize(tasks_amount);
		res_vec.resize(tasks_amount, -1);
		this->res_func = res_func;
	}
	void Manager::RunVersion2(int argc, char** argv)
	{
		cout << "Enter function's argument: ";
		int func_arg;
		std::cin >> func_arg;

		DivideTasks(argv[0], func_arg);

		for (int i = 0; i < tasks_amount; ++i)
			running_processes.insert(i);

		timer->Start(prompt_interval);

		while (!stop_job && !running_processes.empty())
		{
			UpdatePromptMessage();
			UpdateRunningProcesses();
			this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (stop_job && !running_processes.empty())
		{
			StopRunningProcesses();
			cout << "Result computation has been stopped" << endl;
		}
		else {
			ComputeResult();
			cout << "Res= " << res.value() << endl;
		}

		system("pause");
	}
	void Manager::RunParrallelFunction()
	{
		namespace testing = spos::lab1::demo;

		// Parrallel processes accepts function number(0,1,2 ... ,n)
		// Currently used to identify whether call f or g, but can be generalized to n functions
		int f_number;
		cin >> f_number;

		int f_arg;
		cin >> f_arg;

		int res;
		if (f_number & 1)
			res = testing::f_func<testing::INT>(f_arg);
		else
			res = testing::g_func<testing::INT>(f_arg);
		// For n functions external function call should be used
		// CallFunc(function_number, arg, res) or res = CallFunc(function_number, arg)
		cout << res << endl;
	}
}
