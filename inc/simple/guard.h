#ifndef GUARD_H_66BC65DB_AFF6_43C8_8654_D1A2801635E2
#define GUARD_H_66BC65DB_AFF6_43C8_8654_D1A2801635E2

#include	<stdexcept>

struct	guard_invoke_times {
    guard_invoke_times(const char* func_name, int max_invoke_times, int& counter)
        :	max_times_(max_invoke_times),	invoke_times_(counter) {
        if(invoke_times_ >= max_invoke_times) {
            throw	std::logic_error("invoke too many times");
        }
        invoke_times_++;
    }

    ~guard_invoke_times() {
        invoke_times_--;
    }

private:
    int			max_times_;
    int&		invoke_times_;
};

#define		GUARD_RECURSIVE_INVOKE_TIMES(MAX_TIMES)						\
	static	int			s_recursive_invoke_counter___	= 0;											\
	guard_invoke_times	recursion_invoke_guard___(__FUNCTION__, MAX_TIMES, s_recursive_invoke_counter___);

//
//	Exameple:
//
//	void	Test()
//	{
//		RECURSIVE_INVOKE_TIMES_GUARD(10);
//
//		Test();
//	}
//

#endif	//RECURSIONGUARD_H__089BB269_EC67_495D_885A_CAB04219E370__INCLUDED_
