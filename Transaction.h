#include <boost/noncopyable.hpp>
#include <array>

namespace Alepha
{
	template< typename Step >
	class TransactionStep : boost::noncopyable
	{
		private:
			Step *p;

		public:
			explicit inline TransactionStep( Step &step ) : p( &step ) { step.activate(); }

			~TransactionStep()
			{
				this->p->rollback();
			}
	};

			
	template< typename ... Steps > class TransactionInternal;

	template< typename Step >
	class TransactionInternal< Step > : boost::noncopyable
	{
		private:
			TransactionStep< Step > s;
			static_assert( sizeof( TransactionStep< Step > ) == sizeof( void * ), "" );

		public:
			explicit inline TransactionInternal( Step &step ) : s( step ) {}
	};

	template< typename Step, typename ... Remaining >
	class TransactionInternal< Step, Remaining...  > : boost::noncopyable
	{
		private:
			TransactionStep< Step > s;
			TransactionInternal< Remaining... > r;

		public:
			explicit inline TransactionInternal( Step &step, Remaining ... remaining  )
					: s( step ), r( remaining... ) {}
	};

	class Transaction : boost::noncopyable
	{
		private:
			std::array< std::uint8_t, 16 * sizeof( void * ) > impl_buffer;

		public:
			template< typename ... Steps >
			explicit inline
			Transaction( Steps ... steps )
			{
				new (impl_buffer.begin()) TransactionInternal< Steps... >( steps... );
			}
	};
}
