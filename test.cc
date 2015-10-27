#include "Transaction.h"
#include <iostream>
#include <functional>
#include <cassert>

void call();

class ActivationCounter
{
	private:
		static int active;
		friend int main();

	public:
		ActivationCounter() { ++active; }
		ActivationCounter( const ActivationCounter &value ) { ++active; }
		ActivationCounter &operator= ( const ActivationCounter &value ) { ++active; }
		ActivationCounter( ActivationCounter &&value ) { ++active; }
		ActivationCounter &operator= ( ActivationCounter &&value ) { ++active; }
		~ActivationCounter() { --active; }
};

int ActivationCounter::active= 0;

int
main()
{
	call();
	assert( ActivationCounter::active == 0 );
}


class MyException : public std::exception, ActivationCounter
{
	public:
		const char *
		what() const noexcept
		{
			return "My Exception";
		}
};

struct Step1 : ActivationCounter
{
	void activate() { std::cout << "Step 1" << std::endl; }
	void rollback() { std::cout << "Step 1 undo" << std::endl; }
};

struct Step2 : ActivationCounter
{
	void activate() { std::cout << "Step 2" << std::endl; }
	void rollback() { std::cout << "Step 2 undo" << std::endl; }
};

struct Step3 : ActivationCounter
{
	void activate() { std::cout << "Step 3" << std::endl; }
	void rollback() { std::cout << "Step 3 undo" << std::endl; }
};

struct FailingStep : ActivationCounter
{
	void activate() { throw MyException(); }
	void rollback() { abort(); }
};

struct Step4 : ActivationCounter
{
	void activate() { std::cout << "Step 4" << std::endl; }
	void rollback() { std::cout << "Step 4 undo" << std::endl; }
};

class Step : ActivationCounter
{
	private:
		std::function< void() > activation;
		std::function< void() > deactivation;


	public:
		Step( std::function< void () > a, std::function< void () > d )
				: activation( a ), deactivation( d ) {}

		template< typename Activator, typename Deactivator >
		Step( Activator a, Deactivator d )
				: activation( a ), deactivation( d ) {}

		template< typename Activator >
		Step( Activator a )
				: activation( a ), deactivation( []{} ) {}

		void activate() const { activation(); }
		void rollback() const { deactivation(); }
};


void
call()
try
{
	Alepha::Transaction
	{
			Step
			{
				[]{ std::cout << "Lambda doit" << std::endl; },
				[]{ std::cout << "Lambda undo" << std::endl; }
			},

			Step1(),
			Step2(),
			Step3(),
			Step4(),

			Step
			{
				[]{ std::cout << "Penultimate step" << std::endl; },
				[]{ std::cout << "Penultimate undo" << std::endl; }
			},

			Step
			{
				[]
				{
					std::cout << "Should I die?" << std::endl;
					std::string input;
					std::cin >> input;
					if( input == "yes" ) throw MyException();
				}
			},
	};
}
catch( const std::exception &ex )
{
	std::cerr << "Error: " << ex.what() << std::endl;
}
