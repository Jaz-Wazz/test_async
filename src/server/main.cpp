
#include <iofox.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
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
		char buffer[1024];
		auto size = co_await socket.async_receive(asio::buffer(buffer, 1024), io::use_coro);
		fmt::print("[coro] - receive '{}' bytes. data: '{}'.\n", size, std::string_view(buffer, size));
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
