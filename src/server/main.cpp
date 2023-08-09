
#include <iofox.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/read.hpp>
#include <string_view>
#include <fmt/core.h>
#include <exception>

namespace asio { using namespace boost::asio; }

auto coro(asio::io_context & executor) -> io::coro<void>
{
	asio::ip::tcp::endpoint endpoint {asio::ip::make_address("127.0.0.1"), 555};
	asio::ip::tcp::acceptor acceptor {executor, endpoint};
	auto socket = co_await acceptor.async_accept(io::use_coro);

	for(;;)
	{
		char buffer_size[4];
		co_await asio::async_read(socket, asio::buffer(buffer_size), asio::transfer_exactly(4), io::use_coro);
		int & size = reinterpret_cast<int &>(buffer_size);

		char * buffer_data = new char[size];
		co_await asio::async_read(socket, asio::buffer(buffer_data, size), asio::transfer_exactly(size), io::use_coro);

		fmt::print("[coro] - receive '{}' bytes. data: '{}'.\n", size, std::string_view(buffer_data, size));

		delete[] buffer_data;
	}

	fmt::print("sas.\n");
	co_return;
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;
	asio::co_spawn(ctx, coro(ctx), io::rethrowed);
	ctx.run();
	return 0;
}
catch(const std::exception & exception)
{
	fmt::print("[main] - exception: '{}'.\n", exception.what());
}
