
#include <iofox.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffer.hpp>
#include <fmt/core.h>
#include <exception>
#include <chrono>
#include <string>

namespace asio { using namespace boost::asio; }

auto coro(asio::io_context & executor) -> io::coro<void>
{
	asio::ip::tcp::socket socket {executor};
	asio::ip::tcp::endpoint endpoint {asio::ip::make_address("127.0.0.1"), 555};
	co_await socket.async_connect(endpoint, io::use_coro);

	for(;;)
	{
		std::string buffer = "garox";
		auto size = co_await socket.async_write_some(asio::buffer(buffer), io::use_coro);
		fmt::print("[coro] - send '{}' bytes. data: '{}'.\n", size, buffer);
		co_await asio::steady_timer(executor, std::chrono::seconds(1)).async_wait(io::use_coro);
	}
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
