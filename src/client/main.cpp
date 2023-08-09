
#include <iofox.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <fmt/core.h>
#include <exception>
#include <chrono>
#include <string>

namespace asio { using namespace boost::asio; }

auto writer(char name, asio::io_context & executor, asio::ip::tcp::socket & socket) -> io::coro<void>
{
	for(;;)
	{
		std::string buffer(100000, name);

		int package_size = buffer.size();
		buffer.insert(0, reinterpret_cast<char *>(&package_size), 4);

		co_await asio::async_write(socket, asio::buffer(buffer), asio::transfer_exactly(buffer.size()), io::use_coro);
		fmt::print("[coro '{}'] - send '{}' bytes. data: '{}'.\n", name, buffer.size() - 4, buffer.substr(4));
	}
}

auto coro(asio::io_context & executor, asio::ip::tcp::socket & socket) -> io::coro<void>
{
	asio::ip::tcp::endpoint endpoint {asio::ip::make_address("127.0.0.1"), 555};
	co_await socket.async_connect(endpoint, io::use_coro);

	asio::co_spawn(executor, writer('x', executor, socket), io::rethrowed);
	asio::co_spawn(executor, writer('y', executor, socket), io::rethrowed);
}

int main() try
{
	io::windows::set_asio_locale(io::windows::lang::english);
	asio::io_context ctx;

	asio::ip::tcp::socket socket {ctx};
	asio::co_spawn(ctx, coro(ctx, socket), io::rethrowed);

	ctx.run();
	return 0;
}
catch(const std::exception & exception)
{
	fmt::print("[main] - exception: '{}'.\n", exception.what());
}
